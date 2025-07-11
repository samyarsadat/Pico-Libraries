/*
    Pico Libraries
    Micro-ROS self-test diagnostics message helper functions.
    
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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "uros_utils_lib/selftest_resp_helpers.h"
#include <diagnostic_msgs/msg/key_value.h>
#include <rosidl_runtime_c/string_functions.h>
#include "common/opassert.h"


void selftest_resp_free(diagnostic_msgs__srv__SelfTest_Response* resp) {
    if (resp == NULL) {
        assert(false);
        return;
    }

    rosidl_runtime_c__String__fini(&resp->id);
    diagnostic_msgs__msg__DiagnosticStatus__Sequence__fini(&resp->status);
}

void selftest_resp_init(diagnostic_msgs__srv__SelfTest_Response* resp, const char* id, size_t status_capacity) {
    if (resp == NULL) {
        assert(false);
        return;
    }

    opassert(rosidl_runtime_c__String__init(&resp->id));
    opassert(rosidl_runtime_c__String__assign(&resp->id, id));
    opassert(diagnostic_msgs__msg__DiagnosticStatus__Sequence__init(&resp->status, status_capacity));

    for (size_t i = 0; i < status_capacity; i++) {
        diagnostic_msgs__msg__DiagnosticStatus* status = &resp->status.data[i];
        opassert(rosidl_runtime_c__String__init(&status->name));
        opassert(rosidl_runtime_c__String__init(&status->message));
        opassert(rosidl_runtime_c__String__init(&status->hardware_id));
    }
}

void selftest_stat_set(diagnostic_msgs__msg__DiagnosticStatus* status, const char* hardware_id, 
                       const char* name, const char* message, uint8_t level) {
    assert(status != NULL);
    opassert(rosidl_runtime_c__String__assign(&status->hardware_id, hardware_id));
    opassert(rosidl_runtime_c__String__assign(&status->name, name));
    opassert(rosidl_runtime_c__String__assign(&status->message, message));
    status->level = level;
}

void selftest_stat_kv_init(diagnostic_msgs__msg__DiagnosticStatus* status, size_t kv_capacity) {
    assert(status != NULL);
    opassert(diagnostic_msgs__msg__KeyValue__Sequence__init(&status->values, kv_capacity));

    for (size_t i = 0; i < kv_capacity; i++) {
        diagnostic_msgs__msg__KeyValue* kv = &status->values.data[i];
        opassert(rosidl_runtime_c__String__init(&kv->key));
        opassert(rosidl_runtime_c__String__init(&kv->value));
    }
}

void selftest_stat_kv_set(diagnostic_msgs__msg__KeyValue* kv, const char* key, const char* value) {
    assert(kv != NULL);
    opassert(rosidl_runtime_c__String__assign(&kv->key, key));
    opassert(rosidl_runtime_c__String__assign(&kv->value, value));
}