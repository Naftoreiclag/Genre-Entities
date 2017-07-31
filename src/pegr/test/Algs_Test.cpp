#include "pegr/util/Algs.hpp"
#include "pegr/test/Test_Util.hpp"

namespace pegr {
namespace Test {

//@Test Util algs test
void test_0000_algs() {
    {
        std::map<int, int> first;
        for (int i = 1; i <= 10; ++i) {
            first[i] = i * i;
        }
        
        std::map<int, int> second;
        for (int i = 1; i <= 10; ++i) {
            second[i * 2] = i / 2;
        }
        
        std::vector<int> intersect = 
                Util::map_key_intersect(first, second);
        
        std::vector<int> expected = {2, 4, 6, 8, 10};
        
        verify_equals(expected, intersect);
    }
    
    {
        std::map<std::string, const char*> first;
        first["a"] = "apple";
        first["b"] = "banana";
        first["c"] = "cherry";
        first["d"] = "doughnut";
        
        std::map<std::string, std::string> second;
        second["b"] = "boron";
        second["c"] = "carbon";
        second["f"] = "fluorine";
        second["h"] = "hydrogen";
        
        std::vector<std::string> intersect = 
                Util::map_key_intersect(first, second);
        
        std::vector<std::string> expected = {"b", "c"};
        
        verify_equals(expected, intersect);
    }
}

} // namespace Test
} // namespace pegr
