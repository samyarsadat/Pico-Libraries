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
#include "uros_utils_lib/general.h"
#include "utils_lib/hardware.h"
#include "pico_log_lib/logger.h"
#include "pico_uart_transports.h"
#include <rclc/rclc.h>
#include <rmw_microros/rmw_microros.h>
#include "hardware/watchdog.h"


// Note: these must be implemented/declared elsewhere.
void system_panic(const char* msg);
extern Logger logger;

// Logging macro
#define LOG(lvl, msg, ...) logger.log(__func__, __FILE__, __LINE__, lvl, msg, ##__VA_ARGS__);


// Constructor
uRosBridgeAgent::uRosBridgeAgent() : Agent(BRIDGE_AGENT_NAME, BRIDGE_AGENT_MEMORY) {
    this->publishers = static_cast<rcl_publisher_t**>(pvPortCalloc(MAX_PUBLISHERS, sizeof(rcl_publisher_t*)));
    this->rc_executors = static_cast<uRosExecAgent**>(pvPortCalloc(MAX_EXECUTORS, sizeof(uRosExecAgent*)));
}

// Destructor
uRosBridgeAgent::~uRosBridgeAgent() {
    if (instance != nullptr) {
        delete instance;
    }

    if (publishers != nullptr) {
        vPortFree(publishers);
    }

    if (rc_executors != nullptr) {
        vPortFree(rc_executors);
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
    
    if (!rcutils_set_default_allocator(&rtos_allocators)) {
        constexpr char* msg = "Failed to set default allocator for micro-ROS!";
        LOG(LOG_LVL_FATAL, msg);
        system_panic(msg);
    }

    // Set MicroROS transport
    if (rmw_uros_set_custom_transport(
        true,
        nullptr,
        pico_serial_transport_open,
        pico_serial_transport_close,
        pico_serial_transport_write,
        pico_serial_transport_read
    ) != RMW_RET_OK) {
        constexpr char* msg = "Failed to set custom transport for micro-ROS!";
        LOG(LOG_LVL_FATAL, msg);
        system_panic(msg);
    }
}

// Initialize MicroROS node and executor.
// This function should be called before any other uROS-related functions.
// This function is NOT thread-safe.
void uRosBridgeAgent::uros_init_node(const char *node_name, const char *name_space, uint8_t node_domain_id) {
    if (!node_initialized) {
        // Initialize the MicroROS allocator
        rcl_allocator = rcl_get_default_allocator();

        // Initialize the MicroROS node
        rcl_init_opts = rcl_get_zero_initialized_init_options();
        RCCHECK(rcl_init_options_init(&rcl_init_opts, rcl_allocator), RC_HARD_CHECK);
        RCCHECK(rcl_init_options_set_domain_id(&rcl_init_opts, (size_t) node_domain_id), RC_HARD_CHECK);
        RCCHECK(rclc_support_init_with_options(&rc_support, 0, nullptr, &rcl_init_opts, &rcl_allocator), RC_HARD_CHECK);
        RCCHECK(rclc_node_init_default(&rc_node, node_name, name_space, &rc_support), RC_HARD_CHECK);
        
        node_initialized = true;
    }
}

// Add a MicroROS executor to the agent.
// The bridge agent will manage the executor.
// This function should be called after uros_init_node().
bool uRosBridgeAgent::uros_add_executor(uRosExecAgent *executor_agent) {
    assert(executor_agent != nullptr);
    
    for (int i = 0; i < MAX_EXECUTORS; i++) {
        if (rc_executors[i] == nullptr) {
            rc_executors[i] = executor_agent;
            return true;
        }
    }

    return false;
}

// Initialize MicroROS executors.
// This function should be called after uros_init_node().
// This function is NOT thread-safe.
void uRosBridgeAgent::uros_init_executors() {
    for (int i = 0; i < MAX_EXECUTORS; i++) {
        if (rc_executors[i] != nullptr && !rc_executors[i]->is_initialized()) {
            rc_executors[i]->set_bridge_agent(this);
            rc_executors[i]->uros_init_executor();
        }
    }
}

// Finalize MicroROS node, executor, services, subscriptions,
// publishers and timers, and stop the agent.
// This function is NOT thread-safe.
void uRosBridgeAgent::uros_fini() {
    current_uros_state = AGENT_DISCONNECTED;
    
    for (int i = 0; i < MAX_PUBLISHERS; i++) {
        if (publishers[i] != nullptr) {
            (void) rcl_publisher_fini(publishers[i], &rc_node);
            publishers[i] = nullptr;
        }
    }

    for (int i = 0; i < MAX_EXECUTORS; i++) {
        if (rc_executors[i] != nullptr && rc_executors[i]->is_initialized()) {
            rc_executors[i]->uros_fini();
            rc_executors[i] = nullptr;
        }
    }

    (void) rcl_node_fini(&rc_node);
    (void) rclc_support_fini(&rc_support);
}

// Initialize a publisher.
// Call this before uros_init_executor().
// This function is NOT thread-safe.
rcl_ret_t uRosBridgeAgent::init_publisher(rcl_publisher_t *publisher, const rosidl_message_type_support_t *type_support, const char *topic_name, UROS_QOS_MODE qos_mode) {
    rcl_ret_t ret_code = -1;
    
    for (int i = 0; i < MAX_PUBLISHERS; i++) {
        if (publishers[i] != nullptr) {
            *publisher = rcl_get_zero_initialized_publisher();
            
            if (qos_mode == QOS_RELIABLE) {
                ret_code = rclc_publisher_init_default(publisher, &rc_node, type_support, topic_name);
            } else {
                ret_code = rclc_publisher_init_best_effort(publisher, &rc_node, type_support, topic_name);
            }
        }

        publishers[i] = publisher;
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

// Get the MicroROS executor.
rclc_executor_t* uRosBridgeAgent::get_executor(uint8_t num) {
    return rc_executors[num]->get_executor();
}

// Get the MicroROS agent state.
uRosBridgeAgent::UROS_STATE uRosBridgeAgent::get_agent_state() {
    return current_uros_state;
}

// Main execution function.
void uRosBridgeAgent::execute() {
    LOG(LOG_LVL_DEBUG, "Starting micro-ROS bridge notification timer...");
    add_repeating_timer_ms(AGENT_STATE_MACHINE_EXEC_INTERVAL_MS, 
                           uRosBridgeAgent::exec_notify_timer_callback, (void *) this, &exec_timer_rt);

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
                init_func();
                current_uros_state = AGENT_CONNECTED;
                break;
            case AGENT_CONNECTED:
                current_uros_state = ping_agent() ? AGENT_CONNECTED : AGENT_DISCONNECTED;
                check_exec_interval(last_exec_time, AGENT_STATE_MACHINE_EXEC_INTERVAL_MS + 10, 
                                    "Agent state machine exec interval exceeded.", true);
                break;
            case AGENT_DISCONNECTED:
                LOG(LOG_LVL_WARN, "Micro-ROS agent disconnected! Preparing for reset...");
                cancel_repeating_timer(&exec_timer_rt);
                
                // Must return at some point! Preferably, quickly.
                fini_func();

                LOG(LOG_LVL_INFO, "Cleanup completed. Resetting system...");
                watchdog_reset();
                break;
        }
    }
}

// PRIVATE: Executor notification timer callback.
bool uRosBridgeAgent::exec_notify_timer_callback(struct repeating_timer *rt) {
    uRosBridgeAgent* bridge_agent = (uRosBridgeAgent*) rt->user_data;
    assert(bridge_agent != nullptr);

    BaseType_t higher_prio_woken;
    vTaskNotifyGiveFromISR(uRosBridgeAgent::get_instance()->get_rtos_task(), &higher_prio_woken);
    portYIELD_FROM_ISR(higher_prio_woken);
    return true;
}