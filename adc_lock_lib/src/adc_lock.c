/*
    Pico Libraries (originally from The ROS robot project)
    ADC locking library
    
    Copyright 2024-2025 Samyar Sadat Akhavi
    Written by Samyar Sadat Akhavi, 2024-2025.
 
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

#include "adc_lock_lib/adc_lock.h"
#include "hardware/adc.h"
#include "FreeRTOS.h"
#include "semphr.h"


/* ---- Initialize ADC mutex ---- */
SemaphoreHandle_t adc_mutex = NULL;
bool adc_init_mutex() {
    if (adc_mutex == NULL) {
        adc_mutex = xSemaphoreCreateMutex();
        
        if (adc_mutex != NULL) {
            return true;
        }
    }

    return false;
}

/* ---- Destroy the ADC mutex ---- */
void adc_destroy_mutex() {
    if (adc_mutex != NULL) {
        xSemaphoreTake(adc_mutex, portMAX_DELAY);
        vSemaphoreDelete(adc_mutex);
        adc_mutex = NULL;
    }
}

/* ---- Take the ADC mutex ---- */
bool adc_take_mutex() {
    return (adc_mutex != NULL) && (xSemaphoreTake(adc_mutex, portMAX_DELAY) == pdTRUE);
}

/* ---- Release the ADC mutex ---- */
void adc_release_mutex() {
    if (adc_mutex != NULL && xSemaphoreGetMutexHolder(adc_mutex) == xTaskGetCurrentTaskHandle()) {
        xSemaphoreGive(adc_mutex);
    }
}

/* ---- Change ADC mux channel with mutex ---- */
bool adc_select_input_with_mutex(uint8_t channel) {
    if (adc_mutex != NULL && xSemaphoreGetMutexHolder(adc_mutex) == xTaskGetCurrentTaskHandle()) {
        adc_select_input(channel);
        return true;
    }

    return false;
}