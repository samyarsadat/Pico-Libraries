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
#include <rcl/rcl.h>
#include <diagnostic_msgs/msg/diagnostic_status.h>


// Diagnostics message levels
enum DIAG_MSG_LEVEL {
    DIAG_LVL_OK    = diagnostic_msgs__msg__DiagnosticStatus__OK,
    DIAG_LVL_WARN  = diagnostic_msgs__msg__DiagnosticStatus__WARN,
    DIAG_LVL_ERROR = diagnostic_msgs__msg__DiagnosticStatus__ERROR,
    DIAG_LVL_STALE = diagnostic_msgs__msg__DiagnosticStatus__STALE
};


/*
    Key-value pair array class.
*/
class DiagKvPairs {
    public:
        DiagKvPairs(const size_t capacity);
        ~DiagKvPairs();

        bool add(char* key, char* value);
        size_t size();
        diagnostic_msgs__msg__KeyValue* arr_ptr();

    private:
        size_t arr_size = 0, capacity = 0;
        diagnostic_msgs__msg__KeyValue* storage_ptr;
};


/*
    Diagnostics publisher util class.
*/
class DiagPublisher {
    public:
        DiagPublisher(const rcl_publisher_t* diag_pub);
        
        void enable_diag(const bool enable);
        bool publish(const DIAG_MSG_LEVEL level, char* hw_name, char* hw_id, char* msg, DiagKvPairs* kv_pairs);

    private:
        const rcl_publisher_t* publisher;
        bool diag_enabled = true;
};