/*
 *  Copyright 2017 James Fong
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "pegr/text/Text.hpp"

#include <fstream>
#include <sstream>

#include "pegr/except/Except.hpp"

namespace pegr {
namespace Text {

Text_Res_Cptr find_text_resource(Resour::Oid oid) {
    const Resour::Object& obj = Resour::find_object(oid, 
            Resour::Object::Type::STRING);
    
    Text_Res_Ptr text_res = std::make_shared<Text_Res>();
    text_res->m_string = read_file_as_string(obj.m_fname);
    
    return text_res;
}

std::string read_file_as_string(boost::filesystem::path file) {
    std::ifstream is(file.string().c_str(), std::ios::in | std::ios::binary);
    if (!is) {
        throw Except::Runtime("Failed to open file");
    }
    
    
    // Reserve enough space ahead of time
    is.seekg(0, std::ios::end);
    std::string retval(is.tellg(), '\0');
    assert(retval.size() == is.tellg());
    
    // Read everything
    is.seekg(0, std::ios::beg);
    is.read(&retval[0], retval.size());
    return retval;
}

} // namespace Text
} // namespace pegr
