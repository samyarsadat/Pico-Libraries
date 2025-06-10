#  Pico Libraries (originally from The ROS robot project) - Library CMake config
#  Helper functions for using MicroROS with FreeRTOS.  
#
#  Copyright 2024 Samyar Sadat Akhavi
#  Written by Samyar Sadat Akhavi, 2024.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https: www.gnu.org/licenses/>.


# Add source files
add_library(${PROJECT_NAME} uros_bridge.c++
                            uros_executor.c++
                            uros_allocators.c
                            pico_uart_transport.c)

# Include header files
target_include_directories(${PROJECT_NAME} PUBLIC ../include)

# Link to libraries
target_link_libraries(${PROJECT_NAME} pico_stdlib
                                      micro_ros
                                      FreeRTOS-Kernel
                                      uros_utils_lib
                                      utils_lib)

# Enable all warnings
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)