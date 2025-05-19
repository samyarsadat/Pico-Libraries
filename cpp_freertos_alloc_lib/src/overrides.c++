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
    along with this program.  If not, see <https: www.gnu.org/licenses/>.
*/

#include <cstddef>
#include <new>
#include "FreeRTOS.h"


void* operator new(size_t size) {
    return pvPortMalloc(size);
}

void* operator new[](size_t size) {
    return pvPortMalloc(size);
}

void operator delete(void* ptr) noexcept {
    vPortFree(ptr);
}

void operator delete[](void* ptr) noexcept {
    vPortFree(ptr);
}

void operator delete(void* ptr, size_t) noexcept {
    vPortFree(ptr);
}

void operator delete[](void* ptr, size_t) noexcept {
    vPortFree(ptr);
}

void* operator new(size_t size, const std::nothrow_t&) {
    return pvPortMalloc(size);
}

void* operator new[](size_t size, const std::nothrow_t&) {
    return pvPortMalloc(size);
}

void* operator new(size_t size, std::align_val_t alignment) {
    size_t extra = static_cast<size_t>(alignment) - 1 + sizeof(void*);
    char* raw = static_cast<char*>(pvPortMalloc(size + extra));
    
    if (!raw) {
        assert(false);
        return nullptr;
    }
    
    void* aligned = reinterpret_cast<void*>(
        (reinterpret_cast<uintptr_t>(raw) + sizeof(void*) + 
         static_cast<size_t>(alignment) - 1) & 
        ~(static_cast<size_t>(alignment) - 1)
    );
    
    *(reinterpret_cast<void**>(aligned) - 1) = raw;
    return aligned;
}

void* operator new[](size_t size, std::align_val_t alignment) {
    return operator new(size, alignment);
}

void* operator new(size_t size, std::align_val_t alignment, const std::nothrow_t&) {
    return operator new(size, alignment);
}

void* operator new[](size_t size, std::align_val_t alignment, const std::nothrow_t&) {
    return operator new(size, alignment);
}

void operator delete(void* ptr, std::align_val_t) noexcept {
    if (ptr) {
        void* original = *(reinterpret_cast<void**>(ptr) - 1);
        vPortFree(original);
    }
}

void operator delete[](void* ptr, std::align_val_t alignment) noexcept {
    operator delete(ptr, alignment);
}

void operator delete(void* ptr, size_t, std::align_val_t alignment) noexcept {
    operator delete(ptr, alignment);
}

void operator delete[](void* ptr, size_t, std::align_val_t alignment) noexcept {
    operator delete(ptr, alignment);
}

void* operator new(size_t, void* ptr) {
    return ptr;
}

void* operator new[](size_t, void* ptr) {
    return ptr;
}