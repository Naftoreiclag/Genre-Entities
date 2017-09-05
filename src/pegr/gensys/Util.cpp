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

#include "pegr/gensys/Util.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

#include "pegr/algs/Partition_Tracker.hpp"

namespace pegr {
namespace Gensys {
namespace Util {

Pod::Chunk_Ptr new_pod_chunk_from_interm_prims(
        const std::map<Interm::Symbol, Interm::Prim>& members, 
        std::map<Interm::Symbol, std::size_t>& symbol_to_offset) {
    symbol_to_offset.clear();
    std::map<std::size_t, std::vector<Interm::Symbol> > symbols_by_size;
    
    for (const auto& member : members) {
        const Interm::Symbol& symb = member.first;
        const Interm::Prim& val = member.second;
        
        switch (val.get_type()) {
            case Interm::Prim::Type::I32: {
                symbols_by_size[sizeof(int32_t)].push_back(symb);
                break;
            }
            case Interm::Prim::Type::I64: {
                symbols_by_size[sizeof(int64_t)].push_back(symb);
                break;
            }
            case Interm::Prim::Type::F32: {
                symbols_by_size[sizeof(float)].push_back(symb);
                break;
            }
            case Interm::Prim::Type::F64: {
                symbols_by_size[sizeof(double)].push_back(symb);
                break;
            }
            default: {
                break;
            }
        }
    }
    
    // Largest objects at the front of the list
    std::vector<std::size_t> sorted_sizes;
    for (const auto& member : symbols_by_size) {
        sorted_sizes.push_back(member.first);
    }
    std::sort(sorted_sizes.begin(), sorted_sizes.end(), 
            std::greater<std::size_t>());
    
    Algs::Partition_Tracker ptrack;
    for (std::size_t size : sorted_sizes) {
        const auto& equal_sized_members = symbols_by_size[size];
        for (auto member_symbol : equal_sized_members) {
            
            std::size_t alignment_interval;
            if (size <= 1) {
                alignment_interval = 1;
            } else if (size <= 2) {
                alignment_interval = 2;
            } else if (size <= 4) {
                alignment_interval = 4;
            } else {
                alignment_interval = 8;
            }
            
            std::size_t off = 0;
            while (!ptrack.can_occupy(off, size)) off += alignment_interval;
            
            ptrack.occupy(off, size);
            symbol_to_offset[member_symbol] = off;
        }
    }
    
    Pod::Chunk_Ptr pcp = Pod::new_pod_chunk(ptrack.get_minimum_size());
    copy_named_prims_into_pod_chunk(members, symbol_to_offset, pcp, 0);
    return pcp;
}

void copy_named_prims_into_pod_chunk(
        const std::map<Interm::Symbol, Interm::Prim>& members, 
        const std::map<Interm::Symbol, std::size_t>& symbol_to_offset, 
        Pod::Chunk_Ptr pcp, std::size_t dest_offset) {
    for (const auto& symbol_offset_pair : symbol_to_offset) {
        const Interm::Symbol& symb = symbol_offset_pair.first;
        
        const auto& member_iter = members.find(symb);
        
        /* Skip any members that are not specified
         * (such as when an archetype sets new defaults)
         */
        if (member_iter == members.end()) {
            continue;
        }
        
        const Interm::Prim& val = member_iter->second;
        
        // Skip any empty values (such as when an archetype sets new defaults)
        if (val.is_empty()) {
            continue;
        }
        
        std::size_t offset = dest_offset + symbol_offset_pair.second;
        
        switch (val.get_type()) {
            case Interm::Prim::Type::I32: {
                pcp.set_value<int32_t>(offset, val.get_i32());
                break;
            }
            case Interm::Prim::Type::I64: {
                pcp.set_value<int64_t>(offset, val.get_i64());
                break;
            }
            case Interm::Prim::Type::F32: {
                pcp.set_value<float>(offset, val.get_f32());
                break;
            }
            case Interm::Prim::Type::F64: {
                pcp.set_value<double>(offset, val.get_f64());
                break;
            }
            default: {
                break;
            }
        }
    }
}
    
    
} // namespace Util
} // namespace Gensys
} // namespace pegr

