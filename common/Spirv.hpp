#pragma once 

#include <cstdint>
#include <cstddef>

#include <vulkan/vulkan.hpp>

namespace Sol {

namespace Spirv {

VkShaderStageFlagBits parse(size_t code_size, const uint32_t* p_code); 

} // namespace Spirv

} // Sol
