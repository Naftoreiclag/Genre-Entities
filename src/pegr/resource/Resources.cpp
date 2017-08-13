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

#include "pegr/logger/Logger.hpp"
#include "pegr/resource/Json_Util.hpp"
#include "pegr/except/Except.hpp"

namespace pegr {
namespace Resour {

Oid::Oid(std::string repr, std::string def_pack) {
    std::size_t delim_idx = repr.find(':');
    if (delim_idx == std::string::npos) {
        m_package = def_pack;
        m_resource = repr;
    } else {
        m_package = repr.substr(0, delim_idx);
        m_resource = repr.substr(delim_idx + 1);
    }
}
Oid::Oid(const char* core_res) {
    m_package = "";
    m_resource = core_res;
}

const std::string& Oid::get_package() const {
    return m_package;
}

const std::string& Oid::get_resource() const {
    return m_resource;
}

Package n_core_package;
std::map<std::string, Package> n_named_packages;
const boost::filesystem::path& n_core_res_dir = "core";

const std::map<std::string, Object::Type> n_string_to_type = {
    {"material", Object::Type::MATERIAL},
    {"model", Object::Type::MODEL},
    {"shader-program", Object::Type::SHADER_PROGRAM},
    {"texture", Object::Type::TEXTURE},
    {"image", Object::Type::IMAGE},
    {"geometry", Object::Type::GEOMETRY},
    {"font", Object::Type::FONT},
    {"waveform", Object::Type::WAVEFORM},
    {"shader", Object::Type::SHADER},
    {"script", Object::Type::SCRIPT},
    {"string", Object::Type::STRING}
};

Object::Type object_type_from_string(std::string str) {
    auto iter = n_string_to_type.find(str);
    if (iter == n_string_to_type.end()) {
        return Object::Type::UNKNOWN;
    }
    return iter->second;
}

const char* object_type_to_string(Object::Type ot) {
    switch (ot) {
        case Object::Type::MATERIAL: return "material";
        case Object::Type::MODEL: return "model";
        case Object::Type::SHADER_PROGRAM: return "shader-program";
        case Object::Type::TEXTURE: return "texture";
        case Object::Type::IMAGE: return "image";
        case Object::Type::GEOMETRY: return "geometry";
        case Object::Type::FONT: return "font";
        case Object::Type::WAVEFORM: return "waveform";
        case Object::Type::SHADER: return "shader";
        case Object::Type::SCRIPT: return "script";
        case Object::Type::STRING: return "string";
        default: return "unknown";
    }
}

Package::Package(boost::filesystem::path package_file) {
    m_home = package_file.parent_path();
    
    const Json::Value package_json = Json_Util::read(package_file);
    const Json::Value& userdata_json = package_json["userdata"];
    const Json::Value& info_json = userdata_json["info"];
    
    m_id = Json_Util::as_string(info_json["id"]);
    m_human_name = Json_Util::as_string(info_json["name"], m_id.c_str());
    m_human_desc = Json_Util::as_string(info_json["desc"], "no description");
    
    const Json::Value& resources_json = package_json["resources"];
    
    for (Json::ValueConstIterator iter = resources_json.begin(); 
            iter != resources_json.end(); ++iter) {
        const Json::Value& obj_id_json = iter.key();
        const Json::Value& obj_json = *iter;
        
        std::string id = Json_Util::as_string(obj_id_json);
        
        Object obj;
        obj.m_fname = Json_Util::as_string(obj_json["file"]);
        obj.m_fname = m_home / obj.m_fname;
        obj.m_type = object_type_from_string(
                Json_Util::as_string(obj_json["type"]));
        m_name_to_object.emplace(std::move(id), std::move(obj));
    }
}

std::string Package::get_id() const {
    return m_id;
}
std::string Package::get_human_name() const {
    return m_human_name;
}
std::string Package::get_human_desc() const {
    return m_human_desc;
}
const Object& Package::find_object(const std::string& name) const {
    auto iter = m_name_to_object.find(name);
    if (iter == m_name_to_object.end()) {
        std::stringstream sss;
        if (m_id == "") {
            sss << "Cannot find core resource, [";
        } else {
            sss << "Cannot find resource, ["
                << m_id
                << ':';
        }
        sss << name
            << "]";
        throw Except::Runtime(sss.str());
    }
    const Object& obj = iter->second;
    return obj;
}
const boost::filesystem::path& Package::get_home() const {
    return m_home;
}
std::size_t Package::get_num_resources() const {
    return m_name_to_object.size();
}

void Package::merge(Package& opack) {
    m_name_to_object.insert(opack.m_name_to_object.begin(), 
            opack.m_name_to_object.end());
}

void Package::make_core() {
    m_id = "";
}

struct Load_Workspace {
    std::vector<std::string> m_core_load_order;
    std::map<std::string, Package> m_core_packages;
    Package m_agg_package;
};

void read_core_load_order(Load_Workspace& wl) {
    boost::filesystem::path loado_file = n_core_res_dir / "load_order.json";
    const Json::Value loado_json = Json_Util::read_commentated(loado_file);
    
    for (const Json::Value& id_json : loado_json) {
        wl.m_core_load_order.push_back(Json_Util::as_string(id_json));
    }
    Logger::log()->info("Core package load order:");
    for (std::string pack_id : wl.m_core_load_order) {
        Logger::log()->info("\t[%v]", pack_id);
    }
}

void read_core_packages(Load_Workspace& wl) {
    if (!boost::filesystem::exists(n_core_res_dir)) {
        boost::filesystem::create_directory(n_core_res_dir);
    }
    boost::filesystem::directory_iterator end_iter;
    for (boost::filesystem::directory_iterator iter(n_core_res_dir); 
            iter != end_iter; ++iter) {
        boost::filesystem::path subdir = *iter;
        
        if (boost::filesystem::is_directory(subdir)) {
            boost::filesystem::path package_file = subdir / "data.package";
            if (!boost::filesystem::exists(package_file)) {
                continue;
            }
            try {
                Package package(package_file);
                std::string id = package.get_id(); // Copy
                
                auto dupe_check_iter = wl.m_core_packages.find(id);
                if (dupe_check_iter != wl.m_core_packages.end()) {
                    Package& opack = dupe_check_iter->second;
                    std::stringstream sss;
                    sss << "Two core packages have the same id, \""
                        << id
                        << "\": "
                        << opack.get_home()
                        << " and "
                        << subdir;
                    throw Except::Runtime(sss.str());
                }
                
                wl.m_core_packages.emplace(id, std::move(package));
            } catch (Except::Runtime& e) {
                Logger::log()->warn("Package in %v is corrupted: %v", 
                        subdir, e.what());
            }
        }
    }
}

void collapse_core_packages(Load_Workspace& wl) {
    Logger::log()->info("Collapsing core packages...");
    for (std::vector<std::string>::reverse_iterator iter = 
            wl.m_core_load_order.rbegin(); iter != wl.m_core_load_order.rend();
            ++iter) {
        const std::string& pack_id = *iter;
        auto pack_iter = wl.m_core_packages.find(pack_id);
        if (pack_iter == wl.m_core_packages.end()) {
            std::stringstream sss;
            sss << "Package \""
                << pack_id
                << "\" specified in load order config, but was not found";
            throw Except::Runtime(sss.str());
        }
        
        Package& pack = pack_iter->second;
        std::size_t orig_size = n_core_package.get_num_resources();
        n_core_package.merge(pack);
        std::size_t merged_count = 
                n_core_package.get_num_resources() - orig_size;
        std::size_t overwr_count = 
                pack.get_num_resources() - merged_count;
        
        Logger::log()->info("\tMerged %v and overwrote %v from [%v]",
                merged_count, overwr_count, pack.get_id());
    }
    n_core_package.make_core();
}

void load_core() {
    Load_Workspace lw;
    try {
        read_core_load_order(lw);
    } catch (Except::Runtime& e) {
        std::stringstream sss;
        sss << "Error while reading core load order: "
            << e.what();
        throw Except::Runtime(sss.str());
    }
    try {
        read_core_packages(lw);
    } catch (Except::Runtime& e) {
        std::stringstream sss;
        sss << "Error while loading core packages: "
            << e.what();
        throw Except::Runtime(sss.str());
    }
    try {
        collapse_core_packages(lw);
    } catch (Except::Runtime& e) {
        std::stringstream sss;
        sss << "Error while collapsing core packages: "
            << e.what();
        throw Except::Runtime(sss.str());
    }
    
    Logger::log()->info("Core resources loaded successfully");
    Logger::log()->info("\t%v core packages", lw.m_core_packages.size());
    Logger::log()->info("\t%v core resources", 
            n_core_package.get_num_resources());
}

void initialize() {
    load_core();
}

void cleanup() {
    n_named_packages.clear();
    n_core_package = Package();
}

const Object& find_object(const Oid& oid, Object::Type required_type) {
    if (oid.get_package() == "") {
        return n_core_package.find_object(oid.get_resource());
    }
    const auto iter = n_named_packages.find(oid.get_package());
    if (iter == n_named_packages.end()) {
        std::stringstream sss;
        sss << "Cannot find package for resource, ["
            << oid.get_package()
            << ':'
            << oid.get_resource()
            << ']';
        throw Except::Runtime(sss.str());
    }
    const Package& pack = iter->second;
    
    const Object& obj = pack.find_object(oid.get_resource());
    
    if (required_type != Object::Type::UNKNOWN && obj.m_type != required_type) {
        std::stringstream sss;
        sss << "Resource with id ["
            << oid.get_package()
            << ':'
            << oid.get_resource()
            << "] is type "
            << object_type_to_string(obj.m_type)
            << ", but expected type "
            << object_type_to_string(required_type);
        throw Except::Runtime(sss.str());
    }
    
    return obj;
}

} // namespace Resour
} // namespace pegr
