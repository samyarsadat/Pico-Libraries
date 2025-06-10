/*
    Pico Libraries
    C++ new and delete memory allocator overrides for FreeRTOS.

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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <cstddef>
#include <new>
#include "FreeRTOS.h"


// Basic new operators
void* operator new(size_t size) {
    return pvPortMalloc(size);
}

void* operator new[](size_t size) {
    return pvPortMalloc(size);
}

// Basic delete operators
void operator delete(void* ptr) noexcept {
    if (ptr) {
        vPortFree(ptr);
    }
}

void operator delete[](void* ptr) noexcept {
    if (ptr) {
        vPortFree(ptr);
    }
}

// Sized delete operators (C++ 14)
void operator delete(void* ptr, size_t) noexcept {
    if (ptr) {
        vPortFree(ptr);
    }
}

void operator delete[](void* ptr, size_t) noexcept {
    if (ptr) {
        vPortFree(ptr);
    }
}

// Nothrow new operators
void* operator new(size_t size, const std::nothrow_t&) noexcept {
    return pvPortMalloc(size);
}

void* operator new[](size_t size, const std::nothrow_t&) noexcept {
    return pvPortMalloc(size);
}

// Aligned new operators (C++ 17)
void* operator new(size_t size, std::align_val_t alignment) {
    size_t align = static_cast<size_t>(alignment);
    size_t extra = align - 1 + sizeof(void*);
    char* raw = static_cast<char*>(pvPortMalloc(size + extra));
    
    // Calculate aligned address
    uintptr_t raw_addr = reinterpret_cast<uintptr_t>(raw);
    uintptr_t aligned_addr = (raw_addr + sizeof(void*) + align - 1) & ~(align - 1);
    void* aligned = reinterpret_cast<void*>(aligned_addr);
    
    // Store original pointer just before the aligned address
    *(reinterpret_cast<void**>(aligned) - 1) = raw;
    
    return aligned;
}

void* operator new[](size_t size, std::align_val_t alignment) {
    return operator new(size, alignment);
}

// Nothrow aligned new operators
void* operator new(size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept {
    return operator new(size, alignment);
}

void* operator new[](size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept {
    return operator new(size, alignment);
}

// Aligned delete operators
void operator delete(void* ptr, std::align_val_t) noexcept {
    if (ptr) {
        void* original = *(reinterpret_cast<void**>(ptr) - 1);
        vPortFree(original);
    }
}

void operator delete[](void* ptr, std::align_val_t alignment) noexcept {
    operator delete(ptr, alignment);
}

// Sized aligned delete operators
void operator delete(void* ptr, size_t, std::align_val_t alignment) noexcept {
    operator delete(ptr, alignment);
}

void operator delete[](void* ptr, size_t, std::align_val_t alignment) noexcept {
    operator delete(ptr, alignment);
}

// Placement new operators (standard library compatibility)
void* operator new(size_t, void* ptr) noexcept {
    return ptr;
}

void* operator new[](size_t, void* ptr) noexcept {
    return ptr;
}