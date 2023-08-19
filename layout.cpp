#include "vulkan/vulkan_core.h"
#include "common/Spv.hpp"
#include "common/Assert.hpp"

#define TEST_LAYOUT true

#if TEST_LAYOUT
#include "test/test.hpp"
#include "File.hpp"
#include <iostream>
#endif

// Docs/explanation for my understanding SPIRV image/texel buffer types and their mappings 
// to Vulkan descriptor types at the bottom of the file (Before tests)

using namespace Sol;

struct LayoutInfo {};

void get_layout_binding(
		Spv::Var var,
		HashMap<u16, Spv::Type> *types, 
		//HashMap<u16, Spv::DecoInfo> *deco_infos, 
		VkDescriptorSetLayoutBinding *binding) 
{
	if (types == nullptr || binding == nullptr)
		return;

    // @TODO Need a better implementation for handling shader access stages
	if (var.storage == Spv::Storage::UNIFORM || var.storage == Spv::Storage::STORAGE_BUFFER) {
		binding->stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        if (var.storage == Spv::Storage::UNIFORM)
		    binding->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        else
		    binding->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    }

	Spv::Type *var_type = types->find_cpy(var.type_id);
	if (var_type->name == Spv::Name::ARRAY) {
		binding->descriptorCount = var_type->arr.len;
		var_type = types->find_cpy(var_type->arr.type_id);
	} else { 
		binding->descriptorCount = 1;
	}

	// @SpvBug Idk what is the correct way to handle what resources are accessed from 
	// which shader stage. (linked to the above @TODO)
	// @Incomplete This needs to be tested against different containers, such as 
	// different descriptor types but contained in structs. Idk what effect this 
	// indirection will have.
	switch(var_type->name) {
	using Name = Spv::Name;
	// @TODO Handle inline uniform descriptor type (This will also need to be added to Spv type)
	case Name::SAMPLER:
	{
		binding->descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		binding->stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	}
	case Name::SAMPLED_IMAGE:
	{
		binding->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding->stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	}
	case Name::IMAGE:
	{
		Spv::Image img_type = var_type->image;
		switch(img_type.dim) {
			using Dim = Spv::Image::Dim;
			using Flags = Spv::Image::Flags;

			case Dim::D1:
			case Dim::D2:
			case Dim::D3:
			case Dim::CUBE:
			case Dim::RECT: 
			{
				binding->descriptorType = 
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				break;
			}
			case Dim::BUFFER:
			{
				if (img_type.flags & (u8)Flags::SAMPLED)
					binding->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
				else if (img_type.flags & (u8)Flags::READ_WRITE)
					binding->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;

				break;
			}
			case Dim::SUBPASS_DATA:
			{
				binding->descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
				break;
			}

			default:
				break;
		}
		break;
	}
    case Spv::Name::STRUCT:
        break;
	default:
        std::cout << "Type " << (u32)var_type->name << '\n';
		DEBUG_ASSERT(false, "Unsupported Type");
		break;
	}
}

// @TODO Handle immutablesamplers
// @TODO I think I can parse both fragment and vert shader side by side, and compare where samplers are used to understand their access stage
// @TODO streamline this when Spv::Serialize is completed
const u8 DESC_SET_COUNT = 4;
const u8 DESC_BINDING_COUNT = 4;
VkPipelineLayout get_layout(LayoutInfo *info, Spv *spv, VkDescriptorSetLayoutBinding **bindings) {
	auto var_iter = spv->vars.iter();
	auto *var_kv = var_iter.next();

	// @Incomplete this is a bit hackish, but minimum working version is good
    Spv::DecoInfo *deco_info;
	while(var_kv) {
		deco_info = spv->decorations.find_cpy(var_kv->key);

		if (deco_info == nullptr || (deco_info->flags & (u32)Spv::DecoFlagBits::DESC_SET) == 0) {
            var_kv = var_iter.next();
			continue;
        }

		DEBUG_ASSERT(deco_info->flags & (u32)Spv::DecoFlagBits::BINDING, 
				"Descriptor does not have a binding");

        bindings[deco_info->desc_set][deco_info->binding] = {};

        bindings[deco_info->desc_set][deco_info->binding].binding = deco_info->binding;
		get_layout_binding(var_kv->value, &spv->types, //&spv->decorations,
                &bindings[deco_info->desc_set][deco_info->binding]);

		var_kv = var_iter.next();
	}

    VkPipelineLayout ret;
    return ret;
}

#if TEST_LAYOUT

void test1(Spv spv) {
    VkDescriptorSetLayoutBinding *descs[DESC_SET_COUNT]; 
    for(int i = 0; i < DESC_SET_COUNT; ++i)
	    descs[i] = (VkDescriptorSetLayoutBinding*)lin_alloc(DESC_BINDING_COUNT * sizeof(VkDescriptorSetLayoutBinding));

    get_layout(nullptr, &spv, descs);
    spv.kill();


}

void run_tests() {
    TEST_MODULE_BEGIN("VkPipelineSetup", true, false);

    size_t code_size;
    const u32* pcode = File::read_spv(&code_size, "shaders/test_1.vert.spv", HEAP);
    bool ok;
    Spv spv = Spv::parse(code_size, pcode, &ok);
    TEST_EQ("ParseOk", ok, true, false);

    test1(spv);

    TEST_MODULE_END();
    heap_free((void*)pcode);
}

int main() {
    MemoryConfig mem_config;
    MemoryService::instance()->init(&mem_config);

    Test::Suite::instance()->init(false);

    run_tests();

    Test::Suite::instance()->kill();

    MemoryService::instance()->shutdown();
}

#endif

/* My understanding SPIRV image/sampled image types and their mappings to Vulkan types
typedef enum VkDescriptorType {
    VK_DESCRIPTOR_TYPE_SAMPLER = 0,
	// If an OpVariable's OpType points to OpTypeSampledImage, I understand this as being 
	// combined image sampler descriptor type. 
    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER = 1,

	// If an OpVariable's OpType points to an OpTypeImage, I understand this as requiring 
	// analysis of the Image's attributes to understand its descriptor type.
    VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE = 2,
    VK_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3,
    VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER = 4,
    VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER = 5,
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 6,
    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER = 7,
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC = 8,
    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC = 9,
    VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT = 10,
  // Provided by VK_VERSION_1_3
    VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK = 1000138000,
  // Provided by VK_KHR_acceleration_structure
    VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR = 1000150000,
  // Provided by VK_NV_ray_tracing
    VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV = 1000165000,
  // Provided by VK_QCOM_image_processing
    VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM = 1000440000,
  // Provided by VK_QCOM_image_processing
    VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM = 1000440001,
  // Provided by VK_EXT_mutable_descriptor_type
    VK_DESCRIPTOR_TYPE_MUTABLE_EXT = 1000351000,
  // Provided by VK_EXT_inline_uniform_block
    VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,
  // Provided by VK_VALVE_mutable_descriptor_type
    VK_DESCRIPTOR_TYPE_MUTABLE_VALVE = VK_DESCRIPTOR_TYPE_MUTABLE_EXT,
} VkDescriptorType;
*/

/* Old code for matching specific descriptor types

*/
