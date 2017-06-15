#include "pegr/script/Script.hpp"

namespace pegr {
namespace Test {

//@Test 000 Initialization Sanity Test
bool init_sanity_test() {
    return Script::is_initialized();
}

}
}
