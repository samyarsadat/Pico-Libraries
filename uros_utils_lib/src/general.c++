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
#include "pico_log_lib/logger.h"
#include <rmw_microros/rmw_microros.h>
#include "semphr.h"


// Note: clean_shutdown() must be defined elsewhere!
extern void clean_shutdown();
extern Logger logger;


bool check_rc(rcl_ret_t rctc, RT_CHECK_MODE mode, const char *func, uint16_t line) {
    if (rctc != RCL_RET_OK) {
        switch (mode) {
            case RT_HARD_CHECK:
                snprintf(buffer, sizeof(buffer), "RCL Return check failed: [code: %d, RT_HARD_CHECK]", rctc);
                write_log(buffer, LOG_LVL_FATAL, FUNCNAME_LINE_ONLY, func, "", line);
                publish_diag_report(DIAG_LVL_ERROR, DIAG_NAME_SYSTEM, DIAG_ID_SYS_UROS, DIAG_ERR_MSG_UROS_RC_CHECK_FAIL, NULL);
                clean_shutdown();
                break;
            case RT_SOFT_CHECK:
                snprintf(buffer, sizeof(buffer), "RCL Return check failed: [code: %d, RT_SOFT_CHECK]", rctc);
                write_log(buffer, LOG_LVL_ERROR, FUNCNAME_LINE_ONLY, func, "", line);
                publish_diag_report(DIAG_LVL_WARN, DIAG_NAME_SYSTEM, DIAG_ID_SYS_UROS, DIAG_WARN_MSG_UROS_RC_CHECK_FAIL, NULL);
                break;
            case RT_LOG_ONLY_CHECK:
                snprintf(buffer, sizeof(buffer), "RCL Return check failed: [code: %d, RT_LOG_ONLY_CHECK]", rctc);
                write_log(buffer, LOG_LVL_WARN, FUNCNAME_LINE_ONLY, func, "", line);
                break;
        }

        return false;
    }

    return true;
}

bool check_bool(bool function, RT_CHECK_MODE mode, const char *func, uint16_t line) {
    if (!function) {
        switch (mode) {
            case RT_HARD_CHECK:
                write_log("BOOL Return check failed: [RT_HARD_CHECK]", LOG_LVL_FATAL, FUNCNAME_LINE_ONLY, func, "", line);
                publish_diag_report(DIAG_LVL_ERROR, DIAG_NAME_SYSTEM, DIAG_ID_SYS_GENERAL, DIAG_ERR_MSG_BOOL_RT_CHECK_FAIL, NULL);
                clean_shutdown();
                break;
            case RT_SOFT_CHECK:
                write_log("BOOL Return check failed: [RT_SOFT_CHECK]", LOG_LVL_ERROR, FUNCNAME_LINE_ONLY, func, "", line);
                publish_diag_report(DIAG_LVL_WARN, DIAG_NAME_SYSTEM, DIAG_ID_SYS_GENERAL, DIAG_WARN_MSG_BOOL_RT_CHECK_FAIL, NULL);
                break;
            case RT_LOG_ONLY_CHECK:
                write_log("BOOL Return check failed: [RT_LOG_ONLY_CHECK]", LOG_LVL_WARN, FUNCNAME_LINE_ONLY, func, "", line);
                break;
        }
    }

    return function;
}

bool check_exec_interval(uint32_t &last_call_time_ms, const uint16_t max_exec_time_ms, const char* log_msg, bool pub_diag,
                         const char* func, const char* file, const uint16_t line) {
    // Initialize last_call_time_ms if it's 0 (first call).
    if (last_call_time_ms == 0) { 
        last_call_time_ms = time_us_32() / 1000; 
    }

    uint32_t time_ms = time_us_32() / 1000;
    uint32_t exec_time_ms = time_ms - last_call_time_ms;
    last_call_time_ms = time_ms;
    
    if (exec_time_ms > max_exec_time_ms) {
        // This is also quite ugly, but it also works.
        log_msg = log_msg + " [act: " + std::to_string(exec_time_ms) + "ms, lim: " + std::to_string(max_exec_time_ms) + "ms]";
        write_log(log_msg, LOG_LVL_WARN, FUNCNAME_ONLY, func);

        /* DIAG PUB */

        return false;
    }

    return true;
}

bool ping_agent(const int timeout_ms, const uint8_t attempts) {
    return (rmw_uros_ping_agent(timeout_ms, attempts) == RMW_RET_OK);
}