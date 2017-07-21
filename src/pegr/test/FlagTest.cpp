#include "pegr/gensys/Runtime_Types.hpp"
#include "pegr/test/TestUtil.hpp"

namespace pegr {
namespace Test {
    
void verify_bitset(uint64_t bitset, uint64_t flags, bool status) {
    if (status) {
        verify_equals(flags, bitset & flags);
    } else {
        verify_equals(0, bitset & flags);
    }
}

//@Test Flag test
void test_0001_flags() {
    uint64_t flags = 0;
    
    verify_equals(0, flags);
    
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_SPAWNED, false);
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_KILLED, false);
    
    flags |= Gensys::Runtime::ENT_FLAG_KILLED;
    
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_SPAWNED, false);
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_KILLED, true);
    
    flags |= Gensys::Runtime::ENT_FLAG_SPAWNED;
    
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_SPAWNED, true);
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_KILLED, true);
    
    flags &= ~Gensys::Runtime::ENT_FLAG_SPAWNED;
    
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_SPAWNED, false);
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_KILLED, true);
    
    flags &= ~Gensys::Runtime::ENT_FLAG_KILLED;
    
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_SPAWNED, false);
    verify_bitset(flags, Gensys::Runtime::ENT_FLAG_KILLED, false);
}

} // namespace Test
} // namespace pegr
