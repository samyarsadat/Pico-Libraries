/*
    Pico Libraries (originally from The ROS robot project)
    uROS Bridge singleton object for managing MicroROS and MicroROS comms.
    This object handles the MicroROS executor and the MicroROS node.
    It also provides methods for initializing publishers, subscribers, and services.
    
    Copyright 2024 Samyar Sadat Akhavi
    Written by Samyar Sadat Akhavi, 2024.
    Inspired by: https://github.com/jondurrant/RPIPicoFreeRTOSuROSPubSub
 
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

#include "uros_freertos_abstract_lib/uros_bridge.h"
#include "uros_freertos_abstract_lib/uros_executor.h"
#include "uros_freertos_abstract_lib/internal/uros_allocators.h"
#include "uros_freertos_abstract_lib/internal/pico_uart_transport.h"
#include "pico_log_lib/logger.h"
#include "uros_utils_lib/misc.h"
#include <rclc/rclc.h>
#include <rmw_microros/rmw_microros.h>
#include "FreeRTOS.h"
#include <common/opassert.h>


// Note: these must be implemented/declared elsewhere.
extern Logger logger;

// Logging macro
#define LOG(lvl, msg, ...) logger.log(__func__, "", __LINE__, lvl, msg, ##__VA_ARGS__);


// Constructor
uRosExecAgent::uRosExecAgent(const char* name, exectr_timing_conf_t* timing_conf) : Agent(name, EXECTR_AGENT_MEMORY) {
    this->timing_conf = timing_conf;
    this->rc_executor = rclc_executor_get_zero_initialized_executor();

    #if MAX_SUBSCRIBERS > 0
    this->subscribers = static_cast<rcl_subscription_t**>(pvPortCalloc(MAX_SUBSCRIBERS, sizeof(rcl_subscription_t*)));
    #endif
    
    #if MAX_SERVICES > 0
    this->services = static_cast<rcl_service_t**>(pvPortCalloc(MAX_SERVICES, sizeof(rcl_service_t*)));
    #endif

    strcpy(this->exec_sys_name, EXECTR_SYSNAME_PATH);
    strcat(this->exec_sys_name, this->agent_name);
}

// Destructor
uRosExecAgent::~uRosExecAgent() {
    assert(!this->executor_initialized);

    if (subscribers != nullptr) {
        vPortFree(subscribers);
    }

    if (services != nullptr) {
        vPortFree(services);
    }
}

// Initialize MicroROS executor.
// This function should be called after uros_init_node().
// This function is NOT thread-safe.
rcl_ret_t uRosExecAgent::uros_init_executor() {
    assert(this->bridge_instance != nullptr);

    if (!this->executor_initialized) {
        // Initialize the MicroROS executor
        rcl_ret_t ret_code = rclc_executor_init(&this->rc_executor, &this->bridge_instance->get_support()->context, 
                                                this->executor_handles, this->bridge_instance->get_allocator());
        if (ret_code == RCL_RET_OK) {
            executor_initialized = true;
        }
        
        return ret_code;
    }

    return RCL_RET_OK;
}

// Check if the executor is initialized.
bool uRosExecAgent::is_initialized() {
    return this->executor_initialized;
}

// Finalize MicroROS executor, services, and subscriptions.
// This function is NOT thread-safe.
// This must be called before the object is destroyed.
void uRosExecAgent::uros_fini() {
    if (this->executor_initialized) {
        assert(this->bridge_instance != nullptr);
        (void) cancel_repeating_timer(&this->exec_timer_rt);  // Calling this on a cancelled timer is safe.
        this->stop();

        for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
            if (this->subscribers[i] != nullptr) {
                (void) rclc_executor_remove_subscription(&this->rc_executor, this->subscribers[i]);
                (void) rcl_subscription_fini(subscribers[i], this->bridge_instance->get_node());
                this->subscribers[i] = nullptr;
            }
        }

        for (int i = 0; i < MAX_SERVICES; i++) {
            if (this->services[i] != nullptr) {
                (void) rclc_executor_remove_service(&this->rc_executor, this->services[i]);
                (void) rcl_service_fini(services[i], this->bridge_instance->get_node());
                this->services[i] = nullptr;
            }
        }

        (void) rclc_executor_fini(&rc_executor);
        this->executor_initialized = false;
        LOG(LOG_LVL_INFO, "Micro-ROS executor %s finalized.", this->agent_name);
    }
}

// Initialize a subscriber.
// Call this before uros_init_executor().
// This function is NOT thread-safe.
rcl_ret_t uRosExecAgent::init_subscriber(rcl_subscription_t *subscriber, const rosidl_message_type_support_t *type_support, const char *topic_name, UROS_QOS_MODE qos_mode) {
    assert(this->bridge_instance != nullptr);
    rcl_ret_t ret_code = -1;

    for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
        if (this->subscribers[i] == nullptr) {
            *subscriber = rcl_get_zero_initialized_subscription();

            if (qos_mode == QOS_RELIABLE) {
                ret_code = rclc_subscription_init_default(subscriber, this->bridge_instance->get_node(), type_support, topic_name);
            } else {
                ret_code = rclc_subscription_init_best_effort(subscriber, this->bridge_instance->get_node(), type_support, topic_name);
            }

            if (ret_code == RCL_RET_OK) {
                this->subscribers[i] = subscriber;
                this->executor_handles ++;
            }

            break;
        }
    }

    return ret_code;
}

// Initialize a service.
// Call this before uros_init_executor().
// This function is NOT thread-safe.
rcl_ret_t uRosExecAgent::init_service(rcl_service_t *service, const rosidl_service_type_support_t *type_support, const char *service_name, UROS_QOS_MODE qos_mode) {
    assert(this->bridge_instance != nullptr);
    rcl_ret_t ret_code = -1;
    
    for (int i = 0; i < MAX_SERVICES; i++) {
        if (this->services[i] == nullptr) {
            *service = rcl_get_zero_initialized_service();

            if (qos_mode == QOS_RELIABLE) {
                ret_code = rclc_service_init_default(service, this->bridge_instance->get_node(), type_support, service_name);
            } else {
                ret_code = rclc_service_init_best_effort(service, this->bridge_instance->get_node(), type_support, service_name);
            }

            if (ret_code == RCL_RET_OK) {
                this->services[i] = service;
                this->executor_handles ++;
            }

            break;
        }
    }

    return ret_code;
}

// Add a subscriber to the executor.
// This function is NOT thread-safe.
rcl_ret_t uRosExecAgent::add_subscriber(rcl_subscription_t *subscriber, void *msg, rclc_subscription_callback_t callback, rclc_executor_handle_invocation_t invocation) {
    assert(this->executor_initialized);
    rcl_ret_t ret_code = -1;

    for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
        if (this->subscribers[i] == subscriber) {
            ret_code = rclc_executor_add_subscription(&this->rc_executor, subscriber, msg, callback, invocation);
            break;
        }
    }

    return ret_code;
}

// Add a service to the executor.
// This function is NOT thread-safe.
rcl_ret_t uRosExecAgent::add_service(rcl_service_t *service, void *request, void *response, rclc_service_callback_t callback) {
    assert(this->executor_initialized);
    rcl_ret_t ret_code = -1;

    for (int i = 0; i < MAX_SERVICES; i++) {
        if (this->services[i] == service) {
            ret_code = rclc_executor_add_service(&this->rc_executor, service, request, response, callback);
            break;
        }
    }

    return ret_code;
}

// Add n amount of handles to the executor.
// This function is only effective before the executor is started.
void uRosExecAgent::add_executor_handles(int num_handles) {
    if (!this->executor_initialized) {
        this->executor_handles += num_handles;
        return;
    }

    assert(false);
}

// Get the MicroROS executor.
rclc_executor_t* uRosExecAgent::get_executor() {
    return &this->rc_executor;
}

// Set a pointer to the managing uROS bridge agent.
// This function is called ONCE by the bridge agent ONLY.
void uRosExecAgent::set_bridge_agent(uRosBridgeAgent* bridge_agent) {
    assert(bridge_agent != nullptr);
    assert(this->bridge_instance == nullptr);
    this->bridge_instance = bridge_agent;
}

// Get the micro-ROS system name of the executor.
char* uRosExecAgent::get_executor_sysname() {
    return this->exec_sys_name;
}

// Main execution function.
void uRosExecAgent::execute() {
    assert(this->executor_initialized);

    LOG(LOG_LVL_INFO, "Starting micro-ROS executor notification timer (%s)...", this->agent_name);
    opassert(add_repeating_timer_ms(this->timing_conf->exec_interval_ms, 
                                    uRosExecAgent::exec_notify_timer_callback, 
                                    (void *) this, &exec_timer_rt));
    
    uint32_t last_exec_time = 0;
    uint8_t exec_fail_retry = 0;

    while (true) {
        xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);   // Wait for notification indefinitely

        rcl_ret_t ret_code = rclc_executor_spin_some(&rc_executor, RCL_MS_TO_NS(this->timing_conf->exectr_timeout_ms));
        CHECK_EXEC_INTERVAL(last_exec_time, this->timing_conf->exec_interval_limit_ms, 
                            "Executor execution time exceeded limits!", this->exec_sys_name, true);

        if (ret_code != RCL_RET_OK) {
            if (exec_fail_retry == MAX_EXECTR_FAIL_RETRY) {
                LOG(LOG_LVL_FATAL, "Maximum executor spin failure retries reached! Stopping executor %s!", this->agent_name);
                (void) cancel_repeating_timer(&exec_timer_rt);
                this->bridge_instance->notify_executor_failure(this, ret_code, exec_fail_retry);
                this->stop();
            }

            LOG(LOG_LVL_ERROR, "Failed to spin executor %s! Error: %d, retry: %d", this->agent_name, ret_code, exec_fail_retry);
            exec_fail_retry++;
        }

        taskYIELD();
    }
}

// PRIVATE: Executor notification timer callback.
bool uRosExecAgent::exec_notify_timer_callback(struct repeating_timer *rt) {
    assert(rt->user_data != nullptr);
    TaskHandle_t agent_task = static_cast<uRosExecAgent*>(rt->user_data)->get_rtos_task();
    
    if (agent_task != nullptr) {
        BaseType_t higher_prio_woken;
        vTaskNotifyGiveFromISR(agent_task, &higher_prio_woken);
        portYIELD_FROM_ISR(higher_prio_woken);
    }

    return true;
}