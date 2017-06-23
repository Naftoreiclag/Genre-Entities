#include "pegr/script/Script.hpp"

namespace pegr {
namespace Test {

//@Test Initialization Sanity Test
void test_0000_init_sanity() {
    if (!Script::is_initialized()) {
        throw std::runtime_error("Script system not initialized!");
    }
}

} // namespace Test
} // namespace pegr
