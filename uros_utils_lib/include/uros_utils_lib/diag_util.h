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


#define KV_CONVERSION_BUFF_SIZE  50
#define KV_FTOA_DIG_AFTER_DEC_POINT  7

// Diagnostics message levels
enum DIAG_MSG_LEVEL {
    DIAG_LVL_OK    = diagnostic_msgs__msg__DiagnosticStatus__OK,
    DIAG_LVL_WARN  = diagnostic_msgs__msg__DiagnosticStatus__WARN,
    DIAG_LVL_ERROR = diagnostic_msgs__msg__DiagnosticStatus__ERROR,
    DIAG_LVL_STALE = diagnostic_msgs__msg__DiagnosticStatus__STALE
};

#define VALUE_CONV_BUFF()                                                 \
    ret_ptr = static_cast<char*>(pvPortMalloc(KV_CONVERSION_BUFF_SIZE));  \
    assert(this->to_free_char_ptrs_index < this->to_free_char_ptrs_size); \
    this->to_free_char_ptrs[this->to_free_char_ptrs_index++] = ret_ptr;

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

        template <typename KT, typename VT>
        bool add(KT key, VT value) {
            if (this->arr_size < this->capacity) {
                this->add(conv_to_str(key), conv_to_str(value));
                return true;
            }

            return false;
        }

        size_t size();
        diagnostic_msgs__msg__KeyValue* arr_ptr();

    private:
        size_t arr_size = 0, capacity = 0;
        diagnostic_msgs__msg__KeyValue* storage_ptr;
        
        size_t to_free_char_ptrs_size = 0;
        char** to_free_char_ptrs;
        size_t to_free_char_ptrs_index = 0;

        // Base implementation.
        void add(char* key, char* value);

        template<typename T>
        char* conv_to_str(T&& value) {
            using TD = std::decay_t<T>;
            char* ret_ptr = nullptr;
            
            if constexpr (std::is_same_v<TD, bool>) {
                return value ? "true" : "false";
            } else if constexpr (std::is_same_v<TD, const char*> || std::is_same_v<TD, char*>) {
                assert(value != nullptr);
                return const_cast<char*>(value);
            } else if constexpr (is_unsigned_integer_v<TD>) {
                VALUE_CONV_BUFF();
                (void) utoa(static_cast<uint>(value), ret_ptr, 10);
            } else if constexpr (is_signed_integer_v<TD>) {
                VALUE_CONV_BUFF();
                (void) itoa(static_cast<int>(value), ret_ptr, 10);
            } else if constexpr (std::is_floating_point_v<TD>) {
                VALUE_CONV_BUFF();
                (void) ftoa(ret_ptr, static_cast<float>(value), KV_FTOA_DIG_AFTER_DEC_POINT);
            } else {
                static_assert(always_false_v<TD>, "Unsupported type!");
            }

            return ret_ptr;
        }
        
        template<typename T>
        static constexpr bool is_unsigned_integer_v = std::is_integral_v<T> && std::is_unsigned_v<T> && !std::is_same_v<T, bool>;
        template<typename T>
        static constexpr bool is_signed_integer_v = std::is_integral_v<T> && std::is_signed_v<T>;

        template<typename T>
        static constexpr bool always_false_v = false;
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