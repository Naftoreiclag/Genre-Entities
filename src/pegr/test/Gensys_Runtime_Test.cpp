#include "pegr/gensys/Gensys.hpp"

namespace pegr {
namespace Test {

//@Test Gensys Runtime Test
void test_0099_gensys_runtime() {
    Gensys::cleanup();
    Gensys::initialize();
}

} // namespace Test
} // namespace pegr
