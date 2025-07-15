/*
    Pico Libraries - Helper/commonly used functions
    These are general IO functions that can be used in any other program.
    They are not program-specific.
    
    Copyright 2022-2025 Samyar Sadat Akhavi
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

#include "utils_lib/hardware.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"


// ---- Functions ----
// TODO: Better usage information

// Arduino pinMode-like function
void init_pin(uint pin, PIN_CONFIG_MODE_t mode) {
    switch(mode) {
        case OUTPUT:
            gpio_init(pin);
            gpio_set_dir(pin, true);
            break;
        case OUTPUT_PWM:
            gpio_init(pin);
            gpio_set_dir(pin, true);
            gpio_set_function(pin, GPIO_FUNC_PWM);
            pwm_set_enabled(pwm_gpio_to_slice_num(pin), true);
            break;
        case INPUT:
            gpio_init(pin);
            gpio_set_dir(pin, false);
            break;
        case INPUT_PULLUP:
            gpio_init(pin);
            gpio_set_dir(pin, false);
            gpio_pull_up(pin);
            break;
        case INPUT_PULLDOWN:
            gpio_init(pin);
            gpio_set_dir(pin, false);
            gpio_pull_down(pin);
            break;
        case INPUT_ADC:
            adc_gpio_init(pin);
            gpio_set_dir(pin, false);
            break;
        case PROT_I2C:
            gpio_init(pin);
            gpio_set_function(pin, GPIO_FUNC_I2C);
            break;
        case PROT_UART:
            gpio_init(pin);
            gpio_set_function(pin, GPIO_FUNC_UART);
    }
}

// gpio_put function but for PWM-enbaled pins
void gpio_put_pwm(uint pin, uint16_t level) {
    // All A channel PWM pins' pin numbers are even so we can easily check to see whether
    // the pin we are setting is on channel A or channel B.
    if (pin & 1) {
        pwm_set_chan_level(pwm_gpio_to_slice_num(pin), PWM_CHAN_B, level);
    } else {
        pwm_set_chan_level(pwm_gpio_to_slice_num(pin), PWM_CHAN_A, level);
    }
}

// Returns the temperature measured by the RP2040/RP2350's internal sensor in Celsius
// NOTE: The ADC must be initialized and the temperature sensor must be enabled!
float get_proc_temp() {
    adc_select_input(ADC_TEMPERATURE_CHANNEL_NUM);
    const double reading_volts = adc_read() * ADC_CONVERSION_FACTOR;
    const float reading_celsius = 27 - (reading_volts - 0.706) / 0.001721;  // Formula is valid for RP2040 and RP2350.
    return reading_celsius;
}