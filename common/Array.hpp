#pragma once
#include "Allocator.hpp"
#include "Assert.hpp"
//#include "VulkanErrors.hpp"

namespace Sol {

template <typename T>
struct Array {
  T* mem = nullptr;
  size_t cap = 0;
  size_t len = 0;
  Allocator *alloc = &MemoryService::instance()->scratch_allocator;

static Array<T> get(size_t size, size_t alignment) {
    Array a;
    a.init(size, alignment);
    return a;
}
static Array<T> get(size_t size, size_t alignment, Allocator *alloc_) {
    Array a;
    a.init(size, alignment, alloc_);
    return a;
}
void init(size_t size, size_t alignment, Allocator *alloc_) {
  cap = size;
  alloc = alloc_;
  mem = (T*)mem_alloca(size * sizeof(T), alignment, alloc);
}
void init(size_t size, size_t alignment) {
  cap = size;
  mem = (T*)mem_alloca(size * sizeof(T), alignment, alloc);
}
void reset() {
  len = 0;
}
void kill() {
    mem_free(mem, alloc);
}
void push(T &t) {
  ASSERT(len < cap, "Push to Array<T> with insufficient capacity");
  mem[len] = std::move(t);
  ++len;
}
void push(const T &t) {
  ASSERT(len < cap, "Push to Array<T> with insufficient capacity");
  mem[len] = std::move(t);
  ++len;
}
void push_cpy(T &t) {
  ASSERT(len < cap, "Push to Array<T> with insufficient capacity");
  mem[len] = std::move(t);
  ++len;
}
void push_cpy(const T &t) {
  ASSERT(len < cap, "Push to Array<T> with insufficient capacity");
  mem[len] = std::move(t);
  ++len;
}
T pop() {
  if (len == 0)
    return NULL;
  --len;
  return mem[len];
}
void fill_zero() {
    T t;
    for(size_t i = len; i < cap; ++i)
        push(t);
}
void swap_last(size_t i) {
  ASSERT(i < len, "Out of bounds access on Array<T>");
  T tmp = mem[i];
  mem[i] = mem[len - 1];
  mem[len - 1] = tmp;
}
void copy_here(T* data, size_t count) {
  ASSERT(cap - len >= count, "Array<T>::copy_here with insufficient size");
  mem_cpy(mem + len, data, count * sizeof(T));
  len += count;
}

T& operator[](size_t i) {
  ASSERT(i < len, "Out of Bounds access on Array<T>");
  return mem[i];
}
};

} // namespace Sol
