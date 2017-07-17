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

namespace Staged {

struct Comp {
    Comp() = default;
    Comp(std::unique_ptr<Interm::Comp_Def>&& interm)
    : m_interm(std::move(interm)) {}
    
    std::unique_ptr<Interm::Comp_Def> m_interm;
    std::unique_ptr<Runtime::Component> m_runtime;
    std::map<Interm::Symbol, std::size_t> m_symbol_to_offset;
    Pod::Unique_Chunk_Ptr m_compiled_chunk;
};
struct Arche {
    Arche() = default;
    Arche(std::unique_ptr<Interm::Arche>&& interm)
    : m_interm(std::move(interm)) {}
    
    std::unique_ptr<Interm::Arche> m_interm;
    std::unique_ptr<Runtime::Arche> m_runtime;
};
struct Genre {
    Genre() = default;
    Genre(std::unique_ptr<Interm::Genre>&& interm)
    : m_interm(std::move(interm)) {}
    
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

class Collection {
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
    
    void erase(Comp* comp) {
        vector_remove_unique_ptrs(m_comps, comp);
    }
    
    void erase(Arche* arche) {
        vector_remove_unique_ptrs(m_arches, arche);
    }
    
    void erase(Genre* genre) {
        vector_remove_unique_ptrs(m_genres, genre);
    }
    
    void add_comp(std::string id, std::unique_ptr<Comp>&& obj) {
        m_comps_by_interm[obj->m_interm.get()] = obj.get();
        m_comps_by_id[id] = obj.get();
        m_comps.emplace_back(std::move(obj));
    }
    
    void add_arche(std::string id, std::unique_ptr<Arche>&& obj) {
        m_arches_by_interm[obj->m_interm.get()] = obj.get();
        m_arches_by_id[id] = obj.get();
        m_arches.emplace_back(std::move(obj));
    }
    
    void add_genre(std::string id, std::unique_ptr<Genre>&& obj) {
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

Staged::Collection m_staged;

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

void compile_component(Staged::Comp* comp) {
    // Pack POD data into the chunk, and record where each member was placed
    comp->m_compiled_chunk.reset(
            Util::new_pod_chunk_from_interm_prims(
                    comp->m_interm->m_members,
                    comp->m_symbol_to_offset));
    
    // Construct runtime data
    comp->m_runtime = std::make_unique<Runtime::Component>();
    
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
        runtime_prim.m_byte_offset = offset;
        
        // Ensure that the runtime primitive is actually POD
        assert(runtime_prim.m_type != Runtime::Prim::Type::FUNC);
        assert(runtime_prim.m_type != Runtime::Prim::Type::STR);
        
        // Store in runtime data
        comp->m_runtime->m_member_offsets[symbol] = std::move(runtime_prim);
    }
}

void compile_archetype(const Staged::Arche* arche) {
    // Find the total size of the pod data and make a chunk for the archetype
    {
        // Find the total size, which is the sum of the component POD chunk
        std::size_t total_size = 0;
        for (const auto& implem_pair : arche->m_interm->m_implements) {
            const Interm::Arche::Implement& implem = implem_pair.second;
            const auto& comp_iter = 
                    m_staged.get_comps_by_interm().find(implem.m_component);
            assert(comp_iter != m_staged.get_comps_by_interm().end());
            const Staged::Comp* comp = comp_iter->second;
            total_size += comp->m_compiled_chunk.get().get_size();
        }
        arche->m_runtime->m_default_chunk.reset(Pod::new_pod_chunk(total_size));
    }

    // POD
    {
        // Stores how many bytes have already been used up in the POD chunk
        std::size_t accumulated = 0;

        // For every implementation
        for (const auto& implem_pair : arche->m_interm->m_implements) {
            
            // Get the implementation
            const Interm::Arche::Implement& implem = implem_pair.second;

            const auto& comp_iter = 
                    m_staged.get_comps_by_interm().find(implem.m_component);
            
            assert(comp_iter != m_staged.get_comps_by_interm().end());
            
            const Staged::Comp* comp = comp_iter->second;

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

            // Copy over the offsets
            // 

            // Keep track of how much space has been used
            accumulated += comp->m_compiled_chunk.get().get_size();
        }

        // This should have exactly filled the POD chunk (guaranteed above)
        assert(accumulated
                == arche->m_runtime->m_default_chunk.get().get_size());
    }

    // Strings
    {

    }
}

void compile() {
    assert(m_global_state == GlobalState::MUTABLE);

    for (const auto& comp : m_staged.get_comps()) {
        compile_component(comp.get());
    }

    for (const auto& arche : m_staged.get_arches()) {
        compile_archetype(arche.get());
    }

    m_global_state = GlobalState::EXECUTABLE;
}

void cleanup() {
    assert(m_global_state != GlobalState::UNINITIALIZED);
    m_staged.clear();
    m_global_state = GlobalState::UNINITIALIZED;
}

void overwrite(std::string id_str, const char* attacker) {
    {
        auto iter = m_staged.get_comps_by_id().find(id_str);
        if (iter != m_staged.get_comps_by_id().end()) {
            Logger::log()->warn(
                    "Overwriting staged component [%v] with %v", attacker);
            m_staged.erase(iter->second);
        }
    }
    {
        auto iter = m_staged.get_arches_by_id().find(id_str);
        if (iter != m_staged.get_arches_by_id().end()) {
            Logger::log()->warn(
                    "Overwriting staged archetype [%v] with %v", attacker);
            m_staged.erase(iter->second);
        }
    }
    {
        auto iter = m_staged.get_genres_by_id().find(id_str);
        if (iter != m_staged.get_genres_by_id().end()) {
            Logger::log()->warn(
                    "Overwriting staged genre [%v] with %v", attacker);
            m_staged.erase(iter->second);
        }
    }
}

void stage_component(std::string id_str,
        std::unique_ptr<Interm::Comp_Def>&& comp) {
    overwrite(id_str, "component");
    m_staged.add_comp(id_str, 
            std::make_unique<Staged::Comp>(std::move(comp)));
}
Interm::Comp_Def* get_staged_component(std::string id_str) {
    auto iter = m_staged.get_comps_by_id().find(id_str);
    if (iter == m_staged.get_comps_by_id().end()) {
        Logger::log()->warn("Could not find staged component: %v", id_str);
        return nullptr;
    }
    return (iter->second)->m_interm.get();
}
void unstage_component(std::string id_str) {
    auto iter = m_staged.get_comps_by_id().find(id_str);
    if (iter == m_staged.get_comps_by_id().end()) {
        Logger::log()->warn("Could not find staged component: %v", id_str);
        return;
    }
    m_staged.erase(iter->second);
}

void stage_archetype(std::string id_str,
        std::unique_ptr<Interm::Arche>&& arche) {
    overwrite(id_str, "archetype");
    m_staged.add_arche(id_str, 
            std::make_unique<Staged::Arche>(std::move(arche)));
}
Interm::Arche* get_staged_archetype(std::string id_str) {
    auto iter = m_staged.get_arches_by_id().find(id_str);
    if (iter == m_staged.get_arches_by_id().end()) {
        Logger::log()->warn("Could not find staged archetype: %v", id_str);
        return nullptr;
    }
    return (iter->second)->m_interm.get();
}
void unstage_archetype(std::string id_str) {
    auto iter = m_staged.get_arches_by_id().find(id_str);
    if (iter == m_staged.get_arches_by_id().end()) {
        Logger::log()->warn("Could not find staged archetype: %v", id_str);
        return;
    }
    m_staged.erase(iter->second);
}
void stage_genre(std::string id_str, std::unique_ptr<Interm::Genre>&& genre) {
    overwrite(id_str, "genre");
    m_staged.add_genre(id_str, 
            std::make_unique<Staged::Genre>(std::move(genre)));
}
Interm::Genre* get_staged_genre(std::string id_str) {
    auto iter = m_staged.get_genres_by_id().find(id_str);
    if (iter == m_staged.get_genres_by_id().end()) {
        Logger::log()->warn("Could not find staged genre: %v", id_str);
        return nullptr;
    }
    return (iter->second)->m_interm.get();
}
void unstage_genre(std::string id_str) {
    auto iter = m_staged.get_genres_by_id().find(id_str);
    if (iter == m_staged.get_genres_by_id().end()) {
        Logger::log()->warn("Could not find staged genre: %v", id_str);
        return;
    }
    m_staged.erase(iter->second);
}

ObjectType get_staged_type(std::string id) {
    if (m_staged.get_comps_by_id().find(id) 
            != m_staged.get_comps_by_id().end()) {
        return ObjectType::COMP_DEF;
    }
    if (m_staged.get_arches_by_id().find(id) 
            != m_staged.get_arches_by_id().end()) {
        return ObjectType::ARCHETYPE;
    }
    if (m_staged.get_genres_by_id().find(id) 
            != m_staged.get_genres_by_id().end()) {
        return ObjectType::GENRE;
    }
    return ObjectType::NOT_FOUND;
}

Runtime::Component* find_component(std::string id) {}
Runtime::Arche* find_archetype(std::string id) {}
Runtime::Genre* find_genre(std::string id) {}

} // namespace Gensys
} // namespace pegr
