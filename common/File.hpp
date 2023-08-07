#pragma once

#include <cstdint>

#include "Allocator.hpp"

namespace Sol {

namespace File {
    const uint32_t *read_spv(size_t *byte_count, const char *file_name, Allocator *alloc);
    void *read_char(size_t *byte_count, const char *file_name, Allocator *alloc);
}

} // namespace Sol
