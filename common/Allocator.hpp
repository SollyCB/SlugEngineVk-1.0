#pragma once
// clang-format off

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vector>
#include <iostream>

#include "tlsf.h"

#define MEM_STATS
namespace Sol {

inline size_t memory_align(size_t size, size_t alignment) {
  const size_t alignment_mask = alignment - 1;
  return (size + alignment_mask) & ~alignment_mask;
}

struct MemoryStatsHeap {
  size_t allocated_bytes;
  size_t total_bytes;
  uint32_t allocation_count = 0;
  std::vector<void*> active_allocations;
  void add(std::size_t a) {
    if (a) {
      allocated_bytes += a;
      ++allocation_count;
    }
  }
};
static std::vector<void*> Global_Allocation_Tracker;

struct Allocator {
  virtual ~Allocator() {}
  virtual void *allocate(size_t size, size_t alignment) = 0;
  virtual void *reallocate(size_t size, size_t cpy_size, void* ptr, size_t alignment) = 0;
  virtual void deallocate(void* ptr) = 0;
};

struct HeapAllocator : public Allocator {
  ~HeapAllocator() override;

  void *handle;
  void *memory;
  size_t allocated = 0;
  size_t limit = 0;
#if defined MEM_STATS
  std::vector<void*>* active_allocations = &Global_Allocation_Tracker;
#endif

  /* Initialize/Kill service */
  void init(size_t size);
  void shutdown();

  /* General API */
  void *allocate(size_t size, size_t alignment) override;
  void *reallocate(size_t size, size_t cpy_size, void* ptr, size_t alignment) override;
  void deallocate(void* ptr) override;
};

struct LinearAllocator : public Allocator {
  ~LinearAllocator() override;

  uint8_t *mem = nullptr;
  size_t cap = 0;
  size_t alloced = 0;

  void init(size_t size);
  void cut(size_t size);
  void free();
  void kill();

  inline size_t get_mark() {
      return alloced;
  }
  inline void cut_diff(size_t mark) {
      if (alloced > mark)
          cut(alloced - mark);
  }

  /* General API */
  void *allocate(size_t size, size_t alignment) override;
  void *reallocate(size_t size, size_t cpy_size, void* ptr, size_t alignment) override;
  void deallocate(void* ptr) override;
};

struct MemoryConfig {
  size_t size_heap = 32 * 1024 * 1024;
  size_t size_linear = 32 * 1024;
};

struct MemoryService {
  HeapAllocator system_allocator;
  LinearAllocator scratch_allocator;
  // return a pointer to an instance of a static MemoryService
  static MemoryService* instance();
  void init(MemoryConfig* config);
  // free all memory associated with the service
  void shutdown();
};

static HeapAllocator *HEAP = &MemoryService::instance()->system_allocator;
static LinearAllocator *SCRATCH = &MemoryService::instance()->scratch_allocator;

#define lin_alloca(size, alignment) ((Sol::MemoryService::instance()->scratch_allocator).allocate(size, alignment))
#define heap_alloca(size, alignment) ((Sol::MemoryService::instance()->system_allocator).allocate(size, alignment))
#define lin_alloc(size) ((Sol::MemoryService::instance()->scratch_allocator).allocate(size, 8))
#define heap_alloc(size) ((Sol::MemoryService::instance()->system_allocator).allocate(size, 8))
#define lin_free(ptr) ((Sol::MemoryService::instance()->scratch_allocator).deallocate(ptr))
#define heap_free(ptr) ((Sol::MemoryService::instance()->system_allocator).deallocate(ptr))

#define mem_alloca(size, alignment, alloc) ((alloc)->allocate(size, alignment))
#define mem_realloc(size, cpy_size, ptr, alignment, alloc) ((alloc)->reallocate(size, cpy_size, (void*)(ptr), alignment))
#define mem_free(ptr, alloc) ((alloc)->deallocate((void*)(ptr)))
#define mem_cpy(to, from, size) (memcpy(to, from, size))

} // Sol
