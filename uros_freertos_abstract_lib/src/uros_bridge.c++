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
#include "uros_freertos_abstract_lib/internal/uros_allocators.h"
#include "uros_freertos_abstract_lib/internal/pico_uart_transport.h"
#include "uros_utils_lib/misc.h"
#include "uros_utils_lib/diag_util.h"
#include "uros_common/definitions.h"
#include "uros_common/diag_msgs.h"
#include "utils_lib/hardware.h"
#include "pico_log_lib/logger.h"
#include <rclc/rclc.h>
#include <rmw_microros/rmw_microros.h>
#include "hardware/watchdog.h"
#include "common/opassert.h"


// Note: these must be implemented/declared elsewhere.
extern Logger logger;
extern DiagPublisher diag_util;

// Logging macro
#define LOG(lvl, msg, ...) logger.log(__func__, "", __LINE__, lvl, msg, ##__VA_ARGS__);


// Constructor
uRosBridgeAgent::uRosBridgeAgent() : Agent(BRIDGE_AGENT_NAME, BRIDGE_AGENT_MEMORY) {
    this->publishers = static_cast<rcl_publisher_t**>(pvPortCalloc(MAX_PUBLISHERS, sizeof(rcl_publisher_t*)));
    this->rc_executors = static_cast<uRosExecAgent**>(pvPortCalloc(MAX_EXECUTORS, sizeof(uRosExecAgent*)));

    this->rcl_init_opts = rcl_get_zero_initialized_init_options();
    this->rc_node = rcl_get_zero_initialized_node();

    memset(this->init_ret_codes, -1, sizeof(this->init_ret_codes));
}

// Destructor
uRosBridgeAgent::~uRosBridgeAgent() {
    if (publishers != nullptr) {
        vPortFree(publishers);
    }

    if (rc_executors != nullptr) {
        vPortFree(rc_executors);
    }

    if (instance != nullptr) {
        delete instance;
    }
}

// Get the singleton instance
uRosBridgeAgent *uRosBridgeAgent::instance = nullptr;
uRosBridgeAgent *uRosBridgeAgent::get_instance() {
    if (instance == nullptr) {
        instance = new uRosBridgeAgent();
    }

    return instance;
}

// Pre-init configuration
void uRosBridgeAgent::configure(uros_init_function init_function, uros_fini_function fini_function) {
    this->init_func = init_function;
    this->fini_func = fini_function;

    // Set MicroROS default allocators
    rcl_allocator_t rtos_allocators = rcutils_get_zero_initialized_allocator();
    rtos_allocators.allocate = uros_rtos_allocate;
    rtos_allocators.deallocate = uros_rtos_deallocate;
    rtos_allocators.reallocate = uros_rtos_reallocate;
    rtos_allocators.zero_allocate = uros_rtos_zero_allocate;
    
    // rcutils_set_default_allocator only checks for allocator validity.
    opassert(rcutils_set_default_allocator(&rtos_allocators));

    // Set MicroROS transport
    // rmw_uros_set_custom_transport only checks for nullptr arguments.
    opequal(rmw_uros_set_custom_transport(
        true,
        nullptr,
        pico_serial_transport_open,
        pico_serial_transport_close,
        pico_serial_transport_write,
        pico_serial_transport_read
    ), RMW_RET_OK);
}

// Initialize the MicroROS node.
// This function should be called before any other uROS-related functions.
// This function is NOT thread-safe.
rcl_ret_t uRosBridgeAgent::uros_init_node(const char *node_name, const char *name_space, uint8_t node_domain_id) {
    if (this->init_ret_codes[2] != RCL_RET_OK) {
        // Initialize the MicroROS allocator
        this->rcl_allocator = rcl_get_default_allocator();

        // Initialize the MicroROS node
        this->init_ret_codes[0] = rcl_init_options_init(&this->rcl_init_opts, this->rcl_allocator);
        opequal(rcl_init_options_set_domain_id(&this->rcl_init_opts, (size_t) node_domain_id), RCL_RET_OK);
        this->init_ret_codes[1] = rclc_support_init_with_options(&this->rc_support, 0, nullptr, &this->rcl_init_opts, &this->rcl_allocator);
        this->init_ret_codes[2] = rclc_node_init_default(&this->rc_node, node_name, name_space, &this->rc_support);

        for (int i = 0; i < UROS_INIT_RET_CODE_COUNT; i++) {
            if (this->init_ret_codes[i] != RCL_RET_OK) {
                LOG(LOG_LVL_ERROR, "Failed to initialize micro-ROS node (num: %d)!", i);
                return this->init_ret_codes[i];
            }
        }
    }

    return RCL_RET_OK;
}

// Add a MicroROS executor to the agent.
// The bridge agent will manage the executor.
bool uRosBridgeAgent::uros_add_executor(uRosExecAgent *executor_agent) {
    assert(executor_agent != nullptr);
    
    for (int i = 0; i < MAX_EXECUTORS; i++) {
        if (rc_executors[i] == nullptr) {
            rc_executors[i] = executor_agent;
            executor_agent->set_bridge_agent(this);
            return true;
        }
    }

    return false;
}

// Initialize MicroROS executors.
// This function should be called after uros_init_node().
// This function is NOT thread-safe.
rcl_ret_t uRosBridgeAgent::uros_init_executors() {
    assert(this->init_ret_codes[2] == RCL_RET_OK);
    rcl_ret_t ret_code;

    for (int i = 0; i < MAX_EXECUTORS; i++) {
        if (rc_executors[i] != nullptr && !rc_executors[i]->is_initialized()) {
            ret_code = rc_executors[i]->uros_init_executor();

            if (ret_code != RCL_RET_OK) {
                LOG(LOG_LVL_ERROR, "Failed to initialize micro-ROS executor %s!", rc_executors[i]->get_agent_name());
                return ret_code;
            }
        }
    }

    return RCL_RET_OK;
}

// Set the agent disconnect flag to true.
void uRosBridgeAgent::disconnect_agent() {
    LOG(LOG_LVL_INFO, "Disconnecting micro-ROS agent...");
    this->disco_agent_flag = true;
}

// Finalize MicroROS node, executor, services, subscriptions,
// publishers and timers, and stop the agent.
// This function is NOT thread-safe.
// Only call it from the bridge fini function.
void uRosBridgeAgent::uros_fini() {
    (void) cancel_repeating_timer(&this->exec_timer_rt);
    this->stop();

    for (int i = 0; i < MAX_PUBLISHERS; i++) {
        if (this->publishers[i] != nullptr) {
            (void) rcl_publisher_fini(this->publishers[i], &this->rc_node);
            this->publishers[i] = nullptr;
        }
    }

    for (int i = 0; i < MAX_EXECUTORS; i++) {
        if (this->rc_executors[i] != nullptr) {
            this->rc_executors[i]->uros_fini();
            this->rc_executors[i] = nullptr;
        }
    }

    if (this->init_ret_codes[2] == RCL_RET_OK) {
        (void) rcl_node_fini(&this->rc_node);
        this->rc_node = rcl_get_zero_initialized_node();
        this->init_ret_codes[2] = -1;
    }

    if (this->init_ret_codes[1] == RCL_RET_OK) {
        (void) rclc_support_fini(&this->rc_support);
        this->init_ret_codes[1] = -1;
    }

    if (this->init_ret_codes[0] == RCL_RET_OK) {
        (void) rcl_init_options_fini(&this->rcl_init_opts);
        this->rcl_init_opts = rcl_get_zero_initialized_init_options();
        this->init_ret_codes[0] = -1;
    }
}

// Initialize a publisher.
// Call this after uros_init_executor().
// This function is NOT thread-safe.
rcl_ret_t uRosBridgeAgent::init_publisher(rcl_publisher_t *publisher, const rosidl_message_type_support_t *type_support, const char *topic_name, UROS_QOS_MODE qos_mode) {
    assert(this->init_ret_codes[2] == RCL_RET_OK);
    rcl_ret_t ret_code = -1;
    
    for (int i = 0; i < MAX_PUBLISHERS; i++) {
        if (publishers[i] == nullptr) {
            *publisher = rcl_get_zero_initialized_publisher();
            
            if (qos_mode == QOS_RELIABLE) {
                ret_code = rclc_publisher_init_default(publisher, &rc_node, type_support, topic_name);
            } else {
                ret_code = rclc_publisher_init_best_effort(publisher, &rc_node, type_support, topic_name);
            }
        
            if (ret_code == RCL_RET_OK) {
                publishers[i] = publisher;
            }

            break;
        }
    }

    return ret_code;
}

// Get the MicroROS allocator.
rcl_allocator_t* uRosBridgeAgent::get_allocator() {
    return &rcl_allocator;
}

// Get the MicroROS node.
rcl_node_t* uRosBridgeAgent::get_node() {
    return &rc_node;
}

// Get the MicroROS support.
rclc_support_t* uRosBridgeAgent::get_support() {
    return &rc_support;
}

// Get a MicroROS executor.
rclc_executor_t* uRosBridgeAgent::get_executor(uint8_t num) {
    if (num < MAX_EXECUTORS) {
        return rc_executors[num]->get_executor();
    }

    assert(false);
    return nullptr;
}

// Get the MicroROS agent state.
uRosBridgeAgent::UROS_STATE uRosBridgeAgent::get_agent_state() {
    return current_uros_state;
}

// This gets called by the executors if they suffer a failure.
void uRosBridgeAgent::notify_executor_failure(uRosExecAgent *executor, rcl_ret_t code, uint8_t retries) {
    assert(executor != nullptr);
    LOG(LOG_LVL_ERROR, "Executor failure! Disconnecting & stopping agent...");
    this->disco_agent_flag = true;

    DiagKvPairs diag_kvs(3);
    diag_kvs.add("code", code);
    diag_kvs.add("retry_count", retries);
    diag_kvs.add("name", executor->get_agent_name());
    (void) diag_util.publish(DIAG_LVL_ERROR, "microros/executors", DIAG_FIRMWARE_HARDWARE_ID, "fatal micro-ROS executor failure", &diag_kvs);
}

// Main execution function.
void uRosBridgeAgent::execute() {
    LOG(LOG_LVL_DEBUG, "Starting micro-ROS bridge notification timer...");
    opassert(add_repeating_timer_ms(AGENT_STATE_MACHINE_EXEC_INTERVAL_MS, 
                                    uRosBridgeAgent::exec_notify_timer_callback, 
                                    (void *) this, &exec_timer_rt));

    uint32_t last_exec_time = 0;
    current_uros_state = WAITING_FOR_AGENT;
    LOG(LOG_LVL_INFO, "Waiting for micro-ROS agent...");

    while (true) {
        xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY);   // Wait for notification indefinitely

        switch (current_uros_state) {
            case WAITING_FOR_AGENT:
                current_uros_state = ping_agent() ? AGENT_AVAILABLE : WAITING_FOR_AGENT;
                break;
            case AGENT_AVAILABLE:
                LOG(LOG_LVL_INFO, "Micro-ROS agent available!");
                current_uros_state = init_func() ? AGENT_CONNECTED : AGENT_DISCONNECTED;
                break;
            case AGENT_CONNECTED:
                current_uros_state = (!this->disco_agent_flag && ping_agent()) ? AGENT_CONNECTED : AGENT_DISCONNECTED;
                check_exec_interval(last_exec_time, AGENT_STATE_MACHINE_EXEC_INTERVAL_MS + 10, 
                                    "Agent state machine exec interval exceeded.", "microros/bridge", true);
                break;
            case AGENT_DISCONNECTED:
                (void) cancel_repeating_timer(&exec_timer_rt);
                LOG(LOG_LVL_WARN, "Micro-ROS agent disconnected!");

                LOG(LOG_LVL_INFO, "Stopping micro-ROS executors...");
                for (int i = 0; i < MAX_EXECUTORS; i++) {
                    if (rc_executors[i] != nullptr) {
                        rc_executors[i]->stop();
                    }
                }
                
                fini_func();
                break;
        }

        taskYIELD();
    }
}

// PRIVATE: Executor notification timer callback.
bool uRosBridgeAgent::exec_notify_timer_callback(struct repeating_timer *rt) {
    uRosBridgeAgent* bridge_agent = (uRosBridgeAgent*) rt->user_data;
    assert(bridge_agent != nullptr);
    TaskHandle_t agent_task = bridge_agent->get_rtos_task();
    
    if (agent_task != nullptr) {
        BaseType_t higher_prio_woken;
        vTaskNotifyGiveFromISR(agent_task, &higher_prio_woken);
        portYIELD_FROM_ISR(higher_prio_woken);
    }

    return true;
}