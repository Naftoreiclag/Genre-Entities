/*
 *  Copyright 2015-2017 James Fong
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

#ifndef PEGR_RESOURCE_JSONUTIL_HPP
#define PEGR_RESOURCE_JSONUTIL_HPP

#include <string>

#include <boost/filesystem.hpp>
#include <json/json.h>

namespace pegr {
namespace Resour {
namespace Json_Util {

Json::Value read_commentated(boost::filesystem::path file);
Json::Value read(boost::filesystem::path file);
void write(boost::filesystem::path filename, const Json::Value& value);

std::string as_string(const Json::Value& val, const char* def = nullptr);

} // namespace Json_Util
} // namespace Resour
} // namespace pegr

#endif // PEGR_RESOURCE_JSONUTIL_HPP
