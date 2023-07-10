#include "Spirv.hpp"
#include <iostream>

namespace Sol {
namespace Spirv {

VkShaderStageFlagBits parse(size_t code_size, const uint32_t *p_code) {

    const uint32_t magic = 0x07230203;
    if (p_code[0] != magic)
        return VK_SHADER_STAGE_ALL;

  uint16_t size = 5;
  while(size < code_size / 4)  {
    uint16_t *info = (uint16_t*)(p_code + size);
    
    switch (info[0]) {
    case 15:
    uint32_t *model = (uint32_t*)(p_code + size + 1);
    switch (*model) {
      case 0:
        return VK_SHADER_STAGE_VERTEX_BIT;
      case 4:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
      default:
        break;
    }
    }

    size += info[1];
  } 

  return VK_SHADER_STAGE_ALL;
}

} // namespace Spirv
} // namespace Sol
