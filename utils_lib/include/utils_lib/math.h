/*
    Pico Libraries - Helper/commonly used functions
    These are general math functions that can be used in any other program.
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
#include <vector>
using namespace std;


// Arduino map-like function
float map(float input, int in_min, int in_max, int out_min, int out_max);

// Adjustable truncate function
float truncate_adj(float input, int trunc_amount);

// Calculates the mean (average) of the numbers in a float vector
float calculate_mean(vector<float> &numbers);

// Calculates the standard deviation of the numbers in a float vector
float calculate_standard_deviation(vector<float> &numbers, float numbers_mean);

// Finds "outliers" in-between the numbers in a float vector using the Z-Score (Standard Score) method
// It returns a boolean vector (with the same size as the input vector) that indicates the "outliers" by returning their slots as true
vector<bool> standard_score_check(vector<float> &numbers, float z_score_threshhold);

// Converts Euler angles to a quaternion
// Output: [x, y, z, w]
vector<float> euler_to_quaternion(float roll, float pitch, float yaw);