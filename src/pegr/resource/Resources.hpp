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
    
/**
 * @class Oid
 * @brief Object id
 */
class Oid {
public:
    Oid() = default;
    Oid(std::string repr, std::string def_pack = "");
    Oid(const char* core_res);

    const std::string& get_package() const;
    const std::string& get_resource() const;
private:
    std::string m_package;
    std::string m_resource;
};

/**
 * @class Object
 * @brief Single resource file
 */
struct Object {
    enum Type {
        MATERIAL,
        MODEL,
        SHADER_PROGRAM,
        TEXTURE,
        IMAGE,
        GEOMETRY,
        FONT,
        WAVEFORM,
        SHADER,
        SCRIPT,
        STRING,
        
        UNKNOWN
    };
    
    Type m_type;
    boost::filesystem::path m_fname;
};

/**
 * @brief Get an object type from the standard string representation
 * @param str 
 * @return Type
 */
Object::Type object_type_from_string(std::string str);

class Package {
public:
    Package() = default;
    Package(boost::filesystem::path package_file);
    
    std::string get_id() const;
    std::string get_human_name() const;
    std::string get_human_desc() const;
    const Object& find_object(const std::string& id) const;
    const boost::filesystem::path& get_home() const;
    
    std::size_t get_num_resources() const;
    
    /**
     * @brief Becomes a co-owner of the resources stored in the other package.
     * If there are any ID conflicts, the this package's object overwrites the
     * other one's.
     */
    void merge(Package& package);
    
    void make_core();
    
private:
    std::string m_human_name;
    std::string m_human_desc;
    std::string m_id;
    
    boost::filesystem::path m_home;
    
    std::map<std::string, Object> m_name_to_object;
};
    
void initialize();

void cleanup();

const Object& find_object(const Oid& oid);
    
} // namespace Resour
} // namespace pegr

#endif // PEGR_RESOURCE_RESOURCES_HPP
