#pragma once

#include <cstdint>

#include "Allocator.hpp"

namespace Sol {

struct File {
    static void *read_bin(size_t *byte_count, const char *file_name);
    static void *read_spirv(size_t *byte_count, const char *file_name);
};

} // namespace Sol
