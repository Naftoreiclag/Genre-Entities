#include "pegr/gensys/GensysIntermediate.hpp"
#include "pegr/test/TestMacros.hpp"

namespace pegr {
namespace Test {

//@Test Gensys Intermediates
bool gensys_primitive_test() {
    Gensys::Interm::Prim prim;
    
    test_assert(prim.is_error(), "Prim is error");
    test_assert(!prim.is_error(), "Prim is not error");
    
}

}
}
