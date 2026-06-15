#include "pr_mem.h"

#include <cstdint>

namespace {

static uint8_t* g_start = nullptr;
static size_t g_size = 0;
static size_t g_offset = 0;

static size_t AlignUp(size_t v, size_t a) {
    if (a == 0) return v;
    const size_t m = a - 1;
    return (v + m) & ~m;
}

} // namespace

void PrMem::Init(void* start, size_t size) {
    g_start = reinterpret_cast<uint8_t*>(start);
    g_size = size;
    g_offset = 0;
}

void* PrMem::Alloc(size_t size, size_t align) {
    if (!g_start || g_size == 0) {
        return nullptr;
    }

    if (align < 1) align = 1;
    if ((align & (align - 1)) != 0) {
        align = 4;
    }

    const size_t off = AlignUp(g_offset, align);
    if (off > g_size) {
        return nullptr;
    }
    if (size > g_size - off) {
        return nullptr;
    }

    void* p = g_start + off;
    g_offset = off + size;
    return p;
}

void PrMem::Free(void* ptr) {
    (void)ptr;
}

PrHeapInfo PrMem::Info() {
    PrHeapInfo info;
    info.start = g_start;
    info.size = g_size;
    return info;
}
