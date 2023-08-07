#include <iostream>

#include "Spv.hpp"
#include "Assert.hpp"
#include "Array.hpp"

/*
 *  TODO:(Sol):
 *  1. Forward referencing in spirv:
 *  	As I understand it, for my use case they do not need to be handled,
 *  	as they are handled sufficiently by collecting OpTypePointers.
 */

namespace Sol {

static const uint32_t magic = 0x07230203;
//static LinearAllocator *scratch = &MemoryService::instance()->scratch_allocator;

template<typename T>
inline T* convert(void *ptr) {
    return reinterpret_cast<T*>(ptr);
}

Spv Spv::parse(size_t code_size, const uint32_t *spirv, bool *ok)
{
    Spv ret = {};
    if (spirv[0] != magic) {
        if (ok != nullptr)
            *ok = false;
        return ret;
    }

#if SPV_DEBUG
    DebugInfo debug = {};
    ret.debug.init(16);
#endif

    ret.types.init(64);
    ret.scratch_mark = SCRATCH->get_mark();
    Type type = {};
    Var var = {};

    uint16_t inc = 5;
    while (inc < code_size / 4) {
        const uint16_t *info = reinterpret_cast<const uint16_t*>(spirv + inc);
        const uint32_t *instr = spirv + inc;

        switch (info[0]) {
        // stage
			case 15:
            {
				// TODO:(Sol): Support other shader stages
				const uint32_t *model = instr + 1;
				switch (*model) {
				case 0:
					ret.stage = VK_SHADER_STAGE_VERTEX_BIT;
					break;
				case 4:
					ret.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
					break;
				default:
					break;
				}
				ret.p_name = reinterpret_cast<const char*>(model + 2);
				break;
			}
            // OpDecorate
            case 71:
            {
                Var* var_ref = ret.get_T<Var>(*(instr + 1), Name::VAR);
                Deco deco = static_cast<Deco>(*(instr + 2));
                uint32_t literal = *(instr + 3);

                switch(deco) {
                    case Deco::BLOCK:
                    {
                        var_ref->deco_info.flags |= static_cast<uint32_t>(DecoFlagBits::BLOCK);
                        break;
                    }
                    case Deco::ROW_MAJOR:
                    {
                        var_ref->deco_info.flags |= static_cast<uint32_t>(DecoFlagBits::ROW_MAJOR);
                        break;
                    }
                    case Deco::COL_MAJOR:
                    {
                        var_ref->deco_info.flags |= static_cast<uint32_t>(DecoFlagBits::COL_MAJOR);
                        break;
                    }
                    case Deco::ARRAY_STRIDE:
                    {
                        var_ref->deco_info.flags |= static_cast<uint32_t>(DecoFlagBits::ARRAY_STRIDE);
                        var_ref->deco_info.array_stride = literal;
                        break;
                    }
                    case Deco::MAT_STRIDE:
                    {
                        var_ref->deco_info.flags |= static_cast<uint32_t>(DecoFlagBits::MAT_STRIDE);
                        var_ref->deco_info.mat_stride = literal;
                        break;
                    }
                    case Deco::BUILTIN:
                    {
                        var_ref->deco_info.flags |= static_cast<uint32_t>(DecoFlagBits::BUILTIN);
                        break;
                    }
                    case Deco::CONSTANT:
                    {
                        var_ref->deco_info.flags |= static_cast<uint32_t>(DecoFlagBits::CONSTANT);
                        break;
                    }
                    case Deco::UNIFORM:
                    {
                        var_ref->deco_info.flags |= static_cast<uint32_t>(DecoFlagBits::UNIFORM);
                        break;
                    }
                    case Deco::LOCATION:
                    {
                        var_ref->deco_info.flags |= static_cast<uint32_t>(DecoFlagBits::LOCATION);
                        var_ref->deco_info.location = literal;
                        break;
                    }
                    case Deco::COMPONENT:
                    {
                        var_ref->deco_info.flags |= static_cast<uint32_t>(DecoFlagBits::COMPONENT);
                        var_ref->deco_info.component = literal;
                        break;
                    }
                    case Deco::BINDING:
                    {
                        var_ref->deco_info.flags |= static_cast<uint32_t>(DecoFlagBits::BINDING);
                        var_ref->deco_info.binding = literal;
                        break;
                    }
                    case Deco::DESC_SET:
                    {
                        var_ref->deco_info.flags |= static_cast<uint32_t>(DecoFlagBits::DESC_SET);
                        var_ref->deco_info.desc_set = literal;
                        break;
                    }
                    case Deco::OFFSET:
                    {
                        var_ref->deco_info.flags |= static_cast<uint32_t>(DecoFlagBits::OFFSET);
                        var_ref->deco_info.offset = literal;
                        break;
                    }
                    default:
                        break;
                }

                break;
            }
            // OpMemberDecorate
            case 72:
            {
                // TODO:(Sol):
                break;
            }
            // OpVar
            case 59:
            {
                Var *var = ret.get_T<Var>(*(instr + 2), Name::VAR);
                var->ptr_id = *(instr + 1);

                break;
            }
            // OpTypePointer
            case 32:
            {
                Ptr *ptr = ret.get_T<Ptr>(*(instr + 1), Name::PTR);
                ptr->storage = static_cast<Storage>(*(instr + 2));
                ptr->type_id = *(instr + 3);

                break;
            }
			// OpTypeForwardPointer
			case 39:
			{
				DEBUG_ASSERT(false,
						"OpTypePointer, confirm that this indeed does need to be \
						handled for my use case...");
				break;
			}
            // OpTypeVoid
            case 19:
            {
                ret.new_optype<int>(instr, nullptr, Name::VOID);
                break;
            }
            // OpTypeBool
            case 20:
            {
                ret.new_optype<int>(instr, nullptr, Name::BOOL);
                break;
            }
            // OpTypeInt
            case 21:
            {
                Int integer = {*(instr + 2), static_cast<bool>(*(instr + 3))};
                ret.new_optype<Int>(instr, &integer, Name::INT);
                break;
            }
            // OpTypeFloat
            case 22:
            {
                Float floating_point = {*(instr + 2)};
                ret.new_optype<Float>(instr, &floating_point, Name::FLOAT);
                break;
            }
            // OpTypeVector
            case 23:
            {
                Vector vec = {*(instr + 2), *(instr + 3)};
                ret.new_optype<Vector>(instr, &vec, Name::VEC);
                break;
            }
            // OpTypeMatrix
            case 24:
            {
                Matrix mat = {*(instr + 2), *(instr + 3)};
                ret.new_optype<Matrix>(instr, &mat, Name::MATRIX);
                break;
            }
            // OpTypeImage
            case 25:
            {
                Image image = {};
                image.type_id = *(instr + 2);
                image.dim = static_cast<Image::Dim>(*(instr + 3));
                image.depth = static_cast<Image::Depth>(*(instr + 4));
                image.array = *(instr + 5);
                image.multisampled = *(instr + 6);
                image.sampled = static_cast<Image::Sampled>(*(instr + 7));
                image.format = static_cast<Image::Format>(*(instr + 8));

                ret.new_optype<Image>(instr, &image, Name::IMAGE);
                break;
            }
            // OpTypeSampler
            case 26:
            {
                ret.new_optype<int>(instr, nullptr, Name::SAMPLER);
                break;
            }
            // OpTypeSampledImage
            case 27:
            {
                SampledImage image = {*(instr + 2)};
                ret.new_optype(instr, &image, Name::SAMPLED_IMAGE);
                break;
            }
            // OpTypeArray
            case 28:
            {
                Array array = {*(instr + 2), *(instr + 3)};
                ret.new_optype(instr, &array, Name::ARRAY);
                break;
            }
            // OpTypeRuntimeArray
            case 29:
            {
                RunTimeArray array = {*(instr + 1)};
                ret.new_optype(instr, &array, Name::RUNTIME_ARRAY);
                break;
            }
            // OpTypeStruct
            case 30:
            {
                Struct structure = {};
                structure.count = static_cast<uint32_t>(info[1] - 2);
                structure.type_ids = reinterpret_cast<uint32_t*>(lin_alloc(structure.count * sizeof(uint32_t)));
                for(uint32_t i = 0; i < structure.count; ++i)
                    structure.type_ids[i] = *(instr + 2 + i);
                ret.new_optype(instr, &structure, Name::STRUCT);
                break;
            }


#if SPV_DEBUG
            // OpName
            case 5:
            {
                debug.id = *(instr + 1);
                debug.name = reinterpret_cast<const char*>(instr + 2);
                ret.debug.push(debug);

                break;
            }
#endif
        }

        inc += info[1];
    }
    if (ok != nullptr)
        *ok = true;
    return ret;
}

void Spv::kill() {
    SCRATCH->cut_diff(scratch_mark);
    types.kill();

#if SPV_DEBUG
    debug.kill();
#endif
}

} // namespace Sol
