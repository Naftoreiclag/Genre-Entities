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

#ifndef PEGR_TEXT_TEXT_HPP
#define PEGR_TEXT_TEXT_HPP

#include <memory>

#include <boost/filesystem.hpp>

#include "pegr/resource/Resources.hpp"

namespace pegr {
namespace Text {
    
struct Text_Res {
    std::string m_string;
};

typedef std::shared_ptr<Text_Res> Text_Res_Ptr;
typedef std::shared_ptr<const Text_Res> Text_Res_Cptr;

Text_Res_Cptr find_text_resource(Resour::Oid oid);

std::string read_file_as_string(boost::filesystem::path file);

} // namespace Text
} // namespace pegr

#endif // PEGR_TEXT_TEXT_HPP
