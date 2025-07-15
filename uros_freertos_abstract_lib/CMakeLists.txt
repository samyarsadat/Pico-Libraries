#  Pico Libraries (originally from The ROS robot project) - Library top level CMake config
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


# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.13)

# Check whether PICO_SDK_PATH is defined.
if (NOT EXISTS $ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "The Pico SDK path environment variable is not set!")
endif ()

# Include build functions from the Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set project name and C/C++ standards
project(uros_freertos_abstract_lib VERSION 0.1.0 LANGUAGES C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Library options
cmake_policy(SET CMP0077 NEW)
option(UROS_TRANSPORT_NO_USB_CHECK "Disable USB connection check for the microROS transport" OFF)

# Initialize the Pico SDK
pico_sdk_init()

# Add source subdirectory
add_subdirectory(src)

# Library config options
if(DEFINED BRIDGE_EXEC_INTERVAL_MS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC BRIDGE_EXEC_INTERVAL_MS=${BRIDGE_EXEC_INTERVAL_MS})
endif()
if(DEFINED BRIDGE_EXEC_INTERVAL_LIMIT_MS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC BRIDGE_EXEC_INTERVAL_LIMIT_MS=${BRIDGE_EXEC_INTERVAL_LIMIT_MS})
endif()
if(DEFINED MAX_EXECTR_FAIL_RETRY)
    target_compile_definitions(${PROJECT_NAME} PUBLIC MAX_EXECTR_FAIL_RETRY=${MAX_EXECTR_FAIL_RETRY})
endif()
if(DEFINED MAX_EXECUTORS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC MAX_EXECUTORS=${MAX_EXECUTORS})
endif()
if(DEFINED MAX_PUBLISHERS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC MAX_PUBLISHERS=${MAX_PUBLISHERS})
endif()
if(DEFINED MAX_SUBSCRIBERS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC MAX_SUBSCRIBERS=${MAX_SUBSCRIBERS})
endif()
if(DEFINED MAX_SERVICES)
    target_compile_definitions(${PROJECT_NAME} PUBLIC MAX_SERVICES=${MAX_SERVICES})
endif()

# CMake flags
set(FLAGS "-mthumb -ffunction-sections -fdata-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")

message("Configured ${PROJECT_NAME} library build v${PROJECT_VERSION}")