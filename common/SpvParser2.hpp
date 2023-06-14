#pragma once

namespace Sol {

struct SpvParser {
  void parse(uint32_t code_size, void* spirv);
};

} // Sol
