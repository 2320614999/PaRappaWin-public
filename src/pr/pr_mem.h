#pragma once

#include <cstddef>

struct PrHeapInfo {
    void* start = nullptr;
    size_t size = 0;
};

struct PrMem {
    static void Init(void* start, size_t size);
    static void* Alloc(size_t size, size_t align = 4);
    static void Free(void* ptr);
    static PrHeapInfo Info();
};
