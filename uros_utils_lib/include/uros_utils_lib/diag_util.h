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

#pragma once
#include "pico/stdlib.h"
#include <rcl/rcl.h>
#include <diagnostic_msgs/msg/diagnostic_status.h>
#include <type_traits>
#include "FreeRTOS.h"


#define KV_CONVERSION_BUFF_SIZE_FLT  50
#define KV_CONVERSION_BUFF_SIZE_INT  12
#define KV_FTOA_DIG_AFTER_DEC_POINT  7

// Diagnostics message levels
enum DIAG_MSG_LEVEL {
    DIAG_LVL_OK    = diagnostic_msgs__msg__DiagnosticStatus__OK,
    DIAG_LVL_WARN  = diagnostic_msgs__msg__DiagnosticStatus__WARN,
    DIAG_LVL_ERROR = diagnostic_msgs__msg__DiagnosticStatus__ERROR,
    DIAG_LVL_STALE = diagnostic_msgs__msg__DiagnosticStatus__STALE
};

#define KEY_CONV_BUFF()                                                      \
    ret_key = static_cast<char*>(pvPortMalloc(KV_CONVERSION_BUFF_SIZE_INT)); \
    assert(this->to_free_char_ptrs_index < this->to_free_char_ptrs_size);    \
    this->to_free_char_ptrs[this->to_free_char_ptrs_index++] = ret_key;

#define VALUE_CONV_BUFF()                                                      \
    ret_value = static_cast<char*>(pvPortMalloc(KV_CONVERSION_BUFF_SIZE_FLT)); \
    assert(this->to_free_char_ptrs_index < this->to_free_char_ptrs_size);      \
    this->to_free_char_ptrs[this->to_free_char_ptrs_index++] = ret_value;

// FTOA function from ftoa.c
#ifdef __cplusplus
extern "C" 
{
#endif
    int ftoa(char* buffer, float value, int digits_after_dec_point);
#ifdef __cplusplus
}
#endif


/*
    Key-value pair array class.
*/
class DiagKvPairs {
    public:
        DiagKvPairs(const size_t capacity);
        ~DiagKvPairs();

        // Base implementation.
        bool add(char* key, char* value);

        // For integer (VT & KT), float (VT only), and boolean (VT only) types.
        template <typename KT, typename VT>
        bool add(KT key, VT value) {    
            char* ret_key;
            char* ret_value;

            if constexpr (std::is_same_v<VT, uint> || std::is_same_v<VT, uint8_t> || std::is_same_v<VT, uint16_t> || std::is_same_v<VT, uint32_t>) {
                VALUE_CONV_BUFF();
                (void) utoa(static_cast<uint>(value), ret_value, 10);
            } else if constexpr (std::is_same_v<VT, int> || std::is_same_v<VT, int16_t> || std::is_same_v<VT, int32_t>) {
                VALUE_CONV_BUFF();
                (void) itoa(static_cast<int>(value), ret_value, 10);
            } else if constexpr (std::is_same_v<VT, float> || std::is_same_v<VT, double>) {
                VALUE_CONV_BUFF();
                (void) ftoa(ret_value, static_cast<float>(value), KV_FTOA_DIG_AFTER_DEC_POINT);
            } else if constexpr (std::is_same_v<VT, bool>) {
                ret_value = value ? "true" : "false";
            } else if constexpr (std::is_same_v<VT, const char*> || std::is_same_v<VT, char*>) {
                assert(value != nullptr);
                ret_value = const_cast<char*>(value);
            } else {
                static_assert(false, "Unsupported value type!");
            }

            if constexpr (std::is_same_v<KT, const char*> || std::is_same_v<KT, char*>) {
                assert(key != nullptr);
                ret_key = const_cast<char*>(key);
            } else if constexpr (std::is_same_v<KT, uint> || std::is_same_v<VT, uint8_t> || std::is_same_v<KT, uint16_t> || std::is_same_v<KT, uint32_t>) {
                KEY_CONV_BUFF();
                (void) utoa(static_cast<uint>(key), ret_key, 10);
            } else if constexpr (std::is_same_v<KT, int> || std::is_same_v<KT, int16_t> || std::is_same_v<KT, int32_t>) {
                KEY_CONV_BUFF();
                (void) itoa(static_cast<int>(key), ret_key, 10);
            } else {
                static_assert(false, "Unsupported key type!");
            }

            return this->add(ret_key, ret_value);
        }

        size_t size();
        diagnostic_msgs__msg__KeyValue* arr_ptr();

    private:
        size_t arr_size = 0, capacity = 0;
        diagnostic_msgs__msg__KeyValue* storage_ptr;
        
        size_t to_free_char_ptrs_size = 0;
        char** to_free_char_ptrs;
        size_t to_free_char_ptrs_index = 0;
};


/*
    Diagnostics publisher util class.
*/
class DiagPublisher {
    public:
        DiagPublisher(const rcl_publisher_t* diag_pub);
        rcl_ret_t publish(const DIAG_MSG_LEVEL level, const char* name, const char* hw_id, const char* msg, DiagKvPairs* kv_pairs, const bool log = true);

    private:
        const rcl_publisher_t* publisher;
        void log_diag_msg(diagnostic_msgs__msg__DiagnosticStatus* diag_msg);
};