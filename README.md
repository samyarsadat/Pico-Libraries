<h1 align="center">Pico Libraries</h1>

<p align="center">
	<a href="LICENSE"><img src="https://img.shields.io/github/license/samyarsadat/Pico-Libraries?color=blue"></a>
	|
	<a href="../../issues"><img src="https://img.shields.io/github/issues/samyarsadat/Pico-Libraries"></a>
	<br><br>
</p>

<br><br>

----
This repository contains a set of C/C++ helper/utility libraries that I have written for the Raspberry Pi Pico.<br>
I use these in some of my other projects.<br>
<br>

> [!NOTE]
> You can add this repository as a submodule to use these libraries.

<br><br>

## Included Libraries

#### Helpers_lib
This library contains some generic helper functions for the Raspberry Pi Pico (RP2040).<br>
These can be used in pretty much any RP2040 project.

#### Local_Helpers_lib
This library contains more project-specific helper functions. 
I use these in projects where I'm using FreeRTOS and MicroROS. 

#### FreeRTOS_Helpers_lib
This library contains generic FreeRTOS helpers/utilities.
(Currently there's only a task abstraction class, but more will be added as needed.)

#### MicroROS_FreeRTOS_Helpers_lib
This library contians uttilities for using MicroROS alongside FreeRTOS.
These are necessary as MicroROS does not work well with FreeRTOS SMP on the Pico as of writing this.
(MicroROS currently does not have official support for FreeRTOS on the Pico)

<br>

## Contact
You can contact me via e-mail.<br>
E-mail: samyarsadat@gigawhat.net<br>
<br>
If you think that you have found a bug or issue please report it <a href="../../issues">here</a>.

<br>

## Contributing
Please take a look at <a href="CONTRIBUTING.md">CONTRIBUTING.md</a> for contributing.

<br>

## Credits
| Role           | Name                                                             |
| -------------- | ---------------------------------------------------------------- |
| Lead Developer | <a href="https://github.com/samyarsadat">Samyar Sadat Akhavi</a> |

<br>
<br>

Copyright © 2024 Samyar Sadat Akhavi.