#include "String.hpp"
#include "Allocator.hpp"
#include "VulkanErrors.hpp"

namespace Sol {

// StringView ///////////////////
void StringView::copy_to_buf(StringBuffer *buf, size_t start, size_t end) {
  char *ptr = buf->str;
  ABORT(start < end, "StringView::copy_to_buf: start > end");
  mem_cpy(buf->str, ptr + start, end - start);
}

// StringBuffer /////////////////
StringBuffer StringBuffer::get(size_t size, const char* str_) {
  StringBuffer buf;
  if (size > 0) {
    buf.init(size);
    buf.copy_here(str_, size);
  } else {
    buf.copy_here(str_, 0);
  }
  return buf;
}
StringBuffer StringBuffer::get(size_t size, std::string str_) {
  StringBuffer buf;
  if (size > 0) {
    buf.init(size);
    buf.copy_here(str_.c_str(), size);
  } else {
    buf.copy_here(str_.c_str(), 0);
  }
  return buf;
}
StringBuffer StringBuffer::get(size_t size, const char* str_, Allocator *alloc_) {
  StringBuffer buf;
  if (size > 0) {
    buf.init(size, alloc_);
    buf.copy_here(str_, size);
  } else {
    buf.copy_here(str_, 0);
  }
  return buf;
}
StringBuffer StringBuffer::get(size_t size, std::string str_, Allocator *alloc_) {
  StringBuffer buf;
  if (size > 0) {
    buf.init(size, alloc_);
    buf.copy_here(str_.c_str(), size);
  } else {
    buf.copy_here(str_.c_str(), 0);
  }
  return buf;
}

void StringBuffer::init(size_t size) {
  ABORT(size > 0, "StringBuffer::init: size must be greater than 0");
  cap = size; 

  if (alloc == &MemoryService::instance()->system_allocator)
    str = (char*)mem_alloc(size + 1);
  else 
    str = (char*)lin_alloca(size + 1, 1);
  str[0] = '\0';
}
void StringBuffer::init(size_t size, Allocator *alloc_) {
  cap = size;
  alloc = alloc_;

  if (alloc == &MemoryService::instance()->system_allocator)
    str = (char*)mem_alloc(size + 1);
  else 
    str = (char*)lin_alloca(size + 1, 1);
}
void StringBuffer::kill() {
  if (alloc == &MemoryService::instance()->system_allocator)
    mem_free(str);
}

void StringBuffer::grow(size_t size) {
  // +1 for null byte is not in the cap
  if (alloc == &MemoryService::instance()->system_allocator) {
    str = (char*)mem_realloc(size + cap + 1, str);
  } else {
    char* old_str = str;
    str = (char*)lin_alloca(size + cap + 1, 1);
    mem_cpy(str, old_str, len + 1); // len + 1 for null byte
  }
  cap += size;
  str[len] = '\0'; // Just for safety sake, in case for whatever reason it wasnt there for the copy...
}
void StringBuffer::copy_here(const char *str_, size_t size) {
  if (size == 0) {
    while(str_[size] != '\0')
      ++size;
  }

  size_t rem = cap - len;
  if (rem < size) 
    grow(size - rem);

  mem_cpy(str, str_, size);
  len = size;
  str[len] = '\0';
}

void StringBuffer::copy_here(std::string str_, size_t size) {
  size = str_.length();
  if (size == 0)
    return;
  //ABORT(size > 0, "StringBuffer::copy_here size to copy must be > 0");

  if (cap < size) 
    grow(size - cap);

  mem_cpy((void*)str, (void*)str_.c_str(), size);
  len = size;
  str[len] = '\0';
}

void StringBuffer::push(const char *str_) {
  size_t size = 0;
  while(str_[size] != '\0')
    ++size;

  size_t rem = cap - len;
  if (rem < size)
    grow(size - rem);

  mem_cpy(str + len, str_, size);
  len += size;
  str[len] = '\0';
}
void StringBuffer::push(std::string str_) {
  size_t size = str_.length();

  size_t rem = cap - len;
  if (rem < size)
    grow(size - rem);

  mem_cpy(str + len, str_.c_str(), size);
  len += size;
  str[len] = '\0';
}

const char* StringBuffer::c_str() {
  if (len == 0)
    return "\0";
  else 
    return (const char*)str;
}
StringView StringBuffer::view(size_t start, size_t end) {
  StringView view;
  view.buf = this;
  view.start = start;
  view.end = end;
  return view;
}

} // namespace Sol
