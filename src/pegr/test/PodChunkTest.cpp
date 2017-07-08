#include "pegr/gensys/PodChunk.hpp"

#include <stdexcept>
#include <sstream>

namespace pegr {
namespace Test {

//@Test PodChunk test
void test_0085_00_podchunk_test() {
    Gensys::Pod_Chunk_Ptr pcp = Gensys::new_pod_chunk(256);
    
    pcp.set_value<int32_t>(0, 12345);
    pcp.set_value<int64_t>(8, 123456789);
    pcp.set_value<float>(16, 3.14159f);
    pcp.set_value<double>(24, 2.71828);
    
    {
        int32_t got = pcp.get_value<int32_t>(0);
        if (got != 12345) {
            std::stringstream sss;
            sss << "Wrong int32 "
                << got;
            throw std::runtime_error(sss.str());
        }
    }
    {
        int64_t got = pcp.get_value<int64_t>(8);
        if (got != 123456789) {
            std::stringstream sss;
            sss << "Wrong int64 "
                << got;
            throw std::runtime_error(sss.str());
        }
    }
    {
        float got = pcp.get_value<float>(16);
        if (got != 3.14159f) {
            std::stringstream sss;
            sss << "Wrong float "
                << got;
            throw std::runtime_error(sss.str());
        }
    }
    {
        double got = pcp.get_value<double>(24);
        if (got != 2.71828) {
            std::stringstream sss;
            sss << "Wrong double "
                << got;
            throw std::runtime_error(sss.str());
        }
    }
    
    Gensys::delete_pod_chunk(pcp);
}

} // namespace Test
} // namespace pegr
