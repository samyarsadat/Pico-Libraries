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
float map(float input, int in_min, int in_max, int out_min, int out_max) {
    return (float) (((input - in_min) * (out_max - out_min)) / (in_max - in_min)) + out_min;
}

// Adjustable truncate function
float truncate_adj(float input, int trunc_amount) {
    return round(input * pow(10, trunc_amount)) / pow(10, trunc_amount);
}

// Calculates the mean (average) of the numbers in a float vector
float calculate_mean(vector<float> &numbers) {
    float total;
    for (auto &num : numbers) {
        total += num;
    }

    return total / numbers.size();
}

// Calculates the standard deviation of the numbers in a float vector
float calculate_standard_deviation(vector<float> &numbers, float numbers_mean) {
    float deviation_total;
    for (auto &num : numbers) {
        deviation_total += pow(num - numbers_mean, 2);
    }

    return sqrt(deviation_total / numbers.size());
}

// Finds "outliers" in-between the numbers in a float vector using the Z-Score (Standard Score) method
// It returns a boolean vector (with the same size as the input vector) that indicates the "outliers" by returning their slots as true
vector<bool> standard_score_check(vector<float> &numbers, float z_score_threshhold) {
    float mean = calculate_mean(numbers);
    float standard_deviation = calculate_standard_deviation(numbers, mean);
    vector<bool> outliers;
    
    for (auto &num : numbers) {
        if (abs((num - mean) / standard_deviation) > z_score_threshhold) {
            outliers.push_back(true);
        } else {
            outliers.push_back(false);
        }
    }

    return outliers;
}

// Converts Euler angles to a quaternion
// Output: [x, y, z, w]
vector<float> euler_to_quaternion(float roll, float pitch, float yaw) {
    vector<float> quaternion;
    
    float yaw_half = yaw * 0.5;
    float pitch_half = pitch * 0.5;
    float roll_half = roll * 0.5;

    float cy = cos(yaw_half);
    float sy = sin(yaw_half);
    float cp = cos(pitch_half);
    float sp = sin(pitch_half);
    float cr = cos(roll_half);
    float sr = sin(roll_half);

    quaternion.push_back(sr * cp * cy - cr * sp * sy);  // x
    quaternion.push_back(cr * sp * cy + sr * cp * sy);  // y
    quaternion.push_back(cr * cp * sy - sr * sp * cy);  // z
    quaternion.push_back(cr * cp * cy + sr * sp * sy);  // w

    return quaternion;
}
