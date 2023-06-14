#pragma once
#include <vulkan/vulkan.hpp>
#include <iostream>

namespace Sol {

struct VulkanError {
  static const char* match_error(VkResult error);
};

#define ABORT(test, msg) \
  if (!(test)) { std::cerr << "ABORTED: " << msg << '\n'; abort(); }

#if V_LAYERS

#define DEBUG_OBJ_CREATION(creation_func, err_code)  \
  if (err_code != VK_SUCCESS) { \
    const char* err_msg = (VulkanError::match_error(err_code)); \
    std::cerr << "OBJ CREATION ERROR: " << #creation_func << \
    " returned " << err_msg << ", (" << __FILE__ << ", " << __LINE__ << ")\n";  \
    abort(); \
  } 

#define DEBUG_ABORT(test, msg) \
  if (!(test)) { std::cout << "DEBUG_ABORT: " << msg << '\n'; abort(); }

#else 

#define DEBUG_ABORT(test, msg) {}

#define DEBUG_OBJ_CREATION(creation_func, err_code)  {}

#endif



} // Sol
