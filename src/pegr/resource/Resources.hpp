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

#ifndef PEGR_RESOURCE_RESOURCES_HPP
#define PEGR_RESOURCE_RESOURCES_HPP

#include <string>
#include <map>

#include <boost/filesystem.hpp>

namespace pegr {
namespace Resour {

class Package {
public:
    Package(boost::filesystem::path package_file);
    
    std::string get_id();
    std::string get_human_name();
    std::string get_human_desc();
    boost::filesystem::path get_file(std::string id);
    
private:
    std::string m_human_name;
    std::string m_human_desc;
    std::string m_id;
    
    boost::filesystem::path m_home;
    
    // Paths are relative to the package home
    std::map<std::string, boost::filesystem::path> m_name_to_file;
    
    friend void initialize();
};
    
void initialize();

void cleanup();
    
} // namespace Resour
} // namespace pegr

#endif // PEGR_RESOURCE_RESOURCES_HPP
