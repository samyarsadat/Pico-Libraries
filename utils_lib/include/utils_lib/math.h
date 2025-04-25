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
template <typename T>
T map(const T input, const int in_min, const int in_max, const int out_min, const int out_max);

// Adjustable truncate function
template <typename T>
T truncate_adj(const T input, const int trunc_amount);

// Calculates the mean (average) of the numbers in a float vector
template <typename T>
float arr_mean(const vector<T> &numbers);
template <typename T, size_t N>
float arr_mean(const T (&numbers)[N]);

// Calculates the standard deviation of the numbers in a float vector
template <typename T>
float arr_std_dev(const vector<T> &numbers, const float mean);
template <typename T, size_t N>
float arr_std_dev(const T (&numbers)[N], const float mean);

// Finds "outliers" in-between the numbers in a float vector using the Z-Score (Standard Score) method
// It returns a boolean vector (with the same size as the input vector) that indicates the "outliers" by returning their slots as true
template <typename T>
vector<bool> std_score_check(const vector<T> &numbers, const float z_score_threshhold);
template <typename T, size_t N>
void std_score_check(bool (&result)[N], const T (&numbers)[N], const float z_score_threshhold);

// Converts Euler angles to a quaternion
// Output: [x, y, z, w]
template <typename T>
vector<T> euler_to_quaternion(const T roll, const T pitch, const T yaw);
template <typename T, size_t N>
void euler_to_quaternion(const T (&result)[N], const T roll, const T pitch, const T yaw);