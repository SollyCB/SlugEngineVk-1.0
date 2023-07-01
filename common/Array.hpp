#pragma once 
#include "Allocator.hpp"
#include "VulkanErrors.hpp"

namespace Sol {

template <typename T>
struct Array {
  T* mem = nullptr;
  size_t cap = 0;
  size_t len = 0;
  Allocator *alloc = &MemoryService::instance()->scratch_allocator;
  
void init(size_t size, size_t alignment) {
  cap = size;
  mem = (T*)mem_alloc2(size * sizeof(T), alignment, alloc);
}
void reset() {
  len = 0;
}

void push(T t) {
  ABORT(len < cap, "Push to Array<T> with insufficient capacity");
  mem[len] = t;
  ++len;
}
T pop() {
  if (len == 0)
    return NULL;
  --len;
  return mem[len];
}
void swap_last(size_t i) {
  ABORT(i < len, "Out of bounds access on Array<T>");
  T tmp = mem[i];
  mem[i] = mem[len - 1];
  mem[len - 1] = tmp;
}
void copy_here(T* data, size_t count) {
  ABORT(cap - len >= count, "Array<T>::copy_here with insufficient size");
  mem_cpy(mem + len, data, count * sizeof(T));
  len += count;
}

T& operator[](size_t i) {
  ABORT(i < len, "Out of Bounds access on Array<T>");
  return mem[i];
}
};

} // namespace Sol 
