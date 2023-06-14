#include <cstdio>
#include <iostream>

#include "File.hpp"
#include "Allocator.hpp"

namespace Sol {

void* File::read_spirv(size_t *byte_count, const char* file_name) {
  Allocator *alloc = &Sol::MemoryService::instance()->system_allocator;
  FILE *file = fopen(file_name, "r");

  if (!file) {
    std::cerr << "FAILED TO READ FILE " << file_name << "!\n";
    fclose(file);
    return nullptr;
  }

  fseek(file, 0, SEEK_END);
  *byte_count = ftell(file);
  fseek(file, 0, SEEK_SET);

  void* buffer = mem_alloca(*byte_count, sizeof(uint32_t));
  fread(buffer, *byte_count, 1, file);
  fclose(file);

  return buffer;
}

void* File::read_bin(size_t *byte_count, const char* file_name) {
  Allocator *alloc = &Sol::MemoryService::instance()->system_allocator;
  FILE *file = fopen(file_name, "r");

  if (!file) {
    std::cerr << "FAILED TO READ FILE " << file_name << "!\n";
    fclose(file);
    return nullptr;
  }

  fseek(file, 0, SEEK_END);
  *byte_count = ftell(file);
  fseek(file, 0, SEEK_SET);

  void* buffer = mem_alloc(*byte_count);
  fread(buffer, *byte_count, 1, file);
  fclose(file);

  return buffer;
}

}
