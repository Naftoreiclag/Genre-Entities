#include <sstream>
#include <cstdint>

#include "pegr/logger/Logger.hpp"

namespace pegr {
namespace Test {

//@Test Memory Test
void test_0000_memory_test() {
    //                 00000000001111111111222222
    //                 01234567890123456789012345
    const char* str = "abcdefghijklmnopqrstuvwxyz";
    
    std::size_t bee = 1;
    std::size_t sea = 2;
    std::size_t eye = 8;
    std::size_t jay = 9;
    std::size_t pea = 15;
    std::size_t queue = 16;
    std::size_t are = 17;
    std::size_t tea = 19;
    std::size_t you = 20;
    std::size_t why = 24;
    
    std::stringstream sss;
    sss << *str << *(str + bee) << *(str + sea) << *(str + eye) << *(str + jay)
        << *(str + pea) << *(str + queue) << *(str + are) << *(str + tea) 
        << *(str + you) << *(str + why);
    
    std::string expected = "abcijpqrtuy";
    std::string got = sss.str();
    
    Logger::log()->info(got);
    if (expected != got) {
        std::stringstream ers;
        ers << "Expected: " << expected;
        throw std::runtime_error(ers.str());
    }
}

} // namespace Test
} // namespace pegr
