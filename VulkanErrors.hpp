#pragma once
#include <vulkan/vulkan.h>
#include <iostream>

namespace Sol {

struct VulkanError {
  static const char* match_error(VkResult error);
};

#if V_LAYERS

#define DEBUG_OBJ_CREATION(creation_func, err_code)  \
  if (err_code != VK_SUCCESS) { \
    const char* err_msg = (VulkanError::match_error(err_code)); \
    std::cerr << "OBJ CREATION ERROR: " << #creation_func << \
    " returned " << err_msg << ", (" << __FILE__ << ", " << __LINE__ << ")\n";  \
    __builtin_trap(); \
  } 


#else 

#define DEBUG_ABORT_CREATION(obj_name, creation_func, err_code)  

#endif

} // Sol
