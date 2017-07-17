#include "pegr/gensys/Compiler.hpp"

#include <map>
#include <cassert>
#include <sstream>
#include <vector>
#include <memory>

#include "pegr/logger/Logger.hpp"
#include "pegr/gensys/Runtime_Types.hpp"
#include "pegr/gensys/Util.hpp"

namespace pegr {
namespace Gensys {

namespace Work {

struct Comp {
    Comp(std::unique_ptr<Interm::Comp_Def>&& interm)
    : m_interm(std::move(interm)) {
        m_runtime = std::make_unique<Runtime::Component>();
    }
    
    std::unique_ptr<Interm::Comp_Def> m_interm;
    std::unique_ptr<Runtime::Component> m_runtime;
    std::map<Interm::Symbol, std::size_t> m_symbol_to_offset;
    Pod::Unique_Chunk_Ptr m_compiled_chunk;
    std::vector<std::string> m_strings;
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
 * @brief Removes from a vector of unique pointers by comparing their .get()
 * by value
 * @param vec
 * @param ptr
 */
template<typename T, typename V>
void vector_remove_unique_ptrs(std::vector<std::unique_ptr<T> >& vec, V& ptr) {
    vec.erase(std::remove_if(vec.begin(), vec.end(), 
            [&](const std::unique_ptr<T>& elem) -> bool {
                return elem.get() == ptr;
            }), vec.end());
}

/**
 * @class Space
 * @brief Where all the unique objects are stored during the compilation
 * prcess. Exists only during a call to compile()
 */
class Space {
private:
    std::vector<std::unique_ptr<Comp> > m_comps;
    std::vector<std::unique_ptr<Arche> > m_arches;
    std::vector<std::unique_ptr<Genre> > m_genres;

    std::map<const Interm::Comp_Def*, Comp*> m_comps_by_interm;
    std::map<const Interm::Arche*, Arche*> m_arches_by_interm;
    std::map<const Interm::Genre*, Genre*> m_genres_by_interm;

    std::map<std::string, Comp*> m_comps_by_id;
    std::map<std::string, Arche*> m_arches_by_id;
    std::map<std::string, Genre*> m_genres_by_id;
    
public:
    const std::vector<std::unique_ptr<Comp> >& get_comps() const {
        return m_comps;
    }
    const std::vector<std::unique_ptr<Arche> >& get_arches() const {
        return m_arches;
    }
    const std::vector<std::unique_ptr<Genre> >& get_genres() const {
        return m_genres;
    }
    
    const std::map<const Interm::Comp_Def*, Comp*>& get_comps_by_interm() const {
        return m_comps_by_interm;
    }
    const std::map<const Interm::Arche*, Arche*>& get_arches_by_interm() const {
        return m_arches_by_interm;
    }
    const std::map<const Interm::Genre*, Genre*>& get_genres_by_interm() const {
        return m_genres_by_interm;
    }
    
    const std::map<std::string, Comp*>& get_comps_by_id() const {
        return m_comps_by_id;
    }
    const std::map<std::string, Arche*>& get_arches_by_id() const {
        return m_arches_by_id;
    }
    const std::map<std::string, Genre*>& get_genres_by_id() const {
        return m_genres_by_id;
    }
    
    void add_comp(std::unique_ptr<Comp>&& obj, std::string id) {
        m_comps_by_interm[obj->m_interm.get()] = obj.get();
        m_comps_by_id[id] = obj.get();
        m_comps.emplace_back(std::move(obj));
    }
    
    void add_arche(std::unique_ptr<Arche>&& obj, std::string id) {
        m_arches_by_interm[obj->m_interm.get()] = obj.get();
        m_arches_by_id[id] = obj.get();
        m_arches.emplace_back(std::move(obj));
    }
    
    void add_genre(std::unique_ptr<Genre>&& obj, std::string id) {
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

} // namespace Staged

std::map<std::string, std::unique_ptr<Runtime::Component> > m_runtime_comps;
std::map<std::string, std::unique_ptr<Runtime::Arche> > m_runtime_arches;
std::map<std::string, std::unique_ptr<Runtime::Genre> > m_runtime_genres;

std::map<std::string, std::unique_ptr<Interm::Comp_Def> > m_staged_comps;
std::map<std::string, std::unique_ptr<Interm::Arche> > m_staged_arches;
std::map<std::string, std::unique_ptr<Interm::Genre> > m_staged_genres;

// TODO: make into a single lua table
std::vector<Script::Regref_Guard> m_held_functions;

GlobalState m_global_state = GlobalState::UNINITIALIZED;

GlobalState get_global_state() {
    return m_global_state;
}

void initialize() {
    assert(m_global_state == GlobalState::UNINITIALIZED);
    m_global_state = GlobalState::MUTABLE;
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
    std::size_t string_run_idx = 0;
    for (const auto& member : comp->m_interm->m_members) {
        //
        const Interm::Symbol& symbol = member.first;
        const Interm::Prim& prim = member.second;
        
        switch (prim.get_type()) {
            case Interm::Prim::Type::STR: {
                comp->m_strings.push_back(prim.get_string());
                comp->m_symbol_to_offset[symbol] = string_run_idx;
                ++string_run_idx;
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
                runtime_prim.m_byte_offset = offset;
                break;
            }
            case Runtime::Prim::Type::STR: {
                runtime_prim.m_index = offset;
                break;
            }
            case Runtime::Prim::Type::FUNC: {
                // TODO
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
        std::unique_ptr<Interm::Comp_Def>&& interm) {
    
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
    arche->m_runtime->m_default_chunk.reset(Pod::new_pod_chunk(total_size));
}

void compile_archetype_fill_pod(Work::Space& workspace,
        std::unique_ptr<Work::Arche>& arche) {

    // Stores how many bytes have already been used up in the POD chunk
    std::size_t accumulated = 0;

    // For every implementation
    for (const auto& implem_pair : arche->m_interm->m_implements) {
        
        // Get the implementation
        const Interm::Arche::Implement& implem = implem_pair.second;

        const auto& comp_iter = 
                workspace.get_comps_by_interm().find(implem.m_component);
        
        assert(comp_iter != workspace.get_comps_by_interm().end());
        
        const Work::Comp* comp = comp_iter->second;

        // Copy the pod chunk
        Pod::copy_pod_chunk(
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

        // Remember how to find this data later
        arche->m_runtime->m_comp_offsets[comp->m_runtime.get()].m_pod_idx 
                = accumulated;

        // Keep track of how much space has been used
        accumulated += comp->m_compiled_chunk.get().get_size();
    }

    // This should have exactly filled the POD chunk (guaranteed earlier)
    assert(accumulated == arche->m_runtime->m_default_chunk.get().get_size());
}

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

    // This should have exactly filled the string vector
    assert(accumulated == arche->m_runtime->m_default_strings.size());
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
    
    return arche;
}

std::unique_ptr<Work::Genre> compile_genre(Work::Space& workspace, 
        std::unique_ptr<Interm::Genre>&& interm) {
    
    // Make the working archetype
    std::unique_ptr<Work::Genre> genre = 
            std::make_unique<Work::Genre>(std::move(interm));
    
    // TODO
    
    return genre;
}

void compile() {
    assert(m_global_state == GlobalState::MUTABLE);
    
    Work::Space workspace;

    for (auto& entry : m_staged_comps) {
        auto comp = compile_component(workspace, std::move(entry.second));
        workspace.add_comp(std::move(comp), entry.first);
    }

    for (auto& entry : m_staged_arches) {
        auto arche = compile_archetype(workspace, std::move(entry.second));
        workspace.add_arche(std::move(arche), entry.first);
    }

    for (auto& entry : m_staged_genres) {
        auto genre = compile_genre(workspace, std::move(entry.second));
        workspace.add_genre(std::move(genre), entry.first);
    }
    
    m_runtime_comps.clear();
    m_runtime_arches.clear();
    m_runtime_genres.clear();
    
    for (const auto& entry : workspace.get_comps_by_id()) {
        m_runtime_comps[entry.first] = std::move(entry.second->m_runtime);
    }
    
    for (const auto& entry : workspace.get_arches_by_id()) {
        m_runtime_arches[entry.first] = std::move(entry.second->m_runtime);
    }
    
    for (const auto& entry : workspace.get_genres_by_id()) {
        m_runtime_genres[entry.first] = std::move(entry.second->m_runtime);
    }

    m_global_state = GlobalState::EXECUTABLE;
}

void cleanup() {
    assert(m_global_state != GlobalState::UNINITIALIZED);
    m_staged_comps.clear();
    m_staged_arches.clear();
    m_staged_genres.clear();
    m_runtime_comps.clear();
    m_runtime_arches.clear();
    m_runtime_genres.clear();
    m_global_state = GlobalState::UNINITIALIZED;
}

void overwrite(std::string id_str, const char* attacker) {
    {
        auto iter = m_staged_comps.find(id_str);
        if (iter != m_staged_comps.end()) {
            Logger::log()->warn(
                    "Overwriting staged component [%v] with %v", attacker);
            m_staged_comps.erase(iter);
        }
    }
    {
        auto iter = m_staged_arches.find(id_str);
        if (iter != m_staged_arches.end()) {
            Logger::log()->warn(
                    "Overwriting staged archetype [%v] with %v", attacker);
            m_staged_arches.erase(iter);
        }
    }
    {
        auto iter = m_staged_genres.find(id_str);
        if (iter != m_staged_genres.end()) {
            Logger::log()->warn(
                    "Overwriting staged genre [%v] with %v", attacker);
            m_staged_genres.erase(iter);
        }
    }
}

template<typename K, typename V>
V* find_something(std::map<K, std::unique_ptr<V> >& map, const K& key, 
        const char* err_msg) {
    auto iter = map.find(key);
    if (iter == map.end()) {
        Logger::log()->warn(err_msg, key);
        return nullptr;
    }
    return iter->second.get();
}

template<typename K, typename V>
void erase_something(std::map<K, V>& map, const K& key, const char* err_msg) {
    auto iter = map.find(key);
    if (iter == map.end()) {
        Logger::log()->warn(err_msg, key);
        return;
    }
    map.erase(iter);
}

void stage_component(std::string id_str,
        std::unique_ptr<Interm::Comp_Def>&& comp) {
    overwrite(id_str, "component");
    m_staged_comps[id_str] = std::move(comp);
}
Interm::Comp_Def* get_staged_component(std::string id_str) {
    return find_something(m_staged_comps, id_str, 
            "Could not find staged component: %v");
}
void unstage_component(std::string id_str) {
    erase_something(m_staged_comps, id_str, 
            "Could not find staged component: %v");
}
void stage_archetype(std::string id_str,
        std::unique_ptr<Interm::Arche>&& arche) {
    overwrite(id_str, "archetype");
    m_staged_arches[id_str] = std::move(arche);
}
Interm::Arche* get_staged_archetype(std::string id_str) {
    return find_something(m_staged_arches, id_str, 
            "Could not find staged archetype: %v");
}
void unstage_archetype(std::string id_str) {
    erase_something(m_staged_arches, id_str, 
            "Could not find staged archetype: %v");
}
void stage_genre(std::string id_str, std::unique_ptr<Interm::Genre>&& genre) {
    overwrite(id_str, "genre");
    m_staged_genres[id_str] = std::move(genre);
}
Interm::Genre* get_staged_genre(std::string id_str) {
    return find_something(m_staged_genres, id_str, 
            "Could not find staged genre: %v");
}
void unstage_genre(std::string id_str) {
    erase_something(m_staged_genres, id_str, 
            "Could not find staged genre: %v");
}

ObjectType get_staged_type(std::string id) {
    if (m_staged_comps.find(id) != m_staged_comps.end()) {
        return ObjectType::COMP_DEF;
    }
    if (m_staged_arches.find(id) != m_staged_arches.end()) {
        return ObjectType::ARCHETYPE;
    }
    if (m_staged_genres.find(id) != m_staged_genres.end()) {
        return ObjectType::GENRE;
    }
    return ObjectType::NOT_FOUND;
}
Runtime::Component* find_component(std::string id_str) {
    return find_something(m_runtime_comps, id_str, 
            "Could not find component: %v");
}
Runtime::Arche* find_archetype(std::string id_str) {
    return find_something(m_runtime_arches, id_str, 
            "Could not find archetype: %v");
}
Runtime::Genre* find_genre(std::string id_str) {
    return find_something(m_runtime_genres, id_str, 
            "Could not find genre: %v");
}

} // namespace Gensys
} // namespace pegr
