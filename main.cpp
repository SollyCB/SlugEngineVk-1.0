#include "Allocator.hpp"
#include "Engine.hpp"
#include "glTF.hpp"

using namespace Sol;

int main() {
    MemoryConfig mem_config;
    MemoryService::instance()->init(&mem_config);

    Engine::instance()->init();
    Engine::instance()->run();
    Engine::instance()->kill();

    MemoryService::instance()->shutdown();
    return 0;
}
