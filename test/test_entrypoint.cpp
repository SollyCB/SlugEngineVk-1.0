#include "test.hpp"
#include "Allocator.hpp"
#include "spv_test.hpp"

using namespace Sol;

int main() {
    MemoryConfig config = {};
    MemoryService::instance()->init(&config);
    Test::Suite::instance()->init(false);

    Test::Spv c;
    c.run();

    Test::Suite::instance()->kill();
    MemoryService::instance()->shutdown();
    return 0;
}
