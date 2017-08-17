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

#include <map>
#include <string>

#include <boost/filesystem.hpp>

namespace pegr {
namespace Resour {
    
/**
 * @class Oid
 * @brief Object id. Consists of a package to draw from, and the internal name
 * of the resource.
 */
class Oid {
public:
    Oid() = default;
    Oid(std::string repr, std::string def_pack = "");
    Oid(const char* core_res);

    const std::string& get_package() const;
    const std::string& get_resource() const;
    
    Oid get_subtype(std::string subtype) const;
    
    std::string get_dbg_string() const;
    
    bool operator <(const Oid& rhs) const;
    
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

/**
 * @brief Inverse of object_type_from_string
 * @param ot
 * @return String
 */
const char* object_type_to_string(Object::Type ot);

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

/**
 * @brief Attempts to find an object with the given object id. If the required
 * type is specified, then an error is thrown if the object is of a different
 * type.
 * @param oid The object id to use when looking for the resource
 * @param required_type Optional, specifies a required returned object type
 * @return The object/resource
 */
const Object& find_object(const Oid& oid,
        Object::Type required_type = Object::Type::UNKNOWN);
    
} // namespace Resour
} // namespace pegr

#endif // PEGR_RESOURCE_RESOURCES_HPP
