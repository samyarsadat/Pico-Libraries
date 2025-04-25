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
#include <rmw_microros/rmw_microros.h>
#include "semphr.h"


// Note: clean_shutdown() must be defined elsewhere!
extern void clean_shutdown();


bool check_rc(rcl_ret_t rctc, RT_CHECK_MODE mode, const char *func, uint16_t line) {
    if (rctc != RCL_RET_OK) {
        char buffer[70];

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
    }

    return (rctc != RCL_RET_OK);
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

bool check_exec_interval(uint32_t &last_call_time_ms, uint16_t max_exec_time_ms, std::string log_msg, bool publish_diag, const char *func) {
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

        if (publish_diag) {
            std::string report_str = log_msg + " [func: " + func + "]";
            std::vector<diag_kv_pair_item_t> kv_pairs;
            std::string actual_time_str = std::to_string(exec_time_ms) + "ms";
            std::string time_limit = std::to_string(max_exec_time_ms) + "ms";
            kv_pairs.push_back(diag_kv_pair_item_t{"actual_time", actual_time_str});
            kv_pairs.push_back(diag_kv_pair_item_t{"time_limit", time_limit});
            publish_diag_report(DIAG_LVL_WARN, DIAG_NAME_SYSTEM, DIAG_ID_SYS_TIMERS, report_str, &kv_pairs);
        }

        return false;
    }

    return true;
}

bool ping_agent() {
    return (rmw_uros_ping_agent(uros_agent_find_timeout_ms, uros_agent_find_attempts) == RMW_RET_OK);
}