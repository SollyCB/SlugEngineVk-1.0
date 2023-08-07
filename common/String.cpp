#include "String.hpp"
#include "Allocator.hpp"
#include "VulkanErrors.hpp"
#include "Assert.hpp"

namespace Sol {

// StringView ///////////////////
void StringView::copy_to_buf(StringBuffer *buf, size_t start, size_t end)
{
    char *ptr = buf->str;
    ASSERT(start < end, "StringView::copy_to_buf: start > end");
    mem_cpy(buf->str, ptr + start, end - start);
}

// StringBuffer /////////////////
StringBuffer StringBuffer::nil() {
    StringBuffer buf;
    return buf;
}
StringBuffer StringBuffer::get(const char *cstr, size_t size)
{
    StringBuffer buf;
    buf.init(size);
    buf.copy_here(cstr, size);
    return buf;
}
StringBuffer StringBuffer::get(std::string std_str, size_t size)
{
    StringBuffer buf;
    buf.init(size);
    buf.copy_here(std_str, size);
    return buf;
}
StringBuffer StringBuffer::get(const char *cstr, size_t size, Allocator *alloc_)
{
    StringBuffer buf;
    buf.init(size, alloc_);
    buf.copy_here(cstr, size);
    return buf;
}
StringBuffer StringBuffer::get(std::string std_str, size_t size, Allocator *alloc_)
{
    StringBuffer buf;
    buf.init(size, alloc_);
    buf.copy_here(std_str, size);
    return buf;
}

size_t StringBuffer::cstr_len(const char* cstr) {
    size_t size = 0;
    while (cstr[size] != '\0')
        ++size;
    return size;
}

void StringBuffer::init(size_t size)
{
    ASSERT(size > 0, "StringBuffer::init: size must be greater than 0");
    str = reinterpret_cast<char*>(mem_alloca(size + 1, 1, alloc));
    cap = size;

    str[len] = '\0';
}
void StringBuffer::init(size_t size, Allocator *alloc_)
{
    cap = size;
    alloc = alloc_;

    str = reinterpret_cast<char*>(mem_alloca(size + 1, 1, alloc));
    str[len] = '\0';
}
void StringBuffer::kill()
{
    if (alloc == &MemoryService::instance()->system_allocator)
        mem_free(str, alloc);
}

void StringBuffer::grow(size_t size)
{
    // +1 for null byte is not in the cap
    str = (char *)mem_realloc(size + cap + 1, len + 1, str, 1, alloc);
    cap += size;
    // Null byte should already be copied over
    // str[len] = '\0'; 
}
void StringBuffer::copy_here(const char *str_, size_t size)
{
    ASSERT(size <=cap,  "StringBuffer::copy_here: Allocate more memory!");

    mem_cpy(str, str_, size);
    len = size;
    str[len] = '\0';
}

void StringBuffer::copy_here(std::string std_str, size_t size)
{
    ASSERT(size <= cap, "StringBuffer::copy_here: Allocate more memory!");

    mem_cpy((void *)str, (void *)std_str.c_str(), size);
    len = size;
    str[len] = '\0';
}

void StringBuffer::push(const char *cstr)
{
    size_t size = cstr_len(cstr);

    ASSERT(size <= cap - len, "StringBuffer::push: Allocate more memory (size > cap - len)!")

    mem_cpy(str + len, cstr, size);
    len += size;
    str[len] = '\0';
}
void StringBuffer::push(std::string std_str)
{
    size_t size = std_str.length();

    ASSERT(size <= cap - len, "StringBuffer::push: Allocate more memory (size > cap - len)!")

    mem_cpy(str + len, std_str.c_str(), size);
    len += size;
    str[len] = '\0';
}

const char *StringBuffer::cstr()
{
#if 0 
    if (len == 0)
        // In case StringBuffer is for some reason uninitialized
        return "\0";
    else
#endif
    return (const char *)str;
}
StringView StringBuffer::view(size_t start, size_t end)
{
    StringView view;
    view.buf = this;
    view.start = start;
    view.end = end;
    return view;
}

} // namespace Sol
