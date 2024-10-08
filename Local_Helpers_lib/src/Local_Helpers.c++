/*
    Pico Libraries (originally from The ROS robot project) - Local Helper/commonly used functions
    These are specific MicroROS/IO/etc. functions/definitions that are not 
    designed to be used in any other programs.
    They are program-specific.
    
    Copyright 2024 Samyar Sadat Akhavi
    Written by Samyar Sadat Akhavi, 2024.
 
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


// ------- Libraries & Modules -------
#include "local_helpers_lib/Local_Helpers.h"
#include <rmw_microros/rmw_microros.h>
#include "pico/stdio_uart.h"
#include "pico/stdio_usb.h"
#include "pico/stdio/driver.h"
#include "pico/stdio.h"
#include "semphr.h"
#include "hardware/adc.h"



// ------- Global variables -------
bool enable_print_uart_usb_override = false;



// ------- Functions ------- 

// Note: clean_shutdown() must be defined elsewhere!
extern void clean_shutdown();


// ---- RCL return checker ----
bool check_rc(rcl_ret_t rctc, RT_CHECK_MODE mode, const char *func, uint16_t line)
{
    if (rctc != RCL_RET_OK)
    {
        char buffer[70];

        switch (mode)
        {
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

            default:
                break;
        }
    }

    return (rctc != RCL_RET_OK);
}


// ---- Return checker, except for functions that return a boolean ----
bool check_bool(bool function, RT_CHECK_MODE mode, const char *func, uint16_t line)
{
    if (!function)
    {
        switch (mode)
        {
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

            default:
                break;
        }
    }

    return function;
}


// ---- Initialize ADC mutex ----
SemaphoreHandle_t adc_mutex = NULL;
void adc_init_mutex()
{
    if (adc_mutex == NULL)
    {
        adc_mutex = xSemaphoreCreateMutex();
        
        if (adc_mutex == NULL)
        {
            write_log("ADC mutex creation failed!", LOG_LVL_FATAL, FUNCNAME_ONLY);
            clean_shutdown();
        }
    }
}


// ---- Take the ADC mutex ----
bool adc_take_mutex()
{
    return (xSemaphoreTake(adc_mutex, portMAX_DELAY) == pdTRUE);
}


// ---- Release the ADC mutex ----
void adc_release_mutex()
{
    if (xSemaphoreGetMutexHolder(adc_mutex) == xTaskGetCurrentTaskHandle())
    {
        xSemaphoreGive(adc_mutex);
    }
}


// ---- Change ADC mux channel with mutex ----
bool adc_select_input_with_mutex(uint8_t channel)
{
    TaskHandle_t current_task = xTaskGetCurrentTaskHandle();

    if (xSemaphoreGetMutexHolder(adc_mutex) == current_task)
    {
        adc_select_input(channel);
        return true;
    }

    return false;
}


// ---- Initialize print_uart mutex ----
SemaphoreHandle_t print_uart_mutex = NULL;
void init_print_uart_mutex()
{
    if (print_uart_mutex == NULL)
    {
        print_uart_mutex = xSemaphoreCreateMutex();

        if (print_uart_mutex == NULL)
        {
            // We shouldn't call write_log() here because the mutex isn't initialized.
            clean_shutdown();
        }
    }
}


// ---- Enable print_uart() USB override ----
void print_uart_usb_override()
{
    enable_print_uart_usb_override = true;
}


// ---- Print to STDIO UART function ----
bool print_uart(std::string msg)
{
    // We don't care about the mutex if we're not in a task.
    if (xTaskGetCurrentTaskHandle() == NULL)
    {
        if (enable_print_uart_usb_override)
        {
            stdio_usb.out_chars(msg.c_str(), msg.length());
        }

        else
        {
            stdio_uart.out_chars(msg.c_str(), msg.length());
        }

        return true;
    }

    if (xSemaphoreTake(print_uart_mutex, portMAX_DELAY) == pdTRUE)
    {
        if (enable_print_uart_usb_override)
        {
            stdio_usb.out_chars(msg.c_str(), msg.length());
        }

        else
        {
            stdio_uart.out_chars(msg.c_str(), msg.length());
        }

        xSemaphoreGive(print_uart_mutex);
        return true;
    }

    return false;
}


// ---- Logging function (outputs to STDIO UART) ----
void write_log(std::string msg, LOG_LEVEL lvl, LOG_SOURCE_VERBOSITY src_verb, const char *func, const char *file, uint16_t line)
{
    uint32_t timestamp_sec = to_ms_since_boot(get_absolute_time()) / 1000;
    uint16_t timestamp_millisec = to_ms_since_boot(get_absolute_time()) - (timestamp_sec * 1000);

    std::string level;
    if (lvl == LOG_LVL_INFO) { level = "INFO"; }
    else if (lvl == LOG_LVL_WARN) { level = "WARNING"; }
    else if (lvl == LOG_LVL_ERROR) { level = "ERROR"; }
    else if (lvl == LOG_LVL_FATAL) { level = "FATAL"; }

    // This is rather unfortunate, but its the easiest way.
    std::string filename(file);
    std::string funcname(func);

    std::string src;
    if (src_verb == FUNCNAME_ONLY) { src = func; }
    else if (src_verb == FILENAME_LINE_ONLY) { src = filename + ":" + std::to_string(line); }
    else if (src_verb == FUNCNAME_LINE_ONLY) { src = funcname + ":" + std::to_string(line); }
    else if (src_verb == FILENAME_LINE_FUNCNAME) { src = funcname + "@" + filename + ":" + std::to_string(line); }

    // This is quite ugly, but it works.
    msg = "[" + std::to_string(timestamp_sec) + "." + std::to_string(timestamp_millisec) + "] [" + level + "] [" + src + "]: " + msg + "\r\n";
    print_uart(msg);
}


// ---- Pings the MicroROS agent ----
bool ping_agent()
{
    return (rmw_uros_ping_agent(uros_agent_find_timeout_ms, uros_agent_find_attempts) == RMW_RET_OK);
}


// ---- Execution interval checker ----
// ---- Checks the amount of time passed since the last time it was called (with the specific time storage varialble provided) ----
// ---- Returns false if the execution time has exceeded the specified limit ----
bool check_exec_interval(uint32_t &last_call_time_ms, uint16_t max_exec_time_ms, std::string log_msg, bool publish_diag, const char *func)
{
    // Initialize last_call_time_ms if it's 0 (first call).
    if (last_call_time_ms == 0) 
    { 
        last_call_time_ms = time_us_32() / 1000; 
    }

    uint32_t time_ms = time_us_32() / 1000;
    uint32_t exec_time_ms = time_ms - last_call_time_ms;
    last_call_time_ms = time_ms;
    
    if (exec_time_ms > max_exec_time_ms) 
    {
        // This is also quite ugly, but it also works.
        log_msg = log_msg + " [act: " + std::to_string(exec_time_ms) + "ms, lim: " + std::to_string(max_exec_time_ms) + "ms]";
        write_log(log_msg, LOG_LVL_WARN, FUNCNAME_ONLY, func);

        if (publish_diag)
        {
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