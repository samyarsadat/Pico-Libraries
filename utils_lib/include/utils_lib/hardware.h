/*
    Pico Libraries - Helper/commonly used functions
    These are general IO functions that can be used in any other program.
    They are not program-specific.
    
    Copyright 2022-2024 Samyar Sadat Akhavi
    Written by Samyar Sadat Akhavi, 2022-2025.
 
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
#include "pico/stdlib.h"


// ---- Definitions ----
#define ADC_REF_VOLTAGE        3.26f   // Volts
#define ADC_CONVERSION_FACTOR  (ADC_REF_VOLTAGE / (1 << 12))


#ifdef __cplusplus
extern "C" 
{
#endif
    // Arduino pinMode-like function
    typedef enum {
        OUTPUT, 
        OUTPUT_PWM, 
        INPUT, 
        INPUT_PULLUP, 
        INPUT_PULLDOWN, 
        INPUT_ADC, 
        PROT_I2C, 
        PROT_UART
    } PIN_CONFIG_MODE_t;
    void init_pin(uint pin, PIN_CONFIG_MODE_t mode);

    // gpio_put function but for PWM-enbaled pins
    void gpio_put_pwm(uint pin, uint16_t level);

    // Returns the temperature measured by the RP2040/RP2350's internal sensor in Celsius
    // NOTE: The ADC must be initialized and the temperature sensor must be enabled!
    float get_proc_temp();

    // Returns the ADC channel of a given GPIO pin
    int get_gpio_adc_channel(uint gpio);

    // Reset the chip using the watchdog.
    void watchdog_reset();
#ifdef __cplusplus
}
#endif