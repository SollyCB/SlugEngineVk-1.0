#include "Engine.hpp"
#include "Allocator.hpp"
#include "Vec.hpp"
#include "glTF.hpp"

using namespace Sol;

int main() {
  MemoryConfig mem_config;
  MemoryService::instance()->init(&mem_config);

  const char* model_file_name = "test_1.json";
  glTF::Json model_file;
  bool ok = glTF::read_json(model_file_name, &model_file);

  glTF::glTF gltf;
  if (!ok)
    std::cerr << "ALERT! '" << model_file_name << "' does not exist\n";
  else {
    gltf.fill(model_file);
    std::cout << "Loaded Model '" << model_file_name << "', gltf version: " << gltf.asset.version.c_str() 
      << ", Copyright: '" << gltf.asset.copyright.c_str() << "'\n";
  }

  Engine::instance()->init();
  Engine::instance()->run();
  Engine::instance()->kill();

  MemoryService::instance()->shutdown();
  return 0;
}
