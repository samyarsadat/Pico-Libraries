/*
    Pico Libraries
    Agent object abstraction for FreeRTOS tasks.

    Copyright 2024-2025 Samyar Sadat Akhavi
    Written by Samyar Sadat Akhavi, 2024-2025.
    Heavily inspired by: https://github.com/jondurrant/RPIPicoFreeRTOSuROSPubSub

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

#include "freertos_agent_lib/rtos_agent.h"
#include <string.h>


// Constructor
Agent::Agent(const char* name, configSTACK_DEPTH_TYPE task_stack_depth) {
    this->stack_depth = task_stack_depth;
    strncpy(this->agent_name, name, MAX_NAME_LENGTH - 1);
    this->agent_name[MAX_NAME_LENGTH - 1] = '\0';  // Null-terminate anyway, as strncpy() does not guarantee it.
}

// Destructor
Agent::~Agent() {
    stop();
}

// Start the agent (FreeRTOS task)
// Pass false to set_core_affinity to let FreeRTOS decide.
bool Agent::start(UBaseType_t priority, UBaseType_t core_affinity_mask, bool set_core_affinity) {
    BaseType_t res = xTaskCreate(Agent::vTask, this->agent_name, this->stack_depth, (void*) this, priority, &this->task_handle);

    if (set_core_affinity && res == pdPASS) {
        vTaskCoreAffinitySet(this->task_handle, core_affinity_mask);
        return true;
    }

    return false;
}

// Stop the agent (FreeRTOS task)
void Agent::stop() {
    if (task_handle != nullptr) {
        vTaskDelete(task_handle);
        task_handle = nullptr;
    }
}

// Get the task High Water Mark.
// Close to 0 is an overflow risk.
UBaseType_t Agent::get_high_water_mark() {
    if (task_handle != nullptr) {
        return uxTaskGetStackHighWaterMark(task_handle);
    }

    return 0;
}

// Get the FreeRTOS task that is being used by the agent.
TaskHandle_t Agent::get_rtos_task() {
    return task_handle;
}

// Static internal function used by FreeRTOS to start
// the agent task.
void Agent::vTask(void* parameters) {
    Agent* agent = (Agent*) parameters;

    if (agent != NULL) {
        agent->execute();
    }
}