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
#include "uros_freertos_abstract_lib/uros_allocators.h"
#include "uros_utils_lib/general.h"
#include "pico_uart_transports.h"
#include <rclc/rclc.h>
#include <rmw_microros/rmw_microros.h>
#include "FreeRTOS.h"


// Constructor
uRosExecAgent::uRosExecAgent(const char* name, exectr_timing_conf_t* timing_conf) : Agent(name, EXECTR_AGENT_MEMORY) {
    this->timing_conf = timing_conf;
    this->subscribers = static_cast<rcl_subscription_t**>(pvPortCalloc(MAX_SUBSCRIBERS, sizeof(rcl_subscription_t*)));
    this->services = static_cast<rcl_service_t**>(pvPortCalloc(MAX_SERVICES, sizeof(rcl_service_t*)));
    this->timers = static_cast<rcl_timer_t**>(pvPortCalloc(MAX_TIMERS, sizeof(rcl_timer_t*)));
}

// Destructor
uRosExecAgent::~uRosExecAgent() {
    if (subscribers != nullptr) {
        vPortFree(subscribers);
    }

    if (services != nullptr) {
        vPortFree(services);
    }

    if (timers != nullptr) {
        vPortFree(timers);
    }
}

// Initialize MicroROS executor.
// This function should be called after uros_init_node().
// This function is NOT thread-safe.
rcl_ret_t uRosExecAgent::uros_init_executor() {
    if (!this->executor_initialized) {
        // Initialize the MicroROS executor
        this->rc_executor = rclc_executor_get_zero_initialized_executor();
        rcl_ret_t ret_code = rclc_executor_init(&this->rc_executor, &this->bridge_instance->get_support()->context, 
                                                this->executor_handles, this->bridge_instance->get_allocator());
        if (ret_code == RCL_RET_OK) {
            executor_initialized = true;
        }
        
        return ret_code;
    }
}

// Finalize MicroROS executor, services, subscriptions, and timers.
// This function is NOT thread-safe.
void uRosExecAgent::uros_fini() {
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

    for (int i = 0; i < MAX_TIMERS; i++) {
        if (this->timers[i] != nullptr) {
            (void) rclc_executor_remove_timer(&this->rc_executor, this->timers[i]);
            (void) rcl_timer_fini(timers[i]);
            this->timers[i] = nullptr;
        }
    }

    (void) rclc_executor_fini(&rc_executor);
}

// Initialize a subscriber.
// Call this before uros_init_executor().
// This function is NOT thread-safe.
rcl_ret_t uRosExecAgent::init_subscriber(rcl_subscription_t *subscriber, const rosidl_message_type_support_t *type_support, const char *topic_name, UROS_QOS_MODE qos_mode) {
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
        }
    }

    return ret_code;
}

// Initialize a service.
// Call this before uros_init_executor().
// This function is NOT thread-safe.
rcl_ret_t uRosExecAgent::init_service(rcl_service_t *service, const rosidl_service_type_support_t *type_support, const char *service_name, UROS_QOS_MODE qos_mode) {
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
        }
    }

    return ret_code;
}

// Initialize a timer.
// Call this before uros_init_executor().
// This function is NOT thread-safe.
rcl_ret_t uRosExecAgent::init_timer(rcl_timer_t *timer, uint64_t period, rcl_timer_callback_t callback, bool autostart) {
    rcl_ret_t ret_code = -1;

    for (int i = 0; i < MAX_TIMERS; i++) {
        if (this->timers[i] == nullptr) {
            *timer = rcl_get_zero_initialized_timer();
            ret_code = rclc_timer_init_default2(timer, this->bridge_instance->get_support(), RCL_MS_TO_NS(period), callback, autostart);
            
            if (ret_code == RCL_RET_OK) {
                this->timers[i] = timer;
                this->executor_handles ++;
            }
        }
    }

    return ret_code;
}


// Add a subscriber to the executor.
// Call this after uros_init_executor().
// This function is NOT thread-safe.
rcl_ret_t uRosExecAgent::add_subscriber(rcl_subscription_t *subscriber, void *msg, rclc_subscription_callback_t callback, rclc_executor_handle_invocation_t invocation) {
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
// Call this after uros_init_executor().
// This function is NOT thread-safe.
rcl_ret_t uRosExecAgent::add_service(rcl_service_t *service, void *request, void *response, rclc_service_callback_t callback) {
    rcl_ret_t ret_code = -1;

    for (int i = 0; i < MAX_SERVICES; i++) {
        if (this->services[i] == service) {
            ret_code = rclc_executor_add_service(&this->rc_executor, service, request, response, callback);
            break;
        }
    }

    return ret_code;
}


// Add a timer to the executor.
// Call this after uros_init_executor().
// This function is NOT thread-safe.
rcl_ret_t uRosExecAgent::add_timer(rcl_timer_t *timer) {
    rcl_ret_t ret_code = -1;
    
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (this->timers[i] == timer) {
            ret_code = rclc_executor_add_timer(&this->rc_executor, timer);
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
    }
}

// Get the MicroROS executor.
rclc_executor_t* uRosExecAgent::get_executor() {
    return &this->rc_executor;
}

// Main execution function.
void uRosExecAgent::execute() {
    write_log("Starting MicroROS executor notification timer...", LOG_LVL_INFO, FUNCNAME_ONLY);
    add_repeating_timer_ms(exec_interval_ms, uRosExecAgent::exec_notify_timer_callback, (void *) this, &exec_timer_rt);

    uint32_t last_exec_time = 0;
    current_uros_state = WAITING_FOR_AGENT;
    write_log("Waiting for agent...", LOG_LVL_INFO, FUNCNAME_ONLY);

    while (true) {
        xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);   // Wait for notification indefinitely

        switch (current_uros_state) {
            case WAITING_FOR_AGENT:
                current_uros_state = ping_agent() ? AGENT_AVAILABLE:WAITING_FOR_AGENT;
                break;
            case AGENT_AVAILABLE:
                write_log("Agent available!", LOG_LVL_INFO, FUNCNAME_ONLY);
                check_bool(init_func(), RT_HARD_CHECK);
                current_uros_state = AGENT_CONNECTED;
                break;
            case AGENT_CONNECTED:
                current_uros_state = ping_agent() ? AGENT_CONNECTED:AGENT_DISCONNECTED;
                
                if (current_uros_state == AGENT_CONNECTED) {
                    check_exec_interval(last_exec_time, exec_interval_limit_ms, "Executor execution time exceeded limits!", true);
                    bool exec_failed = check_rc(rclc_executor_spin_some(&rc_executor, RCL_MS_TO_NS(exectr_timeout_ms)), RT_LOG_ONLY_CHECK);

                    if (post_exec_func != NULL && !exec_failed) {
                        post_exec_func();
                    }
                }

                break;
            case AGENT_DISCONNECTED:
                write_log("Agent disconnected!", LOG_LVL_INFO, FUNCNAME_ONLY);
                cancel_repeating_timer(&exec_timer_rt);
                fini_func();
                break;
        }
    }
}


// PRIVATE: Executor notification timer callback.
bool uRosExecAgent::exec_notify_timer_callback(struct repeating_timer *rt) {
    uRosExecAgent *bridge_agent = (uRosExecAgent *) rt->user_data;

    if (bridge_agent != NULL) {
        BaseType_t higher_prio_woken;
        vTaskNotifyGiveFromISR(uRosExecAgent::get_instance()->get_rtos_task(), &higher_prio_woken);
        portYIELD_FROM_ISR(higher_prio_woken);
        return true;
    }

    return false;
}