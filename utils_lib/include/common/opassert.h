/*
    Pico Libraries - Custom assert macro
    These are common math-related definitions.
    
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


#ifdef NDEBUG
#define opassert(expr) ((void) expr)
#else
#define opassert(expr) ((expr) ? (void) 0 : __assert_func(__FILE__, __LINE__, __ASSERT_FUNC, #expr))
#endif

#ifdef NDEBUG
#define opequal(l_expr, r_expr) ((void) l_expr)
#else
#define opequal(l_expr, r_expr) ((l_expr == r_expr) ? (void) 0 : __assert_func(__FILE__, __LINE__, __ASSERT_FUNC, #l_expr))
#endif