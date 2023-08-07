// clang-format off
#include <iostream>
#include <cstring>

#include "tlsf.h"
#include "Allocator.hpp"
#include "Assert.hpp"
//#include "VulkanErrors.hpp"

//#define NDEBUG
#include <assert.h>

namespace Sol {

size_t mem_align(size_t size, size_t alignment) {
  const size_t alignment_mask = alignment - 1;
  return (size + alignment_mask) & ~alignment_mask;
}

// MemoryService //////////////////////
static MemoryService GlobalMemoryService;
MemoryService *MemoryService::instance() { return &GlobalMemoryService; }
void MemoryService::init(MemoryConfig* config) {
  std::cout << "Initializing memory service, allocating " << config->size_heap << " bytes to system allocator...\n";
  system_allocator.init(config->size_heap);
  std::cout << "Initializing memory service, allocating " << config->size_linear << " bytes to scratch allocator...\n";
  scratch_allocator.init(config->size_linear);
}
void MemoryService::shutdown() {
  scratch_allocator.kill();
  system_allocator.shutdown();
}

// HeapAllocator /////////////////////
HeapAllocator::~HeapAllocator() { }

void HeapAllocator::init(size_t size) {
  memory = malloc(size);
  limit = size;
  handle = tlsf_create_with_pool(memory, size);
  std::cout << "HeapAllocator, size " << size << " created...\n";
} // init

void HeapAllocator::shutdown() {
  MemoryStatsHeap stats = { 0, limit };
  pool_t pool = tlsf_get_pool(handle);
  tlsf_walk_pool(pool, nullptr, (void*)&stats);
  if (stats.allocated_bytes)
    std::cerr << "FAILED TO SHUTDOWN HEAPALLOCATOR! DETECTED ALLOCATED MEMORY!\n"
      << "  Allocated: " << stats.allocated_bytes << '\n'
      << "  Total: " << stats.total_bytes << '\n';
  else
    std::cout << "HeapAllocator successfully shutdown! All memory free!\n";

  assert(stats.allocated_bytes == 0 && "MEMORY IS STILL ALLOCATED\n");
  tlsf_destroy(handle);
  free(memory);
} // shutdown

  /* General API */
void *HeapAllocator::allocate(size_t size, size_t alignment) {
  void *allocated_mem = alignment == 1 ? tlsf_malloc(handle, size) : tlsf_memalign(handle, alignment, size);
  size_t actual_size = tlsf_block_size(allocated_mem);
  allocated += actual_size;
#if defined MEM_STATS
  active_allocations->push_back(allocated_mem);
#endif
  return allocated_mem;
}
void *HeapAllocator::reallocate(size_t size, size_t cpy_size, void* ptr, size_t alignment) {
    // Unused variable complaint
    cpy_size = 0; // NULL
    alignment = 0; // NULL

    void *allocated_mem = tlsf_realloc(handle, ptr, size);
    size_t actual_size = tlsf_block_size(allocated_mem);
    allocated += actual_size;
    return allocated_mem;
}
void HeapAllocator::deallocate(void* ptr) {
  size_t actual_size = tlsf_block_size(ptr);
  allocated -= actual_size;
  tlsf_free(handle, ptr);

#if defined MEM_STATS
  // TODO: SIMD?
  for(size_t i = 0; i < active_allocations->size(); ++i) {
    if ((*active_allocations)[i] == ptr) {
      size_t last_index = active_allocations->size() - 1;
      void* item = (*active_allocations)[last_index];
      (*active_allocations)[i] = item;
      active_allocations->pop_back();
    }
  }
#endif
}

// LinearAllocator ////////////////////
LinearAllocator::~LinearAllocator() { }

void *LinearAllocator::reallocate(size_t size, size_t cpy_size, void* ptr, size_t alignment) {
    void* new_ptr = allocate(size, alignment);
    mem_cpy(new_ptr, ptr, cpy_size);
    return new_ptr;
}
void LinearAllocator::deallocate(void* ptr) {
    ptr = nullptr;
    alloced = 0;
}

void LinearAllocator::init(size_t size) {
  mem = (uint8_t*)malloc(size);
  cap = size;
}
void *LinearAllocator::allocate(size_t size, size_t alignment) {
  size_t pad = mem_align(size, alignment) - size;
  void* ptr = (void*)(mem + alloced + pad);
  alloced += pad + size;
  ASSERT(alloced < cap, "Linear Allocator: Overflow");
  return ptr;
}
void LinearAllocator::cut(size_t size) {
  alloced -= size;
}
void LinearAllocator::free() {
  alloced = 0;
}
void LinearAllocator::kill() {
  std::cout << "    \nLinear allocator freed:\n";
  std::cout << "        Remaining Alloced size in LinearAllocator: " << alloced << '\n';
  cap = 0;
  DEBUG_ASSERT(mem, "Linear Allocator: free nullptr");
  ::free((void*)mem);
}

} // namespace Sol
