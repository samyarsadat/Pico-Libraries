<h1 align="center">Pico Libraries</h1>

<p align="center">
	<a href="LICENSE"><img src="https://img.shields.io/github/license/samyarsadat/Pico-Libraries?color=blue"></a>
	|
	<a href="../../issues"><img src="https://img.shields.io/github/issues/samyarsadat/Pico-Libraries"></a>
	<br><br>
</p>

<br>

----
This repository contains a set of C/C++ helper/utility libraries that I have written for the Raspberry Pi Pico.\
I use these in some of my other projects.

> [!NOTE]
> You can add this repository as a submodule to use these libraries.

<br>

## Included Libraries

> [!WARNING]
> All of these libraries are currently undergoing testing! They aren't ready for use in "production".

> [!NOTE]
> All of these libraries support both the RP2040 and the RP2350.

> [!WARNING]
> Note that any libraries using FreeRTOS' memory allocation functions (`pvPortMalloc()` and `pvPortCalloc()`) assume that these functions never return null, and so they do not check for it.
>
> To make sure that these functions never return null in case of an allocation failure, set `configUSE_MALLOC_FAILED_HOOK` to `1` in your FreeRTOS config, and ensure that your `vApplicationMallocFailedHook` function NEVER returns.
>
> A heap allocation failure is considered critical, and soft recovery from it is usually not practical on embedded platforms. Resetting the microcontroller is usually recommended in such cases.

<br>

### `cpp_freertos_alloc_lib`
A library for overriding C++'s default `new` and `delete` operators with variants that use memory allocation function provided by FreeRTOS.

<br>

### `freertos_agent_lib`
FreeRTOS task abstraction library. Provides an `Agent` class that can be used to manage a FreeRTOS task.

<br>

### `pico_log_lib`
See `pico_log_lib`'s GitHub repository [here](https://github.com/samyarsadat/Pico-Log-Library).

<br>

### `uros_freertos_abstract_lib`
An abstraction library for integrating microROS with FreeRTOS. It makes configuring microROS and managing its executors easier.

<br>

### `uros_utils_lib`
Provides certain utilities and helpers for microROS. Currently, it includes:
 - A diagnostics publishing abstraction with a key-value pair abstraction.
 - Execution time monitoring and alerting function.
 - MicroROS agent `ping()` function.

<br>

### `utils_lib`
Generic utility and helper functions for use with the Pico. Cuurently, it includes:
 - Mathematical utilities.
   - Value range scaling/mapping function (same as `map()` from Arduino's standard functions).
   - Floating point number precision truncation function.
   - Statistical functions (mean, standard deviation, z-score check).
   - Euler to quaternion conversion function.
 - Hardware utilities.
   - Pin initialization helper (similar to `pinMode()` from Arduino).
   - GPIO PWM output setting helper.
   - Get processor temperature function.
   - Get GPIO ADC channel function.
 - Miscellaneous utilities.
   - Function for getting a `std::string` representation of a Boolean array.

<br>

## Contact
You can contact me via e-mail.\
E-mail: samyarsadat@gigawhat.net\

If you think that you have found a bug or issue please report it [here](../../issues).

<br>

## Contributing
Please take a look at [CONTRIBUTING.md](CONTRIBUTING.md) for contributing.

<br>

## Credits
| Role           | Name                                                             |
| -------------- | ---------------------------------------------------------------- |
| Lead Developer | <a href="https://github.com/samyarsadat">Samyar Sadat Akhavi</a> |

<br>

Copyright © 2024-2025 Samyar Sadat Akhavi.