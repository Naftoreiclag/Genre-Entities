#include "pegr/script/Script.hpp"

namespace pegr {
namespace Test {

//@Test Initialization Sanity Test
bool test_000_init_sanity() {
    return Script::is_initialized();
}

}
}
