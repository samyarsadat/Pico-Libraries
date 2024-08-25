/*
    Pico Libraries (originally from The ROS robot project) - Local Helper/commonly used functions
    Common diagnostics Hardware Name/ID definitions
    
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


// ------- Diagnostics Hardware Names -------
#define DIAG_NAME_SYSTEM         "system"   // Firmware-related diagnostics
#define DIAG_NAME_MCU            "mcus"


// ------- Diagnostics Hardware IDs -------

// ---- System ----
#define DIAG_ID_SYS_GENERAL     "system_general"
#define DIAG_ID_SYS_TIMERS      "system_timers"
#define DIAG_ID_SYS_UROS        "microros"