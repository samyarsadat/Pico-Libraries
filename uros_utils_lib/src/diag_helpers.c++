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
#include "FreeRTOS.h"


/* ---- DiagKvPairs ---- */
DiagKvPairs::DiagKvPairs(const size_t capacity) {
    this->storage_ptr = static_cast<diagnostic_msgs__msg__KeyValue*>(pvPortMallocStack(capacity));
    assert(this->storage_ptr != nullptr);
    
    if (this->storage_ptr != nullptr){
        this->capacity = capacity;
    }
}

DiagKvPairs::~DiagKvPairs() {
    if (this->storage_ptr != nullptr) {
        vPortFreeStack(this->storage_ptr);
    }
}

bool DiagKvPairs::add(char* key, char* value) {
    if (this->arr_size < this->capacity) {
        this->storage_ptr[arr_size] = {
            .key = {key, strlen(key)},
            .value = {value, strlen(value)}
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

bool DiagPublisher::publish(const DIAG_MSG_LEVEL level, char* hw_name, char* hw_id, char* msg, DiagKvPairs* kv_pairs) {
    if (this->diag_enabled) {
        diagnostic_msgs__msg__DiagnosticStatus diag_msg;
        
        diag_msg.level = level;
        diag_msg.name.data = hw_name;
        diag_msg.name.size = strlen(hw_name);
        diag_msg.message.data = msg;
        diag_msg.message.size = strlen(msg);
        diag_msg.hardware_id.data = hw_id;
        diag_msg.hardware_id.size = strlen(hw_id);
        
        if (kv_pairs != nullptr) {
            diag_msg.values.data = kv_pairs->arr_ptr();
            diag_msg.values.size = kv_pairs->size();
        } else {
            diag_msg.values.data = nullptr;
            diag_msg.values.size = 0;
        }

        return rcl_publish(this->publisher, &diag_msg, nullptr) == RCL_RET_OK;
    }

    return true;
}