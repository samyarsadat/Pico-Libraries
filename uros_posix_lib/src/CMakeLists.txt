#  Pico Libraries - Library CMake config
#  Basic POSIX function implementations for uROS on FreeRTOS.
#
#  Copyright 2024-2025 Samyar Sadat Akhavi
#  Written by Samyar Sadat Akhavi, 2024-2025.
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
add_library(${PROJECT_NAME} posix.c)

# Include header files
target_include_directories(${PROJECT_NAME} PUBLIC ../include)

# Link to libraries
target_link_libraries(${PROJECT_NAME} pico_stdlib
                                      FreeRTOS-Kernel)

# Enable all warnings
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)