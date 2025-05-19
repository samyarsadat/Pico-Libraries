/*
    Pico Libraries
    Micro-ROS diagnostics message publishing util.
    
    Copyright 2025 Samyar Sadat Akhavi
    Written by Samyar Sadat Akhavi, 2025.
 
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https: www.gnu.org/licenses/>.
*/

#include "uros_utils_lib/diag_helper.h"
#include "pico_log_lib/logger.h"
#include "FreeRTOS.h"
#include "ftoa.c"
#include <type_traits>
#include <memory>


// This must be declared elsewhere!
extern Logger logger;


/* ---- DiagKvPairs ---- */
DiagKvPairs::DiagKvPairs(const size_t capacity) {
    this->capacity = capacity;
    this->to_free_char_ptrs_size = capacity * 2;  // Only a maximum of 2 buffers are allocated per KV pair.

    this->to_free_char_ptrs = static_cast<char**>(pvPortCalloc(this->to_free_char_ptrs_size, sizeof(char*)));
    this->storage_ptr = static_cast<diagnostic_msgs__msg__KeyValue*>(pvPortCalloc(capacity, sizeof(diagnostic_msgs__msg__KeyValue)));

    for (size_t i = 0; i < this->to_free_char_ptrs_size; i++) {
        this->to_free_char_ptrs[i] == nullptr;
    } 
}

DiagKvPairs::~DiagKvPairs() {
    if (this->storage_ptr != nullptr) {
        vPortFree(this->storage_ptr);
    }

    if (this->to_free_char_ptrs != nullptr) {
        for (size_t i = 0; i < this->to_free_char_ptrs_size; i++) {
            if (this->to_free_char_ptrs[i] != nullptr) {
                vPortFree(this->to_free_char_ptrs[i]);
            }
        }

        vPortFree(this->to_free_char_ptrs);
    }
}

template<typename T>
constexpr bool always_false = false;

#define KEY_CONV_BUFF()                                                      \
    ret_key = static_cast<char*>(pvPortMalloc(KV_CONVERSION_BUFF_SIZE_INT)); \
    assert(this->to_free_char_ptrs_index < this->to_free_char_ptrs_size);    \
    this->to_free_char_ptrs[this->to_free_char_ptrs_index++] = ret_key;

#define VALUE_CONV_BUFF()                                                      \
    ret_value = static_cast<char*>(pvPortMalloc(KV_CONVERSION_BUFF_SIZE_FLT)); \
    assert(this->to_free_char_ptrs_index < this->to_free_char_ptrs_size);      \
    this->to_free_char_ptrs[this->to_free_char_ptrs_index++] = ret_value;

template <typename KT, typename VT>
bool DiagKvPairs::add(KT key, VT value) {
    char* ret_key, ret_value;

    if constexpr (std::is_same<VT, unsigned>) {
        VALUE_CONV_BUFF();
        (void) utoa(value, ret_value, 10);
    } else if constexpr (std::is_same<VT, int>) {
        VALUE_CONV_BUFF();
        (void) itoa(value, ret_value, 10);
    } else if constexpr (std::is_same<VT, float>) {
        VALUE_CONV_BUFF();
        (void) ftoa(ret_value, value, KV_FTOA_DIG_AFTER_DEC_POINT);
    } else if constexpr (std::is_same<VT, bool>) {
        ret_value = value ? "true" : "false";
    } else {
        static_assert(always_false<VT>, "Unsupported value type!");
    }

    if constexpr (std::is_same<KT, const char*>) {
        ret_key = const_cast<char*>(key); 
    } else if constexpr (std::is_same<KT, unsigned>) {
        KEY_CONV_BUFF();
        (void) utoa(key, ret_key, 10);
    } else if constexpr (std::is_same<KT, int>) {
        KEY_CONV_BUFF();
        (void) itoa(key, ret_key, 10);
    } else {
        static_assert(always_false<KT>, "Unsupported key type!");
    }

    return this->add(ret_key, ret_value);
}

bool DiagKvPairs::add(const char* key, const char* value) {
    if (this->arr_size < this->capacity) {
        this->storage_ptr[arr_size] = {
            .key = {const_cast<char*>(key), strlen(key)},
            .value = {const_cast<char*>(value), strlen(value)}
        };
        this->arr_size++;
        return true;
    }

    return false;
}

size_t DiagKvPairs::size() {
    return this->arr_size;
}

diagnostic_msgs__msg__KeyValue* DiagKvPairs::arr_ptr() {
    return this->storage_ptr;
}


/* ---- DiagPublisher ---- */
DiagPublisher::DiagPublisher(const rcl_publisher_t* diag_pub) {
    assert(diag_pub != nullptr);
    this->publisher = diag_pub;
}

void DiagPublisher::enable_diag(bool enabled) {
    this->diag_enabled = enabled;
}

bool DiagPublisher::publish(const DIAG_MSG_LEVEL level, const char* name, const char* hw_id, const char* msg, DiagKvPairs* kv_pairs, const bool log) {
    if (this->diag_enabled) {
        diagnostic_msgs__msg__DiagnosticStatus diag_msg;
        
        diag_msg.level = level;
        diag_msg.name.data = const_cast<char*>(name);
        diag_msg.name.size = strlen(name);
        diag_msg.message.data = const_cast<char*>(msg);
        diag_msg.message.size = strlen(msg);
        diag_msg.hardware_id.data = const_cast<char*>(hw_id);
        diag_msg.hardware_id.size = strlen(hw_id);
        
        if (kv_pairs != nullptr) {
            diag_msg.values.data = kv_pairs->arr_ptr();
            diag_msg.values.size = kv_pairs->size();
        } else {
            diag_msg.values.data = nullptr;
            diag_msg.values.size = 0;
        }

        if (log) {
            log_diag_msg(&diag_msg);
        }

        return rcl_publish(this->publisher, &diag_msg, nullptr) == RCL_RET_OK;
    }

    return true;
}

void DiagPublisher::log_diag_msg(diagnostic_msgs__msg__DiagnosticStatus* diag_msg) {
    size_t kv_buff_size = 1;
    for (size_t i = 0; i < diag_msg->values.size; i++) {
        kv_buff_size += diag_msg->values.data[i].key.size + diag_msg->values.data[i].value.size + 7;
    }

    char* kv_buff = static_cast<char*>(pvPortMalloc(kv_buff_size));
    size_t kv_buff_offset = 0;

    constexpr size_t kv_fmt_num = 4;
    char* kv_fmt[kv_fmt_num] = {"\r\n - ", nullptr, ": ", nullptr};
    size_t kv_fmt_sizes[kv_fmt_num] = {5, 0, 2, 0};

    for (size_t i = 0; i < diag_msg->values.size; i++) {
        kv_fmt[1] = diag_msg->values.data->key.data;
        kv_fmt[3] = diag_msg->values.data->value.data;
        kv_fmt_sizes[1] = diag_msg->values.data->key.size;
        kv_fmt_sizes[3] = diag_msg->values.data->value.size;

        for (size_t i = 0; i < kv_fmt_num; i++) {
            assert(kv_fmt_sizes[i] < kv_buff_size - kv_buff_offset);
            memcpy(kv_buff + kv_buff_offset, kv_fmt[i], kv_fmt_sizes[i]);
            kv_buff_offset += kv_fmt_sizes[i];
        }
    }

    kv_buff[kv_buff_offset] = '\0';
    logger.log(__func__, __FILE__, __LINE__, LOG_LVL_WARN, "Diagnostics [%s]: %s%s", diag_msg->name, diag_msg->message, kv_buff);
    vPortFree(kv_buff);
}