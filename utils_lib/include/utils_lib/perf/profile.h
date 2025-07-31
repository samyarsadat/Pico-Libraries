/*
    Pico Libraries - Macros for profiling execution times.
    Copyright 2025 Samyar Sadat Akhavi
    Written by Samyar Sadat Akhavi, 2025.
 
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
#include "pico_log_lib/logger.h"


#if defined(NDEBUG) || !defined(PERF_PROFILE_EN)
#define PROFILE_VAR_DECLARE(name)             (void) 0
#define PROFILE_SECT_BEGIN_NO_DCLR(name)      (void) 0
#define PROFILE_SECT_BEGIN(name)              (void) 0
#define PROFILE_SECT_END(name)                (void) 0
#define PROFILE_RESULT_LOG(name, time_div)    (void) 0
#define PROFILE_SECT_END_LOG(name, time_div)  (void) 0
#else
#define PROFILE_VARS_DECLARE(name)      \
    uint32_t _profile_start_##name = 0; \
    uint32_t _profile_result_##name = 0

#define PROFILE_SECT_BEGIN_NO_DCLR(name) \
    _profile_start_##name = time_us_32()

#define PROFILE_SECT_END_NO_DCLR(name) \
    _profile_result_##name = (time_us_32() - _profile_start_##name)

#define PROFILE_SECT_BEGIN(name) \
    const uint32_t _profile_start_##name = time_us_32()

#define PROFILE_SECT_END(name) \
    const uint32_t _profile_result_##name = (time_us_32() - _profile_start_##name)

#define PROFILE_RESULT_LOG(name, time_div) \
    LOG(LOG_LVL_DEBUG, "Time for "#name": %f", _profile_result_##name / time_div)

#define PROFILE_SECT_END_LOG(name, time_div) \
    LOG(LOG_LVL_DEBUG, "Time for "#name": %f", (time_us_32() - _profile_start_##name) / time_div)
#endif