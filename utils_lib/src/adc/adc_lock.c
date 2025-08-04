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

#include "utils_lib/adc/adc_lock.h"
#include "hardware/adc.h"
#include "common/opassert.h"

#ifndef UTILS_LIB_NO_FREERTOS
#include "cmsis_gcc.h"
#include "FreeRTOS.h"
#include "semphr.h"
#else
#include "pico/sync.h"
#endif


#ifndef UTILS_LIB_NO_FREERTOS
SemaphoreHandle_t adc_mutex = NULL;
#else
mutex_t adc_mutex;
bool adc_mutex_init = false;
#endif


/* ---- Initialize ADC mutex ---- */
bool adc_init_mutex() {
    #ifndef UTILS_LIB_NO_FREERTOS
    if (adc_mutex == NULL) {
        adc_mutex = xSemaphoreCreateMutex();
        return adc_mutex != NULL;
    }
    #else
    if (!adc_mutex_init) {
        mutex_init(&adc_mutex);
        adc_mutex_init = true;
    }
    #endif

    return true;
}

/* ---- Destroy the ADC mutex ---- */
void adc_destroy_mutex() {
    #ifndef UTILS_LIB_NO_FREERTOS
    if (adc_mutex != NULL) {
        opequal(xSemaphoreTake(adc_mutex, portMAX_DELAY), pdTRUE);
        vSemaphoreDelete(adc_mutex);
        adc_mutex = NULL;
    }
    #else
    (void) 0;  // NOP
    #endif
}

/* ---- Take the ADC mutex ---- */
bool adc_take_mutex() {
    #ifndef UTILS_LIB_NO_FREERTOS
    if (adc_mutex != NULL) {
        if (__get_IPSR() == 0) {   // Check if we are in an ISR or not
            return xSemaphoreTake(adc_mutex, portMAX_DELAY) == pdTRUE;
        }

        // FreeRTOS mutexes cannot be used in ISRs.
        return false;
    }
    #else
    if (adc_mutex_init) {
        mutex_enter_blocking(&adc_mutex);
        return true;
    }
    #endif
    
    return false;
}

/* ---- Release the ADC mutex ---- */
void adc_release_mutex() {
    #ifndef UTILS_LIB_NO_FREERTOS
    if (adc_mutex != NULL) {
        if (__get_IPSR() == 0) {
            opequal(xSemaphoreGive(adc_mutex), pdTRUE);
            return;
        }
    }
    #else
    if (adc_mutex_init) {
        mutex_exit(&adc_mutex);
    }
    #endif
}