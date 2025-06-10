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

#pragma once
#include "freertos_agent_lib/rtos_agent.h"
#include "uros_freertos_abstract_lib/uros_executor.h"
#include "uros_freertos_abstract_lib/common.h"
#include "pico/stdlib.h"
#include <rcl/rcl.h>
#include <rclc/executor.h>
#include <vector>


// Absolute maximums
#define MAX_EXECUTORS   2
#define MAX_PUBLISHERS  8

// Agent config
#define BRIDGE_AGENT_MEMORY                   2048   // Words
#define BRIDGE_AGENT_NAME                     "uros_bridge_agent"
#define AGENT_STATE_MACHINE_EXEC_INTERVAL_MS  200

// Misc.
#define UROS_INIT_RET_CODE_COUNT  4


// uROS Bridge Agent class
class uRosBridgeAgent : public Agent
{
    public:
        // MicroROS init & fini (pubs, subs, services, timers, executor, node, etc.) function typedefs
        typedef void (*uros_init_function)(void);
        typedef void (*uros_fini_function)(void);

        // MicroROS agent state enum
        enum UROS_STATE {
            WAITING_FOR_AGENT,
            AGENT_AVAILABLE,
            AGENT_CONNECTED,
            AGENT_DISCONNECTED
        };

        // Get the singleton instance
        static uRosBridgeAgent* get_instance();

        // Pre-init configuration
        rmw_ret_t configure(uros_init_function init_function, uros_fini_function fini_function);

        // Initialize MicroROS node.
        // This function should be called before any other uROS-related functions.
        // This function is NOT thread-safe.
        rcl_ret_t uros_init_node(const char *node_name, const char *name_space, uint8_t node_domain_id);

        // Add a MicroROS executor to the agent.
        // The bridge agent will manage the executor.
        bool uros_add_executor(uRosExecAgent *executor_agent);

        // Initialize MicroROS executors.
        // This function should be called after uros_init_node().
        // This function is NOT thread-safe.
        rcl_ret_t uros_init_executors();

        // Finalize MicroROS node, executor, services, subscriptions, 
        // publishers and timers, and stop the agent.
        // This function is NOT thread-safe.
        void uros_fini();

        // Initialize a publisher.
        // This function is NOT thread-safe.
        rcl_ret_t init_publisher(rcl_publisher_t *publisher, const rosidl_message_type_support_t *type_support, 
                                 const char *topic_name, UROS_QOS_MODE qos_mode = QOS_BEST_EFFORT);

        // Get the MicroROS allocator.
        rcl_allocator_t* get_allocator();

        // Get the MicroROS node.
        rcl_node_t* get_node();

        // Get the MicroROS support.
        rclc_support_t* get_support();

        // Get the MicroROS executor.
        rclc_executor_t* get_executor(uint8_t num);

        // Get the MicroROS agent state.
        uRosBridgeAgent::UROS_STATE get_agent_state();

        // This gets called by the executors if they suffer a failure.
        void notify_executor_failure(uRosExecAgent *executor, rcl_ret_t code, uint8_t retries);

    private:
        // Constructor & Destructor
        uRosBridgeAgent();
        virtual ~uRosBridgeAgent();
        static uRosBridgeAgent *instance;

        // Hardware timer for agent state machine
        struct repeating_timer exec_timer_rt;
        static bool exec_notify_timer_callback(struct repeating_timer *rt);

        // Initialize function
        uros_init_function init_func;
        uros_fini_function fini_func;

        // MicroROS agent state
        UROS_STATE current_uros_state;

        rcl_init_options_t rcl_init_opts;
        rcl_allocator_t rcl_allocator;
        rcl_node_t rc_node;
        rclc_support_t rc_support;
        rcl_ret_t init_ret_codes[UROS_INIT_RET_CODE_COUNT];

        bool node_initialized = false;
        rcl_publisher_t** publishers = nullptr;
        uRosExecAgent** rc_executors = nullptr;
        bool exec_failed_flag = false;

    protected:
        // Execution function
        virtual void execute();
};