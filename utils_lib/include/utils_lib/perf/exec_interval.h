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

#pragma once
#include "pico/stdlib.h"


#ifdef __cplusplus
extern "C" 
{
#endif
    /*  
        Execution interval checker.
        Checks the amount of time passed since the last time it was called (with the specific time storage varialble provided).
        Returns false if the execution time has exceeded the specified limit.

        This is a copy of the same function from uros_utils_lib, but with micro-ROS integration removed.
    */
    bool check_exec_interval(uint32_t* last_call_time, const uint16_t max_exec_time_ms, const char* msg, 
                             const char* func, const uint16_t line);
#ifdef __cplusplus
}
#endif

#define CHECK_EXEC_INTERVAL(last_call_time, max_exec_time_ms, msg) \
        check_exec_interval(last_call_time, max_exec_time_ms, msg, __func__, __LINE__);
