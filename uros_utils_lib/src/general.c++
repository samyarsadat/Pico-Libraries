/*
    Pico Libraries (originally from The ROS robot project)
    Micro-ROS helper/commonly used functions
    
    Copyright 2024-2025 Samyar Sadat Akhavi
    Written by Samyar Sadat Akhavi, 2024-2025.
 
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

#include "uros_utils_lib/general.h"
#include "uros_utils_lib/diag_helper.h"
#include "uros_common/definitions.h"
#include "pico_log_lib/logger.h"
#include <rmw_microros/rmw_microros.h>
#include "semphr.h"
#include <stdlib.h>


// Note: these must be implemented/declared elsewhere.
void system_panic(const char* msg);
extern Logger logger;
extern DiagPublisher diag_util;


#define ADD_RC_DIAG_KVS()       \
    DiagKvPairs diag_kvs(3);    \
    diag_kvs.add("code", rctc); \
    diag_kvs.add("func", func); \
    diag_kvs.add("line", line);

bool RCCHECK(const rcl_ret_t rctc, const RC_CHECK_MODE mode, char* func, const char* file, const uint16_t line) {
    if (rctc != RCL_RET_OK) {
        if (mode == RC_SOFT_CHECK) {
            ADD_RC_DIAG_KVS();
            diag_util.publish(DIAG_LVL_WARN, DIAG_NAME_SYSTEM, DIAG_ID_SYS_UROS, DIAG_WARN_UROS_RCL_FAIL, &diag_kvs);
        } else if (mode == RC_HARD_CHECK) {
            ADD_RC_DIAG_KVS();
            diag_util.publish(DIAG_LVL_ERROR, DIAG_NAME_SYSTEM, DIAG_ID_SYS_UROS, DIAG_ERR_UROS_RCL_FAIL, &diag_kvs);
            system_panic(DIAG_ERR_UROS_RCL_FAIL);
        } else {
            logger.log(func, file, line, LOG_LVL_ERROR, "RCL failure with code: %d", rctc);
        }

        return false;
    }

    return true;
}

bool check_exec_interval(uint32_t &last_call_time, const uint16_t max_exec_time_ms, const char* msg, bool pub_diag,
                         const char* func, const char* file, const uint16_t line) {
    uint32_t current_time = time_us_32();
    
    // Initialize last_call_time_ms if it's 0 (first call).
    if (last_call_time == 0) { 
        last_call_time = current_time; 
    }

    uint32_t exec_time_ms = (current_time - last_call_time) / 1000;
    last_call_time = current_time;
    
    if (exec_time_ms > max_exec_time_ms) {
        if (pub_diag) {
            DiagKvPairs diag_kvs(3);
            diag_kvs.add("exec_time_ms", exec_time_ms);
            diag_kvs.add("limit_ms", max_exec_time_ms);
            diag_kvs.add("func", func);
            diag_util.publish(DIAG_LVL_WARN, DIAG_NAME_SYSTEM, DIAG_ID_SYS_TIMERS, msg, &diag_kvs);
        } else {
            logger.log(func, file, line, LOG_LVL_WARN, "%s [actual: %ums, limit: %ums]", msg, exec_time_ms, max_exec_time_ms);
        }

        return false;
    }

    return true;
}

bool ping_agent(const int timeout_ms, const uint8_t attempts) {
    return (rmw_uros_ping_agent(timeout_ms, attempts) == RMW_RET_OK);
}