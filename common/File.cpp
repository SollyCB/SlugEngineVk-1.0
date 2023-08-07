#include <cstdio>
#include <iostream>

#include "Allocator.hpp"
#include "File.hpp"
#include "Assert.hpp"

namespace Sol {

const uint32_t *File::read_spv(size_t *byte_count, const char *file_name, Allocator *alloc) {
    FILE *file = fopen(file_name, "r");

    if (!file) {
        std::cerr << "FAILED TO READ FILE " << file_name << "!\n";
        return nullptr;
    }

    fseek(file, 0, SEEK_END);
    *byte_count = ftell(file);
    fseek(file, 0, SEEK_SET);

    void *spv = mem_alloca(*byte_count, 4, alloc);
    size_t read = fread(spv, 1, *byte_count, file);
    DEBUG_ASSERT(read == *byte_count, "Failed to read entire file");
    fclose(file);

    return reinterpret_cast<const uint32_t*>(spv);
}

void *File::read_char(size_t *byte_count, const char *file_name, Allocator *alloc) {
    FILE *file = fopen(file_name, "r");

    if (!file) {
        std::cerr << "FAILED TO READ FILE " << file_name << "!\n";
        fclose(file);
        return nullptr;
    }

    fseek(file, 0, SEEK_END);
    *byte_count = ftell(file);
    fseek(file, 0, SEEK_SET);

    void *buffer = mem_alloca(*byte_count, 1, alloc);
    size_t read = fread(buffer, 1, *byte_count, file);
    DEBUG_ASSERT(read == *byte_count, "Failed to read entire file");
    fclose(file);

    return buffer;
}

} // namespace Sol
