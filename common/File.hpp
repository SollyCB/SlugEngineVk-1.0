#pragma once

#include <cstdint>

#include "Allocator.hpp"

namespace Sol {

struct File {
    static void *read_char(size_t *byte_count, const char *file_name, Allocator *alloc);
    static void *read_spirv(size_t *byte_count, const char *file_name, Allocator *alloc);
};

} // namespace Sol
