#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

#include "glTF.hpp"
#include "nlohmann/json.hpp"
#include "VulkanErrors.hpp"

namespace Sol {
namespace glTF {

const int32_t INVALID_INDEX = -1;
const uint32_t INVALID_COUNT = UINT32_MAX;
const float INVALID_FLOAT = std::numeric_limits<float>::max();

const uint8_t PNG_BYTE_PATTERN[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
const uint8_t JPG_BYTE_PATTERN[3] = { 0xFF, 0xD8, 0xFF };

const int32_t NEAREST_FALLBACK = 9728;
const int32_t LINEAR_FALLBACK = 9729;

bool read_json(const char* file, Json *json) {
  std::ifstream f(file);
  if (!f.is_open())
    return false;

  *json = Json::parse(f);
  f.close();
  return true;
}

void glTF::fill(Json json) {
  asset.fill(json);
  scenes.fill(json); 
  nodes.fill(json);
  buffers.fill(json);
  buffer_views.fill(json);
  accessors.fill(json);
  meshes.fill(json);
  skins.fill(json);
  textures.fill(json);
  images.fill(json);
  samplers.fill(json);
  materials.fill(json);
  cameras.fill(json);
  animations.fill(json);
}

namespace { 
  template<typename T>
  static bool load_T(Json json, const char* key, T *obj) {
    auto tmp = json.find(key);
    if (tmp == json.end())
      return false;

    *obj = tmp.value();
    return true;
  }
  static bool load_string(Json json, const char* key, StringBuffer *str) {
    auto obj = json.find(key);
    if (obj == json.end())
      return false;

    std::string tmp = json.value(key, "");
    str->init(tmp.length());
    str->copy_here(tmp, tmp.length());
    return true;
  }
  template<typename T>
  static bool load_array(Json json, const char* key, Array<T> *array) {
    auto obj = json.find(key);
    if (obj == json.end())
      return false;

    size_t size = obj.value().size();
    array->init(size, 8);
    return true;
  }
  template<typename T>
  static void fill_obj_array(Json json, const char* key, Array<T> *array) {
    for(auto i : json[key]) {
      T t;
      t.fill(i);
      array->push(t);
    }
  }
  static void fill_str_array(Json json, const char* key, Array<StringBuffer> *array) {
    for(auto i : json[key]) {
      std::string str = i;
      StringBuffer buf = StringBuffer::get(str.length(), str);
      array->push(buf);
    }
  }

  static bool check_joints_weights_count(Array<Mesh::Primitive::Attribute> *attrs) {
    uint32_t count_w = 0;
    uint32_t count_j = 0;
    const char* w = "WEIGHTS";
    const char* j = "JOINTS";
    for(int i = 0; i < attrs->len; ++i) {
      StringBuffer str = (*attrs)[i].key;
      int w_check = memcmp(w, str.c_str(), 7);
      int j_check = memcmp(j, str.c_str(), 6);

      if (w_check == 0)
        ++count_w;
      if (j_check == 0)
        ++count_j;
    }
    if (count_j != count_w)
      return false;
    else 
      return true;
  }
}

// Asset /////////////////////////
void Asset::fill(Json json) {
  auto asset = json.find("asset");
  ABORT(asset != json.end(), "glTF has no 'asset' obj");

  load_string(asset.value(), "version", &version);
  ABORT(version.str, "glTF asset has no 'version' field");

  load_string(asset.value(), "copyright", &copyright);
}

// Scenes ///////////////////////
void Scenes::fill(Json json) {
  load_T(json, "scene", &scene);
  load_array(json, "scenes", &scenes);
  fill_obj_array(json, "scenes", &scenes);
}
void Scene::fill(Json json) {
  load_string(json, "name", &name);
  load_array(json, "nodes", &nodes);
  for(auto i : json["nodes"]) 
    nodes.push(i);
}

// Nodes ////////////////////////
void Nodes::fill(Json json) {
  load_array(json, "nodes", &nodes);
  fill_obj_array(json, "nodes", &nodes);
}
void Node::fill(Json json) {
  load_string(json, "name", &name);
  load_T(json, "mesh", &mesh);
  load_T(json, "camera", &camera);
  load_T(json, "skin", &skin);

  load_array(json, "rotation", &rotation);
  for(auto i : json["rotation"])
    rotation.push(i);
  load_array(json, "scale", &scale);
  for(auto i : json["scale"])
    scale.push(i);
  load_array(json, "translation", &translation);
  for(auto i : json["translation"])
    translation.push(i);
  load_array(json, "weights", &weights);
  for(auto i : json["weights"])
    weights.push(i);

  load_array(json, "matrix", &matrix);
  for(auto i : json["matrix"])
    matrix.push(i);

  load_array(json, "children", &children);
  for(auto i : json["children"])
    children.push(i);
}

// Buffers & BufferViews //////////////////////
void Buffers::fill(Json json) {
  load_array(json, "buffers", &buffers);
  fill_obj_array(json, "buffers", &buffers);
}
void Buffer::fill(Json json) {
  load_T(json, "byteLength", &byte_length);
  load_string(json, "uri", &uri);
}

void BufferViews::fill(Json json) {
  load_array(json, "bufferViews", &views);
  fill_obj_array(json, "bufferViews", &views);
}
void BufferView::fill(Json json) {
  load_T(json, "buffer", &buffer);
  load_T(json, "byteLength", &byte_length);
  load_T(json, "byteOffset", &byte_offset);
  load_T(json, "byteStride", &byte_stride);
  load_T(json, "target", &target);
}

// Accessors ///////////////////////
void Accessors::fill(Json json) {
  load_array(json, "accessors", &accessors);
  fill_obj_array(json, "accessors", &accessors);
}
void Accessor::fill(Json json) {
  load_array(json, "max", &max);
  for(auto i : json["max"]) 
    max.push(i);
  load_array(json, "min", &min);
  for(auto i : json["min"])
    min.push(i);

  StringBuffer tmp;
  load_string(json, "type", &tmp);
  if(strcmp("SCALAR", tmp.c_str()) == 0)
    type = SCALAR;
  if(strcmp("VEC2", tmp.c_str()) == 0)
    type = VEC2;
  if(strcmp("VEC3", tmp.c_str()) == 0)
    type = VEC3;
  if(strcmp("VEC4", tmp.c_str()) == 0)
    type = VEC4;
  if(strcmp("MAT2", tmp.c_str()) == 0)
    type = MAT2;
  if(strcmp("MAT3", tmp.c_str()) == 0)
    type = MAT3;
  if(strcmp("MAT4", tmp.c_str()) == 0)
    type = MAT4;

  load_T(json, "componentType", &component_type);
  load_T(json, "byteOffset", &byte_offset);
  load_T(json, "count", &count);
  load_T(json, "bufferView", &buffer_view);

  Json json_sparse;
  if (load_T(json, "sparse", &json_sparse)) {
    sparse.fill(json_sparse);
  }
}
void Accessor::Sparse::fill(Json json) {
  load_T(json, "count", &count);
  
  Json json_indices;
  if (load_T(json, "indices", &json_indices)) {
    indices.fill(json_indices);
  }
  Json json_values;
  if (load_T(json, "values", &json_values)) {
    values.fill(json_values);
  }
}
void Accessor::Sparse::Indices::fill(Json json) {
  load_T(json, "bufferView", &buffer_view);
  load_T(json, "byteOffset", &byte_offset);
  load_T(json, "componentType", &component_type);
}
void Accessor::Sparse::Values::fill(Json json) {
  load_T(json, "bufferView", &buffer_view);
  load_T(json, "byteOffset", &byte_offset);
}

// Meshes ////////////////////
void Meshes::fill(Json json) {
  load_array(json, "meshes", &meshes);
  for(auto i : json["meshes"]) {
    Mesh mesh;
    mesh.fill(i);
    meshes.push(mesh);
  }
}
void Mesh::fill(Json json) {
  load_array(json, "primitives", &primitives);  
  fill_obj_array(json, "primitives", &primitives);  

  load_array(json, "weights", &weights);
  for(auto i : json["weights"])
    weights.push(i);

  extras.fill(json);
}
void Mesh::Primitive::fill(Json json) {
  load_T(json, "indices", &indices);
  load_T(json, "material", &material);
  load_T(json, "mode", &mode);

  if (load_array(json, "attributes", &attributes))
    fill_attrib_array(json["attributes"], &attributes);
  if (load_array(json, "targets", &targets))
    fill_obj_array(json, "targets", &targets);

  bool check = check_joints_weights_count(&attributes);
  ABORT(check, "Primitive JOINTS_n count != WEIGHTS_n count");
}

void Mesh::Primitive::Target::fill(Json json) {
  attributes.init(json.size(), 8);
  if (attributes.cap)
    fill_attrib_array(json, &attributes);
}
void Mesh::Primitive::fill_attrib_array(Json json, Array<Attribute> *attributes) {
  for(auto i : json.items()) {
    Attribute attrib;
    std::string str = i.key();
    attrib.key = StringBuffer::get(str.length(), str);
    attrib.accessor = i.value();
    attributes->push(attrib);
  }
}
void Mesh::Extras::fill(Json json) {
  load_array(json, "targetNames", &target_names);
  fill_str_array(json, "targetNames", &target_names);
}

// Skins ////////////////////
void Skins::fill(Json json) {
  load_array(json, "skins", &skins);
  fill_obj_array(json, "skins", &skins);
}
void Skin::fill(Json json) {
  load_T(json, "inverseBindMatrices", &i_bind_matrices);
  load_T(json, "skeleton", &skeleton);
  load_array(json, "joints", &joints);
  for(auto i : json["joints"])
    joints.push(i);
}

// Textures ////////////////
void Textures::fill(Json json) {
  load_array(json, "textures", &textures);
  fill_obj_array(json, "textures", &textures);
}
void Texture::fill(Json json) {
  load_T(json, "sampler", &sampler);
  load_T(json, "source", &source);
}

// Images ////////////////
void Images::fill(Json json) {
  load_array(json, "images", &images);
  fill_obj_array(json, "images", &images);
}
void Image::fill(Json json) {
  load_string(json, "uri", &uri);
  load_T(json, "bufferView", &buffer_view);
  StringBuffer tmp;
  load_string(json, "mimeType", &tmp);

  // NOTE:: This used to SEGFAULT as c_str() was being called on a potentially uninitialised StringBuffer 
  // (load_string returns before StringBuffer::init() when the key is not found);
  if (strcmp(tmp.c_str(), "image/jpeg") == 0)
    mime_type = JPG;
  if (strcmp(tmp.c_str(), "image/png") == 0)
    mime_type = PNG;
}

// Samplers //////////////
void Samplers::fill(Json json) {
  load_array(json, "samplers", &samplers);
  fill_obj_array(json, "samplers", &samplers);
}
void Sampler::fill(Json json) {
  load_T(json, "magFilter", &mag_filter);
  load_T(json, "minFilter", &min_filter);
  load_T(json, "wrapS", &wrap_s);
  load_T(json, "wrapT", &wrap_t);
}

// Materials ///////////////////
void Materials::fill(Json json) {
  load_array(json, "materials", &materials);
  fill_obj_array(json, "materials", &materials);
}
void Material::fill(Json json) {
  load_string(json, "name", &name);
  load_T(json, "alphaCutoff", &alpha_cutoff);
  load_T(json, "doubleSided", &double_sided);
  load_array(json, "emissiveFactor", &emissive_factor);
  for(auto i : json["emissiveFactor"])
    emissive_factor.push(i);

  StringBuffer tmp;
  load_string(json, "alphaMode", &tmp);
  if (strcmp(tmp.c_str(), "OPAQUE") == 0)
    alpha_mode = OPAQUE;
  if (strcmp(tmp.c_str(), "MASK") == 0)
    alpha_mode = MASK;
  if (strcmp(tmp.c_str(), "BLEND") == 0)
    alpha_mode = BLEND;

  pbr_metallic_roughness.fill(json);
  normal_texture.fill_tex(json, "normalTexture");
  emissive_texture.fill_tex(json, "emissiveTexture");
  occlusion_texture.fill_tex(json, "occlusionTexture");
}
void Material::MatTexture::fill_tex(Json json, const char* key) {
  auto tmp = json.find(key);
  if (tmp == json.end())
    return;
  else
    json = tmp.value();

  load_T(json, "scale", &scale);
  load_T(json, "strength", &scale);
  load_T(json, "index", &index);
  load_T(json, "texCoord", &tex_coord);
}
void Material::PbrMetallicRoughness::fill(Json json) {
  auto tmp = json.find("pbrMetallicRoughness");
  if (tmp == json.end())
    return;
  else
    json = tmp.value();

  load_array(json, "baseColorFactor", &base_color_factor);
  for(auto i : json["baseColorFactor"])
    base_color_factor.push(i);
  base_color_texture.fill_tex(json, "baseColorTexture"); 
  metallic_roughness_texture.fill_tex(json, "metallicRoughnessTexture"); 
  load_T(json, "metallicFactor", &metallic_factor);
  load_T(json, "roughnessFactor", &roughness_factor);
}

// Cameras /////////////////////
void Cameras::fill(Json json) {
  load_array(json, "cameras", &cameras);
  fill_obj_array(json, "cameras", &cameras);
}
void Camera::fill(Json json) {
  load_string(json, "name", &name);

  StringBuffer tmp;
  load_string(json, "type", &tmp);
  if (strcmp(tmp.c_str(), "perspective") == 0)
    type = PERSPECTIVE;
  if (strcmp(tmp.c_str(), "orthographic") == 0)
    type = ORTHO;
  ABORT(type != UNKNOWN, "glTF model camera type must be defined");

  load_T(json, "aspectRatio", &aspect_ratio);
  load_T(json, "yfov", &yfov);
  load_T(json, "xmag", &xmag);
  load_T(json, "ymag", &ymag);
  load_T(json, "zfar", &zfar);
  load_T(json, "znear", &znear);

  if (type == ORTHO) {
    load_T(json["orthographic"], "xmag", &xmag);
    load_T(json["orthographic"], "ymag", &ymag);
    load_T(json["orthographic"], "zfar", &zfar);
    load_T(json["orthographic"], "znear", &znear);

    ABORT(xmag != INVALID_FLOAT, "glTF model Ortho camera must have XMAG defined");
    ABORT(ymag != INVALID_FLOAT, "glTF model Ortho camera must have YMAG defined");
    ABORT(zfar != INVALID_FLOAT, "glTF model Ortho camera must have ZFAR defined");
    ABORT(znear != INVALID_FLOAT, "glTF model Ortho camera must have ZNEAR defined");
  }
  if (type == PERSPECTIVE) {
    load_T(json["perspective"], "aspectRatio", &aspect_ratio);
    load_T(json["perspective"], "yfov", &yfov);
    load_T(json["perspective"], "zfar", &zfar);
    load_T(json["perspective"], "znear", &znear);

    ABORT(yfov != INVALID_FLOAT, "glTF model Perspective camera must have YFOV defined");
    ABORT(znear != INVALID_FLOAT, "glTF model Perspective camera must have ZNEAR defined");
  }
}

// Animations ////////////////////
void Animations::fill(Json json) {
  load_array(json, "animations", &animations);
  fill_obj_array(json, "animations", &animations);
}
void Animation::fill(Json json) {
  load_string(json, "name", &name);

  load_array(json, "channels", &channels);
  fill_obj_array(json, "channels", &channels);
  load_array(json, "samplers", &samplers);
  fill_obj_array(json, "samplers", &samplers);
}
void Animation::Channel::fill(Json json) {
  load_T(json, "sampler", &sampler);
  load_T(json["target"], "node", &target.node);

  StringBuffer tmp;
  load_string(json["target"], "path", &tmp);
  if (strcmp(tmp.c_str(), "rotation") == 0)
    target.path = Target::ROTATION;
  if (strcmp(tmp.c_str(), "translation") == 0)
    target.path = Target::TRANSLATION;
  if (strcmp(tmp.c_str(), "scale") == 0)
    target.path = Target::SCALE;
  if (strcmp(tmp.c_str(), "weights") == 0)
    target.path = Target::WEIGHTS;
}
void Animation::Sampler::fill(Json json) {
  load_T(json, "input", &input);
  load_T(json, "output", &output);

  StringBuffer tmp;
  load_string(json, "interpolation", &tmp);
  if(strcmp(tmp.c_str(), "LINEAR") == 0)
    interpolation = LINEAR;
  if(strcmp(tmp.c_str(), "STEP") == 0)
    interpolation = STEP;
  if(strcmp(tmp.c_str(), "CUBICSPLINE") == 0)
    interpolation = CUBICSPLINE;
}

} // namespace glTF
} // namespace Sol
