/*
    Pico Libraries (originally from The ROS robot project)
    Micro-ROS executor manager for handling multiple executors
    on different threads/cores when using FreeRTOS (singleton object).
    
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
    along with this program.  If not, see <https: www.gnu.org/licenses/>.
*/

#pragma once
#include "freertos_agent_lib/rtos_agent.h"
#include "pico/stdlib.h"
#include <rcl/rcl.h>
#include <rclc/executor.h>
#include <vector>
#include "semphr.h"


// Absolute maximums
#define MAX_SUBSCRIBERS        2
#define MAX_SERVICES           7
#define MAX_EXECTR_FAIL_RETRY  4

// Misc.
#define EXECTR_AGENT_MEMORY  2048


// uROS Bridge Agent class
class uRosExecAgent : public Agent
{
    public:
        struct exectr_timing_conf {
            uint16_t exec_interval_ms = 100;
            uint16_t exec_interval_limit_ms = 125;
            uint16_t exectr_timeout_ms = 40;
        };
        typedef struct exectr_timing_conf exectr_timing_conf_t;

        // Constructor & Destructor
        uRosExecAgent(const char* name, exectr_timing_conf_t* timing_conf);
        virtual ~uRosExecAgent();

        // Agent stop function
        // This function is called by the bridge agent ONLY.
        void stop() override;

        // Initialize MicroROS executor.
        // This function should be called after uros_init_node().
        // This function is NOT thread-safe.
        rcl_ret_t uros_init_executor();

        // Check if the executor is initialized.
        bool is_initialized();

        // Finalize MicroROS node, executor, services, subscriptions, 
        // publishers and timers,and stop the agent.
        // This function is NOT thread-safe.
        void uros_fini();

        // Initialize a subscriber.
        // This function is NOT thread-safe.
        rcl_ret_t init_subscriber(rcl_subscription_t *subscriber, const rosidl_message_type_support_t *type_support, const char *topic_name, UROS_QOS_MODE qos_mode);

        // Initialize a service.
        // This function is NOT thread-safe.
        rcl_ret_t init_service(rcl_service_t *service, const rosidl_service_type_support_t *type_support, const char *service_name, UROS_QOS_MODE qos_mode=QOS_RELIABLE);

        // Add a subscriber to the executor.
        // This function is NOT thread-safe.
        rcl_ret_t add_subscriber(rcl_subscription_t *subscriber, void *msg, rclc_subscription_callback_t callback, rclc_executor_handle_invocation_t invocation=ON_NEW_DATA);

        // Add a service to the executor.
        // This function is NOT thread-safe.
        rcl_ret_t add_service(rcl_service_t *service, void *request, void *response, rclc_service_callback_t callback);

        // Add n amount of handles to the executor.
        // This function is only effective before the executor is started.
        void add_executor_handles(int num_handles);

        // Get the MicroROS executor.
        rclc_executor_t* get_executor();

        // Set a pointer to the managing uROS bridge agent.
        // This function is called ONCE by the bridge agent ONLY.
        void set_bridge_agent(uRosBridgeAgent* bridge_agent);

    private:
        // Hardware timer for execution timing
        struct repeating_timer exec_timer_rt;
        static bool exec_notify_timer_callback(struct repeating_timer *rt);

        uRosBridgeAgent* bridge_instance = nullptr;
        rclc_executor_t rc_executor;
        exectr_timing_conf_t* timing_conf;

        bool node_initialized = false;
        bool executor_initialized = false;

        int executor_handles = 0;
        rcl_subscription_t** subscribers;
        rcl_service_t** services;

    protected:
        // Execution function
        virtual void execute();
};