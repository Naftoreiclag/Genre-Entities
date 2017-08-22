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

#include "pegr/resource/Oid.hpp"

#include "pegr/except/Except.hpp"

#include <cassert>
#include <sstream>

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
        if (std::string(m_resource).find(':') != std::string::npos) {
            std::stringstream sss;
            sss << "Too many colons (\":\") in OID: \""
                << repr
                << '"';
            throw Except::Runtime(sss.str());
        }
    }
}
Oid::Oid(const char* core_res) {
    assert(std::string(core_res).find(':') == std::string::npos);
    m_package = "";
    m_resource = core_res;
}

Oid Oid::get_subtype(std::string subtype) const {
    Oid retval;
    retval.m_package = m_package;
    retval.m_resource.reserve(1 + m_resource.size() + subtype.size());
    retval.m_resource.append(m_resource);
    retval.m_resource.append(1, '#');
    retval.m_resource.append(subtype);
    return retval;
}

const std::string& Oid::get_package() const {
    return m_package;
}

const std::string& Oid::get_resource() const {
    return m_resource;
}

std::string Oid::get_dbg_string() const {
    std::string retval;
    retval.reserve(3 + m_package.size() + m_resource.size());
    retval.append(1, '[');
    retval.append(m_package);
    retval.append(1, ':');
    retval.append(m_resource);
    retval.append(1, ']');
    return retval;
}
std::string Oid::get_repr() const {
    std::string retval;
    retval.reserve(1 + m_package.size() + m_resource.size());
    retval.append(m_package);
    retval.append(1, ':');
    retval.append(m_resource);
    return retval;
}
    
bool Oid::operator <(const Oid& rhs) const {
    return m_package < rhs.m_package || 
            (m_package == rhs.m_package && m_resource < rhs.m_resource);
}

std::ostream& operator <<(std::ostream& sss, const Oid& oid) {
    sss << oid.get_dbg_string();
    return sss;
}

} // namespace Resour
} // namespace pegr
