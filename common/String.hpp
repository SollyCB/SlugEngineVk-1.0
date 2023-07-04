#pragma once
#include "Allocator.hpp"
#include <string>

namespace Sol {

struct StringBuffer;

struct StringView {
    size_t start = 0;
    size_t end = 0;
    StringBuffer *buf;

    const char *c_str();
    void copy_to_buf(StringBuffer *buf, size_t start, size_t end);
};

struct StringBuffer {
    // cap is one less than the true capacity: there is always a byte for null
    // term
    size_t cap = 0;
    size_t len = 0;
    char *str = nullptr;
    Allocator *alloc = &MemoryService::instance()->scratch_allocator;

    /*
     * !! size argument should not include null byte, this is already accounted
     * for !!
     */
    static StringBuffer nil();
    static StringBuffer get(const char *cstr, size_t size);
    static StringBuffer get(std::string std_str, size_t size);
    static StringBuffer get(const char *cstr, size_t size, Allocator *alloc_);
    static StringBuffer get(std::string std_str, size_t size, Allocator *alloc_);
    static size_t cstr_len(const char* cstr);

    void init(size_t size);
    void init(size_t size, Allocator *allocator_);
    void kill();

    void grow(size_t size);
    // Overwrite existing string data
    void copy_here(const char *str_, size_t size);
    void copy_here(std::string str_, size_t size);

    void push(const char *str_);
    void push(std::string str_);
    const char *cstr();
    StringView view(size_t start, size_t end);
};

} // namespace Sol
