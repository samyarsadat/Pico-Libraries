/*
    Pico Libraries - Helper/commonly used functions
    These are general misc. functions that can be used in any other program.
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

#include "utils_lib/misc.h"
using namespace std;


// ------- Functions -------
// TODO: Better usage information

// Get a string representation of a boolean array
string bool_array_as_str(bool bool_array[], uint16_t array_size) {
    string ret_str = "[";
    for (uint16_t i = 0; i < array_size; i++) {
        if (i < array_size - 1) {
            ret_str += bool_array[i] ? "true, " : "false, ";
        } else {
            ret_str += bool_array[i] ? "true]" : "false]";
        }
    }

    return ret_str;
}