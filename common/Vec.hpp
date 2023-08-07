#pragma once

// clang-format off

#include <alloca.h>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>

#include "Allocator.hpp"
#include "Assert.hpp"

namespace Sol {

template <typename T>
struct PopResult {
	bool some = false;
	T item;
};

template <typename T>
struct Vec {
	size_t len = 0;
	size_t cap = 0;
	T* data = nullptr;
	Allocator* allocator = nullptr;

	/* General API */
	// Never uses a linear allocator as is has to be able to grow...
	// If a known size is required, use the Array<T> type
	void init(size_t cap_) {
		cap = cap_;
		allocator = HEAP;
		if (cap > 0)
			data = reinterpret_cast<T*>(heap_alloc(cap * sizeof(T)));
	}

	void kill() {
		mem_free(data, allocator);
		len = 0;
		cap = 0;
	}
	void print() {
		for(size_t i = 0; i < len; ++i) {
		  std::cout << data[i] << '\n';
		}
	}
    void reserve(size_t count) {
        len += count;
        ASSERT(len <= cap, "Vec::alloc len + count > capacity");
    }
    void zero() {
        memset(data + len, 0, cap - len);
    }
	void grow_zero(size_t count) {
		grow(count);
		zero();
	}
	PopResult<T> pop() {
		if (!len) {
			PopResult<T> none;
			none.some = false;
			return none;
		}
		--len;
		return PopResult<T> { true, data[len] };
	}
    void push(T &t) {
        if (cap == len)
          grow();
        data[len] = std::move(t);
        ++len;
    }
	void push_cpy(T t) {
		if (cap == len)
		  grow();
		data[len] = t;
		++len;
	}
	void resize(size_t size) {
		data = reinterpret_cast<T*>(mem_realloc(size * sizeof(T), len, data, 8, allocator));
	}
	void grow(size_t size) {
		cap += size;
		T* new_data = (T*)mem_alloca(cap * sizeof(T), 8, allocator);
		mem_cpy(new_data, data, len * sizeof(T));
		mem_free(data, allocator);
		data = new_data;
	}
	void grow() {
		cap *= 2;
		T* new_data = (T*)mem_alloca(cap * sizeof(T), 8, allocator);
		mem_cpy(new_data, data, len * sizeof(T));
		mem_free(data, allocator);
		data = new_data;
	}
	T* last() {
		return &data[len - 1];
	}
	T& operator[](size_t i) {
		if (i >= len) {
		  std::cerr << "OUT OF BOUNDS ACCESS ON VEC " << data << "\n";
		  exit(-1);
		}
		return data[i];
	}
};

} // namespace Sol
