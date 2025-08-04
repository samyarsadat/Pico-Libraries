/*
    Pico Libraries - Utility functions
    Execution interval cheking utility.
    
    Copyright 2025 Samyar Sadat Akhavi.
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

#include "utils_lib/perf/exec_interval.h"
#include "pico_log_lib/logger.h"


// Note: these must be implemented/declared elsewhere.
extern Logger logger;


bool check_exec_interval(uint32_t* last_call_time, uint16_t max_exec_time_ms, const char* msg,
                         const char* func, const uint16_t line) {
    assert(last_call_time != nullptr && msg != nullptr);
    const uint32_t current_time = time_us_32();
    
    // Initialize last_call_time_ms if it's 0 (first call).
    if (*last_call_time == 0) { 
        *last_call_time = current_time;
        return true;
    }

    uint32_t exec_time_ms = (current_time - *last_call_time) / 1000;
    *last_call_time = current_time;
    
    if (exec_time_ms > max_exec_time_ms) {
        logger.log(func, "", line, LOG_LVL_WARN, "%s [actual: %ums, limit: %ums]", msg, exec_time_ms, max_exec_time_ms);
        return false;
    }

    return true;
}