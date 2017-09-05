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

#include "pegr/gensys/Compiler.hpp"

#include <algorithm>
#include <cassert>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include "pegr/Script/Script_Util.hpp"
#include "pegr/gensys/Gensys.hpp"
#include "pegr/gensys/Util.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/resource/Oid.hpp"

namespace pegr {
namespace Gensys {

// Forward delcarations that let us access the runtime maps
namespace Runtime {
extern std::map<Resour::Oid, std::unique_ptr<Runtime::Comp> > n_runtime_comps;
extern std::map<Resour::Oid, std::unique_ptr<Runtime::Arche> > n_runtime_arches;
extern std::map<Resour::Oid, std::unique_ptr<Runtime::Genre> > n_runtime_genres;
extern std::vector<Script::Unique_Regref> n_held_lua_values;
} // namespace Runtime
    
namespace Compiler {

namespace Work {

struct Comp {
    Comp(std::unique_ptr<Interm::Comp>&& interm)
    : m_interm(std::move(interm)) {
        m_runtime = std::make_unique<Runtime::Comp>();
    }
    
    std::unique_ptr<Interm::Comp> m_interm;
    std::unique_ptr<Runtime::Comp> m_runtime;
    std::map<Interm::Symbol, std::size_t> m_symbol_to_offset;
    Algs::Unique_Chunk_Ptr m_compiled_chunk;
    std::vector<std::string> m_strings;
    std::vector<Script::Regref> m_funcs;
};
struct Arche {
    Arche(std::unique_ptr<Interm::Arche>&& interm)
    : m_interm(std::move(interm)) {
        m_runtime = std::make_unique<Runtime::Arche>();
    }
    
    std::unique_ptr<Interm::Arche> m_interm;
    std::unique_ptr<Runtime::Arche> m_runtime;
};
struct Genre {
    Genre(std::unique_ptr<Interm::Genre>&& interm)
    : m_interm(std::move(interm)) {
        m_runtime = std::make_unique<Runtime::Genre>();
    }
    
    std::unique_ptr<Interm::Genre> m_interm;
    std::unique_ptr<Runtime::Genre> m_runtime;
};

/**
 * @class Space
 * @brief Where all the unique objects are stored during the compilation
 * process. Exists only during a call to compile()
 * 
 * Ah, get it? Work::Space??
 */
class Space {
private:
    std::vector<std::unique_ptr<Comp> > m_comps;
    std::vector<std::unique_ptr<Arche> > m_arches;
    std::vector<std::unique_ptr<Genre> > m_genres;

    std::map<const Interm::Comp*, Comp*> m_comps_by_interm;
    std::map<const Interm::Arche*, Arche*> m_arches_by_interm;
    std::map<const Interm::Genre*, Genre*> m_genres_by_interm;

    std::map<Resour::Oid, Comp*> m_comps_by_id;
    std::map<Resour::Oid, Arche*> m_arches_by_id;
    std::map<Resour::Oid, Genre*> m_genres_by_id;
    
    Script::Util::Unique_Regref_Manager m_unique_regrefs;
    
public:
    Script::Regref add_lua_value(Script::Regref val_ref) {
        m_unique_regrefs.add_lua_value(val_ref);
    }
    
    const std::vector<Script::Unique_Regref>& get_lua_uniques() const {
        return m_unique_regrefs.get_lua_uniques();
    }
    
    std::vector<Script::Unique_Regref> release_lua_uniques() {
        return m_unique_regrefs.release();
    }
    
    const std::vector<std::unique_ptr<Comp> >& get_comps() const {
        return m_comps;
    }
    const std::vector<std::unique_ptr<Arche> >& get_arches() const {
        return m_arches;
    }
    const std::vector<std::unique_ptr<Genre> >& get_genres() const {
        return m_genres;
    }
    
    const std::map<const Interm::Comp*, Comp*>& get_comps_by_interm() const {
        return m_comps_by_interm;
    }
    const std::map<const Interm::Arche*, Arche*>& get_arches_by_interm() const {
        return m_arches_by_interm;
    }
    const std::map<const Interm::Genre*, Genre*>& get_genres_by_interm() const {
        return m_genres_by_interm;
    }
    
    const std::map<Resour::Oid, Comp*>& get_comps_by_id() const {
        return m_comps_by_id;
    }
    const std::map<Resour::Oid, Arche*>& get_arches_by_id() const {
        return m_arches_by_id;
    }
    const std::map<Resour::Oid, Genre*>& get_genres_by_id() const {
        return m_genres_by_id;
    }
    
    void add_comp(std::unique_ptr<Comp>&& obj, Resour::Oid id) {
        m_comps_by_interm[obj->m_interm.get()] = obj.get();
        m_comps_by_id[id] = obj.get();
        m_comps.emplace_back(std::move(obj));
    }
    
    void add_arche(std::unique_ptr<Arche>&& obj, Resour::Oid id) {
        m_arches_by_interm[obj->m_interm.get()] = obj.get();
        m_arches_by_id[id] = obj.get();
        m_arches.emplace_back(std::move(obj));
    }
    
    void add_genre(std::unique_ptr<Genre>&& obj, Resour::Oid id) {
        m_genres_by_interm[obj->m_interm.get()] = obj.get();
        m_genres_by_id[id] = obj.get();
        m_genres.emplace_back(std::move(obj));
    }
    
    void clear() {
        m_comps.clear();
        m_arches.clear();
        m_genres.clear();
        m_comps_by_interm.clear();
        m_arches_by_interm.clear();
        m_genres_by_interm.clear();
        m_comps_by_id.clear();
        m_arches_by_id.clear();
        m_genres_by_id.clear();
    }
};

} // namespace Work

std::map<Resour::Oid, std::unique_ptr<Interm::Comp> > n_staged_comps;
std::map<Resour::Oid, std::unique_ptr<Interm::Arche> > n_staged_arches;
std::map<Resour::Oid, std::unique_ptr<Interm::Genre> > n_staged_genres;

void initialize() {
    assert(get_global_state() == GlobalState::UNINITIALIZED);
}

void cleanup() {
    assert(get_global_state() != GlobalState::UNINITIALIZED);
    n_staged_comps.clear();
    n_staged_arches.clear();
    n_staged_genres.clear();
}

Runtime::Prim::Type prim_type_convert(Interm::Prim::Type it) {
    switch (it) {
        case Interm::Prim::Type::I32: return Runtime::Prim::Type::I32;
        case Interm::Prim::Type::I64: return Runtime::Prim::Type::I64;
        case Interm::Prim::Type::F32: return Runtime::Prim::Type::F32;
        case Interm::Prim::Type::F64: return Runtime::Prim::Type::F64;
        case Interm::Prim::Type::FUNC: return Runtime::Prim::Type::FUNC;
        case Interm::Prim::Type::STR: return Runtime::Prim::Type::STR;
        default: {
            assert(false);
            break;
        }
    }
}

void compile_component_store_pod(Work::Space& workspace, 
        std::unique_ptr<Work::Comp>& comp) {
    comp->m_compiled_chunk.reset(
            Util::new_pod_chunk_from_interm_prims(
                    comp->m_interm->m_members,
                    comp->m_symbol_to_offset));
}

void compile_component_store_non_pod(Work::Space& workspace, 
        std::unique_ptr<Work::Comp>& comp) {
    comp->m_strings.clear();
    for (const auto& member : comp->m_interm->m_members) {
        //
        const Interm::Symbol& symbol = member.first;
        const Interm::Prim& prim = member.second;
        
        switch (prim.get_type()) {
            case Interm::Prim::Type::STR: {
                comp->m_symbol_to_offset[symbol] = comp->m_strings.size();
                comp->m_strings.push_back(prim.get_string());
                break;
            }
            case Interm::Prim::Type::FUNC: {
                comp->m_symbol_to_offset[symbol] = comp->m_funcs.size();
                comp->m_funcs.push_back(prim.get_function()->get());
                break;
            }
            default: {
                continue;
            }
        }
    }
}
 
void compile_component_record_offsets(Work::Space& workspace, 
        std::unique_ptr<Work::Comp>& comp) {
    // Record the member offsets in the runtime data
    for (const auto& sto_entry : comp->m_symbol_to_offset) {
        // Get the symbol and offset
        const Interm::Symbol& symbol = sto_entry.first;
        std::size_t offset = sto_entry.second;
        
        // Get the type of the primitive and the associated offset
        const auto& member_entry = comp->m_interm->m_members.find(symbol);
        assert(member_entry != comp->m_interm->m_members.end());
        Interm::Prim::Type prim_type = member_entry->second.get_type();
        
        // Runtime primitive setup
        Runtime::Prim runtime_prim;
        runtime_prim.m_type = prim_type_convert(prim_type);
        switch (runtime_prim.m_type) {
            case Runtime::Prim::Type::I32:
            case Runtime::Prim::Type::I64:
            case Runtime::Prim::Type::F32:
            case Runtime::Prim::Type::F64: {
                runtime_prim.m_refer.m_byte_offset = offset;
                break;
            }
            case Runtime::Prim::Type::STR:
            case Runtime::Prim::Type::FUNC: {
                runtime_prim.m_refer.m_index = offset;
                break;
            }
            default: {
                assert(false && "Unhandled runtime prim type during compile");
                break;
            }
        }
        
        // Store in runtime data
        comp->m_runtime->m_member_offsets[symbol] = std::move(runtime_prim);
    }
}

std::unique_ptr<Work::Comp> compile_component(Work::Space& workspace, 
        std::unique_ptr<Interm::Comp>&& interm) {
    // Make the working component
    std::unique_ptr<Work::Comp> comp = 
            std::make_unique<Work::Comp>(std::move(interm));
    
    // Pack data and record where each member was placed
    compile_component_store_pod(workspace, comp);
    compile_component_store_non_pod(workspace, comp);
    
    // Record the member offsets in the runtime data
    compile_component_record_offsets(workspace, comp);
    
    return comp;
}

/**
 * @brief Constructs a new pod chunk for the archetype, exactly sizing it to
 * be able to fit all of its components' pod chunks.
 */
void compile_archetype_resize_pod(Work::Space& workspace,
        std::unique_ptr<Work::Arche>& arche) {
    // Find the total size, which is the sum of the component POD chunk
    std::size_t total_size = 0;
    for (const auto& implem_pair : arche->m_interm->m_implements) {
        const Interm::Arche::Implement& implem = implem_pair.second;
        const auto& comp_iter = 
                workspace.get_comps_by_interm().find(implem.m_component);
        assert(comp_iter != workspace.get_comps_by_interm().end());
        const Work::Comp* comp = comp_iter->second;
        total_size += comp->m_compiled_chunk.get().get_size();
    }
    arche->m_runtime->m_default_chunk.reset(Algs::Podc_Ptr::new_podc(total_size));
}

/**
 * @brief Fill the pod chunk made earlier with the components' pod chunks.
 */
void compile_archetype_fill_pod(Work::Space& workspace,
        std::unique_ptr<Work::Arche>& arche) {

    // Stores how many bytes have already been used up in the POD chunk
    std::size_t accumulated = 0;
    
    // For every implementation
    for (const auto& implem_pair : arche->m_interm->m_implements) {
        // Get the implementation
        const Interm::Arche::Implement& implem = implem_pair.second;
        Logger::log()->info("    %v", implem.m_error_msg_name);

        const auto& comp_iter = 
                workspace.get_comps_by_interm().find(implem.m_component);
        
        assert(comp_iter != workspace.get_comps_by_interm().end());
        
        const Work::Comp* comp = comp_iter->second;

        // Copy the pod chunk
        Algs::Podc_Ptr::copy_podc(
                comp->m_compiled_chunk.get(),
                0,
                arche->m_runtime->m_default_chunk.get(),
                accumulated,
                comp->m_compiled_chunk.get().get_size());

        // Set new defaults by overwriting existing component chunk data
        Util::copy_named_prims_into_pod_chunk(
                implem.m_values,
                comp->m_symbol_to_offset,
                arche->m_runtime->m_default_chunk.get(),
                accumulated);
        
        // Also record the symbol for this implementation
        const Runtime::Symbol symb = implem_pair.first;
        Runtime::Comp* run_comp = comp->m_runtime.get();
        arche->m_runtime->m_components[symb] = run_comp;
        
        // Remember how to find this data later
        arche->m_runtime->m_comp_offsets[run_comp].m_pod_idx = accumulated;

        // Keep track of how much space has been used
        accumulated += comp->m_compiled_chunk.get().get_size();
    }

    // This should have exactly filled the POD chunk (guaranteed earlier)
    assert(accumulated == arche->m_runtime->m_default_chunk.get().get_size());
}

/**
 * @brief Copy strings from the component primitives, overwrite with new 
 * defaults.
 */
void compile_archetype_store_strings(Work::Space& workspace,
        std::unique_ptr<Work::Arche>& arche) {

    // Stores the running index for strings in the vector
    std::size_t accumulated = 0;

    // For every implementation
    for (const auto& implem_pair : arche->m_interm->m_implements) {
        
        // Get the implementation
        const Interm::Arche::Implement& implem = implem_pair.second;

        const auto& comp_iter = 
                workspace.get_comps_by_interm().find(implem.m_component);
        
        assert(comp_iter != workspace.get_comps_by_interm().end());
        
        // This should be const, since we shouldn't modify anything else
        const Work::Comp* comp = comp_iter->second;
        
        // Copy the default strings
        std::copy(comp->m_strings.begin(), comp->m_strings.end(), 
                std::back_inserter(arche->m_runtime->m_default_strings));
        
        // Set new defaults by overwriting existing strings
        for (const auto& member : implem.m_values) {
            //
            const Interm::Symbol& symbol = member.first;
            const Interm::Prim& prim = member.second;
            
            if (prim.get_type() != Interm::Prim::Type::STR) {
                continue;
            }
            
            const auto& iter = comp->m_symbol_to_offset.find(symbol);
            
            assert(iter != comp->m_symbol_to_offset.end());
            
            std::size_t offset = iter->second;
            
            arche->m_runtime->m_default_strings[accumulated + offset]
                    = prim.get_string();
        }

        // Remember how to find this data later
        arche->m_runtime->m_comp_offsets[comp->m_runtime.get()].m_string_idx 
                = accumulated;

        // Keep track of how much space has been used
        accumulated += comp->m_strings.size();
    }

    // Every string should have been copied
    assert(accumulated == arche->m_runtime->m_default_strings.size());
}

/**
 * @brief Store any static lua values into the global Gensys Lua value table
 * @param workspace
 * @param arche
 */
void compile_archetype_store_static_lua_values(Work::Space& workspace,
        std::unique_ptr<Work::Arche>& arche) {
    
    std::size_t accumulated = 0;
            
    // For every implementation
    for (const auto& implem_pair : arche->m_interm->m_implements) {
        
        // Get the implementation
        const Interm::Arche::Implement& implem = implem_pair.second;

        const auto& comp_iter = 
                workspace.get_comps_by_interm().find(implem.m_component);
        
        assert(comp_iter != workspace.get_comps_by_interm().end());
        
        // This should be const, since we shouldn't modify anything else
        const Work::Comp* comp = comp_iter->second;
        
        // Copy the defaults
        std::copy(comp->m_funcs.begin(), comp->m_funcs.end(), 
                std::back_inserter(arche->m_runtime->m_static_funcs));
        
        // Set new defaults by overwriting existing funcs
        for (const auto& member : implem.m_values) {
            //
            const Interm::Symbol& symbol = member.first;
            const Interm::Prim& prim = member.second;
            
            if (prim.get_type() != Interm::Prim::Type::FUNC) {
                continue;
            }
            
            const auto& iter = comp->m_symbol_to_offset.find(symbol);
            
            assert(iter != comp->m_symbol_to_offset.end());
            
            std::size_t offset = iter->second;
            
            arche->m_runtime->m_static_funcs[accumulated + offset]
                    = prim.get_function()->get();
        }

        // Remember how to find this data later
        arche->m_runtime->m_comp_offsets[comp->m_runtime.get()].m_func_idx 
                = accumulated;

        // Keep track of how much space has been used
        accumulated += comp->m_funcs.size();
    }

    // Every function should have been collected
    assert(accumulated == arche->m_runtime->m_static_funcs.size());
    
    // Upload every saved function
    for (std::size_t idx = 0; 
            idx < arche->m_runtime->m_static_funcs.size(); ++idx) {
        arche->m_runtime->m_static_funcs[idx] = 
                workspace.add_lua_value(arche->m_runtime->m_static_funcs[idx]);
    }
}

/**
 * @brief Make "redundant" copies of already-compiled data to speed up later
 * runtime usage.
 */
void compile_archetype_make_redundant_copies(Work::Space& workspace,
        std::unique_ptr<Work::Arche>& arche) {
    arche->m_runtime->m_sorted_component_array.reserve(
            arche->m_runtime->m_comp_offsets.size());
    for (auto iter : arche->m_runtime->m_comp_offsets) {
        arche->m_runtime->m_sorted_component_array.push_back(iter.first);
    }
    assert(std::is_sorted(arche->m_runtime->m_sorted_component_array.begin(),
                        arche->m_runtime->m_sorted_component_array.end()));
}

std::unique_ptr<Work::Arche> compile_archetype(Work::Space& workspace, 
        std::unique_ptr<Interm::Arche>&& interm) {
    
    // Make the working archetype
    std::unique_ptr<Work::Arche> arche = 
            std::make_unique<Work::Arche>(std::move(interm));

    // Find the total size of the pod data and make a chunk for the archetype
    compile_archetype_resize_pod(workspace, arche);
    compile_archetype_fill_pod(workspace, arche);
    compile_archetype_store_strings(workspace, arche);
    compile_archetype_store_static_lua_values(workspace, arche);
    compile_archetype_make_redundant_copies(workspace, arche);
    
    return arche;
}

std::unique_ptr<Work::Genre> compile_genre(Work::Space& workspace, 
        std::unique_ptr<Interm::Genre>&& interm) {
    
    // Make the working genre
    std::unique_ptr<Work::Genre> genre = 
            std::make_unique<Work::Genre>(std::move(interm));
    
    for (const Interm::Genre::Pattern& interm_pattern : 
            genre->m_interm->m_patterns) {
        
        // Note that we may require components that are not read from.
        Runtime::Pattern runtime_pattern;
        for (auto locally_named_comp_iter : interm_pattern.m_matching) {
            Interm::Comp* interm_comp = locally_named_comp_iter.second;
            const auto& comp_iter = 
                    workspace.get_comps_by_interm().find(interm_comp);
            assert(comp_iter != workspace.get_comps_by_interm().end());
            const Work::Comp* comp = comp_iter->second;
            runtime_pattern.m_sorted_required_comps_specific
                    .push_back(comp->m_runtime.get());
        }
        std::sort(runtime_pattern.m_sorted_required_comps_specific.begin(),
                runtime_pattern.m_sorted_required_comps_specific.end());
        
        // Convert all the aliases
        for (auto interm_alias_iter : interm_pattern.m_aliases) {
            const Interm::Symbol& alias_symbol = interm_alias_iter.first;
            const Interm::Genre::Pattern::Alias& interm_alias = 
                    interm_alias_iter.second;
            
            const auto& comp_iter = 
                    workspace.get_comps_by_interm().find(interm_alias.m_comp);
            assert(comp_iter != workspace.get_comps_by_interm().end());
            const Work::Comp* comp = comp_iter->second;
            
            Runtime::Pattern::Alias runtime_alias;
            runtime_alias.m_comp = comp->m_runtime.get();
            
            /*
            Logger::log()->info("asdf: %v", interm_alias.m_member);
            for (auto asdf : runtime_alias.m_comp->m_member_offsets) {
                Logger::log()->info(asdf.first);
            }*/
            
            auto prim_iter =
                    runtime_alias.m_comp->m_member_offsets.find(
                            interm_alias.m_member);
            assert(prim_iter != runtime_alias.m_comp->m_member_offsets.end());
            
            runtime_alias.m_prim_copy = prim_iter->second;
            
            assert(runtime_pattern.m_aliases.find(alias_symbol) == 
                    runtime_pattern.m_aliases.end());
            runtime_pattern.m_aliases[alias_symbol] = runtime_alias;
        }
        
        genre->m_runtime->m_patterns.push_back(runtime_pattern);
    }
    
    // TODO: remove the intersection of all patterns
    
    return genre;
}

void compile() {
    Logger::log()->info("Gensys compilation starting...");
    assert(get_global_state() == GlobalState::MUTABLE);
    
    Logger::log()->info("Creating workspace...");
    Work::Space workspace;

    Logger::log()->info("Compiling components...");
    for (auto& entry : n_staged_comps) {
        Logger::log()->info("-> %v", entry.first);
        auto comp = compile_component(workspace, std::move(entry.second));
        workspace.add_comp(std::move(comp), entry.first);
    }

    Logger::log()->info("Processing archetypes...");
    for (auto& entry : n_staged_arches) {
        Logger::log()->info("-> %v", entry.first);
        auto arche = compile_archetype(workspace, std::move(entry.second));
        workspace.add_arche(std::move(arche), entry.first);
    }

    Logger::log()->info("Processing genres...");
    for (auto& entry : n_staged_genres) {
        auto genre = compile_genre(workspace, std::move(entry.second));
        workspace.add_genre(std::move(genre), entry.first);
    }
    
    Runtime::cleanup();
    
    Logger::log()->info("Moving components...");
    for (const auto& entry : workspace.get_comps_by_id()) {
        Runtime::n_runtime_comps[entry.first] 
                = std::move(entry.second->m_runtime);
    }
    
    Logger::log()->info("Moving archetypes...");
    for (const auto& entry : workspace.get_arches_by_id()) {
        Runtime::n_runtime_arches[entry.first] 
                = std::move(entry.second->m_runtime);
    }
    
    Logger::log()->info("Moving genres...");
    for (const auto& entry : workspace.get_genres_by_id()) {
        Runtime::n_runtime_genres[entry.first] 
                = std::move(entry.second->m_runtime);
    }
    
    Logger::log()->info("Moving Lua registry references...");
    Runtime::n_held_lua_values = std::move(workspace.release_lua_uniques());
    
    Logger::log()->info("Compilation complete");
}

void overwrite(Resour::Oid id_str, const char* attacker) {
    {
        auto iter = n_staged_comps.find(id_str);
        if (iter != n_staged_comps.end()) {
            Logger::log()->warn(
                    "Overwriting staged component [%v] with %v", attacker);
            n_staged_comps.erase(iter);
        }
    }
    {
        auto iter = n_staged_arches.find(id_str);
        if (iter != n_staged_arches.end()) {
            Logger::log()->warn(
                    "Overwriting staged archetype [%v] with %v", attacker);
            n_staged_arches.erase(iter);
        }
    }
    {
        auto iter = n_staged_genres.find(id_str);
        if (iter != n_staged_genres.end()) {
            Logger::log()->warn(
                    "Overwriting staged genre [%v] with %v", attacker);
            n_staged_genres.erase(iter);
        }
    }
}

void stage_component(Resour::Oid id_str,
        std::unique_ptr<Interm::Comp>&& comp) {
    overwrite(id_str, "component");
    n_staged_comps[id_str] = std::move(comp);
}
Interm::Comp* get_staged_component(Resour::Oid id_str) {
    return Util::find_something(n_staged_comps, id_str, 
            "Could not find staged component: %v");
}
void unstage_component(Resour::Oid id_str) {
    Util::erase_something(n_staged_comps, id_str, 
            "Could not find staged component: %v");
}
void stage_archetype(Resour::Oid id_str,
        std::unique_ptr<Interm::Arche>&& arche) {
    overwrite(id_str, "archetype");
    n_staged_arches[id_str] = std::move(arche);
}
Interm::Arche* get_staged_archetype(Resour::Oid id_str) {
    return Util::find_something(n_staged_arches, id_str, 
            "Could not find staged archetype: %v");
}
void unstage_archetype(Resour::Oid id_str) {
    Util::erase_something(n_staged_arches, id_str, 
            "Could not find staged archetype: %v");
}
void stage_genre(Resour::Oid id_str, std::unique_ptr<Interm::Genre>&& genre) {
    overwrite(id_str, "genre");
    n_staged_genres[id_str] = std::move(genre);
}
Interm::Genre* get_staged_genre(Resour::Oid id_str) {
    return Util::find_something(n_staged_genres, id_str, 
            "Could not find staged genre: %v");
}
void unstage_genre(Resour::Oid id_str) {
    Util::erase_something(n_staged_genres, id_str, 
            "Could not find staged genre: %v");
}

ObjectType get_staged_type(Resour::Oid id) {
    if (n_staged_comps.find(id) != n_staged_comps.end()) {
        return ObjectType::COMP_DEF;
    }
    if (n_staged_arches.find(id) != n_staged_arches.end()) {
        return ObjectType::ARCHETYPE;
    }
    if (n_staged_genres.find(id) != n_staged_genres.end()) {
        return ObjectType::GENRE;
    }
    return ObjectType::NOT_FOUND;
}

} // namespace Compiler
} // namespace Gensys
} // namespace pegr
