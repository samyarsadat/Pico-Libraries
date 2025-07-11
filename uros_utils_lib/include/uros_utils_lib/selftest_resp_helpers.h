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

#include "pico/stdlib.h"
#include <diagnostic_msgs/srv/self_test.h>
#include <diagnostic_msgs/msg/diagnostic_status.h>


#ifdef __cplusplus
extern "C" 
{
#endif
    void selftest_resp_free(diagnostic_msgs__srv__SelfTest_Response* resp);
    void selftest_resp_init(diagnostic_msgs__srv__SelfTest_Response* resp, const char* id, size_t status_capacity);
    void selftest_stat_set(diagnostic_msgs__msg__DiagnosticStatus* status, const char* hardware_id, const char* name, const char* message, uint8_t level);
    void selftest_stat_kv_init(diagnostic_msgs__msg__DiagnosticStatus* status, size_t kv_capacity);
    void selftest_stat_kv_set(diagnostic_msgs__msg__KeyValue* kv, const char* key, const char* value);
#ifdef __cplusplus
}
#endif