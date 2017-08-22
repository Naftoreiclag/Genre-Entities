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

#ifndef PEGR_RESOURCE_OID_HPP
#define PEGR_RESOURCE_OID_HPP

#include <string>

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
    
    std::string get_repr() const;
    
    bool operator <(const Oid& rhs) const;
    
private:
    std::string m_package;
    std::string m_resource;
};

std::ostream& operator <<(std::ostream& sss, const Oid& oid);

} // namespace Resour
} // namespace pegr

#endif // PEGR_RESOURCE_OID_HPP
