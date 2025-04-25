/*
    Pico Libraries - Helper/commonly used functions
    These are general math functions that can be used in any other program.
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

#include "utils_lib/math.h"
#include <math.h>
using namespace std;


// ------- Functions -------
// TODO: Better usage information

// Arduino map-like function
template <typename T>
T map(const T input, const int in_min, const int in_max, const int out_min, const int out_max) {
    return static_cast<T>((input - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

// Adjustable truncate function
template <typename T>
T truncate_adj(T input, int trunc_amount) {
    return static_cast<T>(round(input * pow(10, trunc_amount)) / pow(10, trunc_amount));
}

// Calculates the mean (average) of the numbers in a float vector/array
template <typename T>
float arr_mean(const vector<T> &numbers) {
    float total = 0.0f;
    for (const T &num : numbers) {
        total += num;
    }

    return total / numbers.size();
}

template <typename T, size_t N>
float arr_mean(const T (&numbers)[N]) {
    float total = 0.0f;
    for (size_t i = 0, i < N, i++) {
        total += numbers[i];
    }

    return total / N;
}

// Calculates the standard deviation of the numbers in a float vector/array
template <typename T>
float arr_std_dev(const vector<T> &numbers, const float mean) {
    float deviation_total = 0.0f;
    for (const T &num : numbers) {
        deviation_total += pow(num - mean, 2);
    }

    return sqrt(deviation_total / numbers.size());
}

template <typename T, size_t N>
float arr_std_dev(const T (&numbers)[N], const float mean) {
    float deviation_total = 0.0f;
    for (size_t i = 0, i < N, i++) {
        deviation_total += pow(numbers[i] - mean, 2);
    }

    return sqrt(deviation_total / N);
}

// Finds "outliers" in-between the numbers in a float vector/array using the Z-Score (Standard Score) method
// It returns a boolean vector/array (with the same size as the input vector/array) that indicates the "outliers" by returning their slots as true
template <typename T>
vector<bool> std_score_check(const vector<T> &numbers, const float z_score_threshhold) {
    const float mean = arr_mean(numbers);
    const float std_dev = arr_std_dev(numbers, mean);
    vector<bool> result;
    
    for (const T &num : numbers) {
        result.push_back(abs((num - mean) / std_dev) > z_score_threshhold);
    }

    return result;
}

template <typename T, size_t N>
void std_score_check(bool (&result)[N], const T (&numbers)[N], const float z_score_threshhold) {
    const float mean = arr_mean(numbers);
    const float std_dev = arr_std_dev(numbers, mean);
    
    for (size_t i = 0, i < N, i++) {
        result[i] = (abs((numbers[i] - mean) / std_dev) > z_score_threshhold);
    }
}

// Converts Euler angles to a quaternion
// Output: [x, y, z, w]
template <typename T>
vector<T> euler_to_quaternion(const T roll, const T pitch, const T yaw) {
    vector<T> result;
    
    T h_yaw = yaw * static_cast<T>(0.5);
    T h_pitch = pitch * static_cast<T>(0.5);
    T h_roll = roll * static_cast<T>(0.5);

    T cy = cos(h_yaw);
    T sy = sin(h_yaw);
    T cp = cos(h_pitch);
    T sp = sin(h_pitch);
    T cr = cos(h_roll);
    T sr = sin(h_roll);

    result.resize(4);
    result[0] = sr * cp * cy - cr * sp * sy;  // x
    result[1] = cr * sp * cy + sr * cp * sy;  // y
    result[2] = cr * cp * sy - sr * sp * cy;  // z
    result[3] = cr * cp * cy + sr * sp * sy;  // w

    return result;
}

template <typename T, size_t N>
void euler_to_quaternion(const T (&result)[N], const T roll, const T pitch, const T yaw) {
    if (N < 4) {
        return;
    }
    
    T h_yaw = yaw * static_cast<T>(0.5);
    T h_pitch = pitch * static_cast<T>(0.5);
    T h_roll = roll * static_cast<T>(0.5);

    T cy = cos(h_yaw);
    T sy = sin(h_yaw);
    T cp = cos(h_pitch);
    T sp = sin(h_pitch);
    T cr = cos(h_roll);
    T sr = sin(h_roll);

    result[0] = sr * cp * cy - cr * sp * sy;  // x
    result[1] = cr * sp * cy + sr * cp * sy;  // y
    result[2] = cr * cp * sy - sr * sp * cy;  // z
    result[3] = cr * cp * cy + sr * sp * sy;  // w
}