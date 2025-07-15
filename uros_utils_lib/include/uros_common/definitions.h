/*
    Pico Libraries (originally from The ROS robot project)
    Common program definitions
    
    Copyright 2024 Samyar Sadat Akhavi
    Written by Samyar Sadat Akhavi, 2024.
 
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
#include "uros_common/diag_msgs.h"

// Defualt firmware hardware ID. Override this!
#ifndef DIAG_FIRMWARE_HARDWARE_ID
#define DIAG_FIRMWARE_HARDWARE_ID  "pico-fw-v1.0_2025-01-01_DEFAULT"
#endif