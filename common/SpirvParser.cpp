#include <iostream>
#include <spirv_cross/spirv.h>
#include <vulkan/vulkan.hpp>

#include "SpirvParser.hpp"

bool parseSpv(uint32_t *data, size_t dataSize) {

  if (data == nullptr) {
    std::cerr << "SPIRV DATA IS NOT SAFE TO READ!! SIGSEGV POTENTIAL\n";
    return false;
  }

  bool magic = 0x07230203 == data[0] ? true : false;
  if (!magic) {
    std::cout << "Failure! Magic SPIRV number (0x07230203) does not match!";
    return false;
  }

  uint32_t wordCount;
  if (dataSize % 4 == 0)
    wordCount = dataSize / 4;
  else {
    std::cout << "SPIRV data does not fit size / 4 requirements!\n";
    return false;
  }

  size_t idBounds = data[3];
  std::vector<Id> ids(idBounds);
  /*for(size_t i = 0; i < idBounds; ++i) {
    Id id;
    ids[i] = id;
  }*/
  ParsedSpv parsed;

  size_t wordIndex = 5;
  while (wordIndex < wordCount) {
    SpvOp opCode = (SpvOp)(data[wordIndex] & 0xFF);
    uint16_t wordCount = (uint16_t)(data[wordIndex] >> 16);

    switch (opCode) {
    case (SpvOpEntryPoint): {
      assert_count(4, "SpvEntryPoint");
      parsed.model = (SpvExecutionModel)(data[wordIndex + 1]);
      parsed.stage = parseExecutionModel(parsed.model);
      assert_stage("Vertex");
      break;
    }
    case (SpvOpDecorate): {
      assert_count(3, "SpvOpDecorate");
      uint32_t idIndex = data[wordIndex + 1];
      assert_idBound("SpvOpDecorate");
      Id &id = ids[idIndex];
      SpvDecoration decoration = (SpvDecoration)(data[wordIndex + 2]);
      switch (decoration) {
      case (SpvDecorationBinding): {
        id.binding = data[wordIndex + 3];
        break;
      }
      case (SpvDecorationDescriptorSet): {
        id.set = data[wordIndex + 3];
        break;
      }
      default:
        break;
      }
      break;
    }
    case (SpvOpMemberDecorate): {
      assert_count(4, "SpvOpMemberDecorate");
      uint32_t idIndex = data[wordIndex + 1];
      assert_idBound("SpvOpMemberDecorate");
      Id &id = ids[idIndex];
      uint32_t memberIndex = data[wordIndex + 2];

      if (id.members.capacity() < memberIndex && memberIndex < 64)
        id.members.resize(64);
      else
        id.members.resize(memberIndex * 2);

      Member &member = id.members[memberIndex];
      SpvDecoration decoration = (SpvDecoration)data[wordIndex + 3];
      switch (decoration) {
      case (SpvDecorationOffset): {
        member.offset = data[wordIndex + 4];
        break;
      }
      default:
        break;
      }
      break;
    }
    case (SpvOpName): {
      assert_count(3, "SpvOpName");
      uint32_t idIndex = data[wordIndex + 1];
      assert_idBound("SpvOpName");
      Id &id = ids[idIndex];
      id.name = (char *)(data + (wordIndex + 2));
      break;
    }
    case (SpvOpMemberName): {
      assert_count(4, "SpvOpMemberName");
      uint32_t idIndex = data[wordIndex + 1];
      assert_idBound("SpvOpMemberName");
      Id &id = ids[idIndex];
      uint32_t memberIndex = data[wordIndex + 2];

      if (id.members.capacity() < memberIndex && memberIndex > 64)
        id.members.resize(memberIndex * 2);
      else
        id.members.resize(64);

      Member &member = id.members[memberIndex];
      member.name = (char *)(data + (wordIndex + 3));
      break;
    }
    case (SpvOpTypeVector): {
      assert_count(4, "SpvOpTypeVector");
      uint32_t idIndex = data[wordIndex + 1];
      assert_idBound("SpvOpTypeVector");

      Id &id = ids[idIndex];
      id.opCode = opCode;
      id.typeIndex = data[wordIndex + 2];
      id.length = data[wordIndex + 3];
      break;
    }
    case (SpvOpTypeMatrix): {
      assert_count(4, "SpvOpTypeMatrix");
      uint32_t idIndex = data[wordIndex + 1];
      assert_idBound("SpvOpTypeMatrix");

      Id &id = ids[idIndex];
      id.opCode = opCode;
      id.typeIndex = data[wordIndex + 2];
      id.length = data[wordIndex + 3];
      break;
    }
    case (SpvOpTypeSampler): {
      assert_count(2, "SpvOpTypeSampler");
      uint32_t idIndex = data[wordIndex + 1];
      assert_idBound("SpvOpTypeSampler");
      Id &id = ids[idIndex];
      id.opCode = opCode;
      break;
    }
    case (SpvOpTypeImage): {
      assert_count(9, "SpvOpTypeImage");
      uint32_t idIndex = data[wordIndex + 1];
      assert_idBound("SpvOpTypeImage");
      Id &id = ids[idIndex];
      id.typeIndex = data[wordIndex + 2];
      id.opCode = opCode;

      Image image;
      image.dim = data[wordIndex + 3];
      image.depth = data[wordIndex + 4];
      image.arrayed = data[wordIndex + 5];
      image.MS = data[wordIndex + 6];
      image.sampled = data[wordIndex + 7];
      image.format = data[wordIndex + 8];
      id.image = image;
      break;
    }
    case (SpvOpTypeSampledImage): {
      assert_count(3, "SpvOpTypeImage");
      uint32_t idIndex = data[wordIndex + 1];
      assert_idBound("SpvOpTypeSampledImage");
      Id &id = ids[idIndex];
      id.opCode = opCode;
      id.typeIndex = data[wordIndex + 2];
      break;
    }
    case (SpvOpVariable): {
      assert_count(4, "SpvOpVariable");
      uint32_t idIndex = data[wordIndex + 2];
      assert_idBound("SpvOpVariable");
      Id &id = ids[idIndex];
      id.opCode = opCode;
      id.typeIndex = data[wordIndex + 1];
      id.storageClass = (SpvStorageClass)data[wordIndex + 3];
      break;
    }
    case (SpvOpTypePointer): {
      assert_count(4, "SpvOpTypePointer");
      uint32_t idIndex = data[wordIndex + 1];
      assert_idBound("SpvOpTypePointer");
      Id &id = ids[idIndex];
      id.opCode = opCode;
      // id.storageClass = (SpvStorageClass)words[wordIndex+2]; <- I think this
      // is unnessecary as the type itself will set the type when i get to that
      // byte but im not sure.
      id.typeIndex = data[wordIndex + 3];
      break;
    }
    case (SpvOpTypeFloat): {
      assert_count(3, "SpvOpTypeFloat");
      uint32_t idIndex = data[wordIndex + 1];
      assert_idBound("SpvOpTypeFloat");
      Id &id = ids[idIndex];
      id.opCode = opCode;
      id.width = data[wordIndex + 2];
      break;
    }
    case (SpvOpTypeInt): {
      assert_count(4, "SpvOpTypeInt");
      uint32_t idIndex = data[wordIndex + 1];
      assert_idBound("SpvOpTypeInt");
      Id &id = ids[idIndex];
      id.opCode = opCode;
      id.width = data[wordIndex + 2];
      id.sign = data[wordIndex + 3];
      break;
    }
    case (SpvOpConstant): {
      assert_count(4, "SpvOpConstant");
      uint32_t idIndex = data[wordIndex + 2];
      assert_idBound("SpvOpConstant");
      Id &id = ids[idIndex];
      id.opCode = opCode;
      id.typeIndex = data[wordIndex + 1];
      id.quiv =
          data[wordIndex +
               3]; // NOTE: this assumes 32bit wide max on all constants...
      break;
    }
    case (SpvOpTypeArray): {
      assert_count(4, "SpvOpTypeArray");
      uint32_t idIndex = data[wordIndex + 1];
      assert_idBound("SpvOpTypeArray");
      Id &id = ids[idIndex];
      id.opCode = opCode;
      id.typeIndex = data[wordIndex + 2];
      id.length = data[wordIndex + 3];
      break;
    }
    case (SpvOpTypeRuntimeArray): {
      assert_count(3, "SpvOpTypeRuntimeArray");
      uint32_t idIndex = data[wordIndex + 1];
      assert_idBound("SpvOpTypeRuntimeArray");
      Id &id = ids[idIndex];
      id.opCode = opCode;
      id.typeIndex = data[wordIndex + 2];
      break;
    }
    case (SpvOpTypeStruct): {
      assert_count(2, "SpvOpTypeStruct");
      uint32_t idIndex = data[wordIndex + 1];
      assert_idBound("SpvOpTypeStruct");

      Id &id = ids[idIndex];
      if (id.members.capacity() == 0)
        id.members.resize(64);
      if (wordCount > 2)
        for (uint32_t memberIndex = 0; memberIndex < (size_t)(wordCount - 2);
             ++memberIndex) {
          id.members[memberIndex].typeIndex = data[wordIndex + memberIndex + 2];
        }

      break;
    }
    default:
      break;
    }

    wordIndex += wordCount;
  }

  for (size_t idIndex = 0; idIndex < ids.size(); ++idIndex) {
    Id &id = ids[idIndex];
    if (id.opCode == SpvOpVariable) {
      switch (id.storageClass) {
      case (SpvStorageClassUniform):
      case (SpvStorageClassUniformConstant): {
        /*if ( id.set == 1 && ( id.binding == k_bindless_texture_binding ||
        id.binding == ( k_bindless_texture_binding + 1 ) ) ) {
            // NOTE(marco): these are managed by the GPU device
            continue;
        }*/ // This is something to pay attention later...
        Id &uniformType = ids[ids[id.typeIndex].typeIndex];

        /*
         * create VkDescriptorSetLayout with the parsed shader info
         */

        switch (uniformType.opCode) {
        case (SpvOpTypeStruct): {
          // This is a VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
        }
        case (SpvOpTypeSampledImage): {
          // This is a VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
        }
        default:
          break;
        }
      }
      default:
        break;
        /*
         * add binding to the layout at correct index
         */
      }
    }
    // free id.members for this id
  }
  // free ids list of ids

  // pretty sure that this is fine (regarding the string names that are pointers
  // to spvBytes) as all of the information should be uploaded to Vulkan through
  // the creation of the DescriptorSetLayouts, but I am not certain. So check
  // this when fully implemented... They allocate the string buffer for the
  // names and the pointer for the data outside of their function, and the
  // pointer that they also give is a u32
  return true;
}

VkShaderStageFlags parseExecutionModel(SpvExecutionModel model) {
  switch (model) {
  case (SpvExecutionModelVertex):
    return VK_SHADER_STAGE_VERTEX_BIT;
  default:
    break;
  }
  return 0;
}
