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

#include "pegr/resource/Resources.hpp"

#include <boost/filesystem.hpp>
#include <json/json.h>

namespace pegr {
namespace Resour {

const boost::filesystem::path& n_core_res_dir = "core";

Package::Package(boost::filesystem::path package_file) {
    m_home = package_file.parent_path();
}

void initialize() {
    if (!boost::filesystem::exists(n_core_res_dir)) {
        boost::filesystem::create_directory(n_core_res_dir);
    }
    boost::filesystem::directory_iterator end_iter;
    for (boost::filesystem::directory_iterator iter(n_core_res_dir); 
            iter != end_iter; ++iter) {
        boost::filesystem::path subdir = *iter;
        
        if (boost::filesystem::is_directory(subdir)) {
            
        }
    }
}

void cleanup() {
    
}

} // namespace Resour
} // namespace pegr
