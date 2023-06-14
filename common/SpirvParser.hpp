#pragma once

#include <iostream>
#include <spirv_cross/spirv.h>
#include <vulkan/vulkan.hpp>

struct Member {
  uint32_t typeIndex;
  uint32_t offset;
  const char *name;
};

struct Image {
  // lots of these values need to be parsed into a better format (they are just
  // integers which assign to different settings) This struct is not being used
  // yet...
  uint32_t dim;
  uint32_t depth;
  uint32_t arrayed;
  uint32_t MS;
  uint32_t sampled;
  uint32_t format;
};

struct Id {
  SpvOp opCode;
  SpvStorageClass storageClass; // variables
  uint32_t set;
  uint32_t binding;
  // int, float
  uint8_t width;
  uint8_t sign;
  // array, vector, matrix
  uint32_t typeIndex;
  uint32_t length;
  uint32_t quiv; // constants
  // structs
  char *name;
  std::vector<Member> members;

  Image image;
};

struct ParsedSpv {
  SpvExecutionModel model;
  VkShaderStageFlags stage;
  std::vector<Id> ids;
};

bool parseSpv(uint32_t *data, size_t dataSize);
VkShaderStageFlags parseExecutionModel(SpvExecutionModel model);

#define assert_count(size, msg)                                                \
  if (wordCount < size) {                                                      \
    std::cerr << "SPIRV PARSER COUNT ASSERTION: " << msg                       \
              << " (file: " << __FILE__ << ")\n";                              \
    return false;                                                              \
  }
#define assert_stage(msg)                                                      \
  if (parsed.stage == 0) {                                                     \
    std::cerr << "SPIRV PARSER SHADER STAGE ASSERTION (file: " << __FILE__     \
              << ")\n";                                                        \
    return false;                                                              \
  }
#define assert_idBound(msg)                                                    \
  if (idIndex > idBounds) {                                                    \
    std::cerr << "ID BOUNDS ASSERTION: " << msg << " (file: " << __FILE__      \
              << ")\n";                                                        \
    return false;                                                              \
  }
