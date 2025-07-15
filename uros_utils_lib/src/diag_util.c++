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

#include "uros_utils_lib/diag_util.h"
#include "pico_log_lib/logger.h"
#include <memory>
#include <string_view>


// This must be declared elsewhere!
extern Logger logger;


/* ---- DiagKvPairs ---- */
DiagKvPairs::DiagKvPairs(const size_t capacity) {
    assert(capacity > 0);
    this->capacity = capacity;
    this->to_free_char_ptrs_size = capacity * 2;  // Only a maximum of 2 buffers are allocated per KV pair.

    this->to_free_char_ptrs = static_cast<char**>(pvPortCalloc(this->to_free_char_ptrs_size, sizeof(char*)));
    this->storage_ptr = static_cast<diagnostic_msgs__msg__KeyValue*>(pvPortCalloc(capacity, sizeof(diagnostic_msgs__msg__KeyValue)));

    for (size_t i = 0; i < this->to_free_char_ptrs_size; i++) {
        this->to_free_char_ptrs[i] = nullptr;
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

size_t DiagKvPairs::size() {
    return this->arr_size;
}

diagnostic_msgs__msg__KeyValue* DiagKvPairs::arr_ptr() {
    return this->storage_ptr;
}

void DiagKvPairs::add(char* key, char* value) {
    assert(key != nullptr && value != nullptr);
    this->storage_ptr[arr_size] = {
        .key = {key, strlen(key), 0},
        .value = {value, strlen(value), 0}
    };
    this->arr_size++;
}


/* ---- DiagPublisher ---- */
DiagPublisher::DiagPublisher(const rcl_publisher_t* diag_pub) {
    assert(diag_pub != nullptr);
    this->publisher = diag_pub;
}

rcl_ret_t DiagPublisher::publish(const DIAG_MSG_LEVEL level, const char* name, const char* hw_id, const char* msg, DiagKvPairs* kv_pairs, const bool log) {
    assert(name != nullptr && hw_id != nullptr && msg != nullptr);
    
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

    #ifndef DIAG_PUBLISHER_DISABLE_PUBLISH
    return rcl_publish(this->publisher, &diag_msg, nullptr);
    #else
    return RCL_RET_OK;
    #endif
}

void DiagPublisher::log_diag_msg(diagnostic_msgs__msg__DiagnosticStatus* diag_msg) {
    assert(diag_msg != nullptr);

    if (diag_msg->values.size == 0) {
        logger.log(__func__, "", __LINE__, LOG_LVL_WARN, "[%s]: %s", diag_msg->name.data, diag_msg->message.data);
        return;
    }

    static constexpr const char* prefix = "\r\n\t- ";
    static constexpr const char* separator = ": ";
    static constexpr size_t prefix_len = std::string_view(prefix).size();
    static constexpr size_t separator_len = std::string_view(separator).size();

    size_t kv_buff_size = 1;
    for (size_t i = 0; i < diag_msg->values.size; i++) {
        kv_buff_size += diag_msg->values.data[i].key.size + diag_msg->values.data[i].value.size + 7;
    }

    char* kv_buff = static_cast<char*>(pvPortMalloc(kv_buff_size));
    char* curr_write_ptr = kv_buff;

    for (size_t i = 0; i < diag_msg->values.size; i++) {
        diagnostic_msgs__msg__KeyValue* kv_ptr = &diag_msg->values.data[i];

        memcpy(curr_write_ptr, prefix, prefix_len);
        curr_write_ptr += prefix_len;

        const size_t key_len = kv_ptr->key.size;
        memcpy(curr_write_ptr, kv_ptr->key.data, key_len);
        curr_write_ptr += key_len;

        memcpy(curr_write_ptr, separator, separator_len);
        curr_write_ptr += separator_len;

        const size_t value_len = kv_ptr->value.size;
        memcpy(curr_write_ptr, kv_ptr->value.data, value_len);
        curr_write_ptr += value_len;
    }

    *curr_write_ptr = '\0';
    logger.log(__func__, "", __LINE__, LOG_LVL_WARN, "[%s]: %s%s", diag_msg->name.data, diag_msg->message.data, kv_buff);
    vPortFree(kv_buff);
}