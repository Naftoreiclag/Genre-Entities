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

#ifndef PEGR_GENSYS_ENTITYEVENTS_HPP
#define PEGR_GENSYS_ENTITYEVENTS_HPP

#include <cstdint>
#include <functional>

#include "pegr/algs/QIFU_Map.hpp"
#include "pegr/gensys/Runtime_Types.hpp"
#include "pegr/scheduler/Sched.hpp"

namespace pegr {
namespace Gensys {
namespace Event {
    
typedef std::uint64_t Listener_Handle;

extern const Listener_Handle EMPTY_HANDLE;

class Entity_Listener {
public:
    Entity_Listener(std::function<void(Runtime::Entity*)> func);
    
    void call(Runtime::Entity* ent);
    
private:
    std::function<void(Runtime::Entity*)> m_func;
};

template<typename Select_T>
class Matching_Entity_Listener {
public:

    /* View_T is the type of the return of calling the .match() method of an
     * instance of the Select_T class. It can be a raw pointer (like in Arche)
     * or a class (like in Comp and Genre), but it must have explicit operator
     * bool and copy/move assignable.
     */
    typedef typename std::result_of<
            decltype(&Select_T::match)
                    (Select_T*, Runtime::Entity*)>::type View_T;

    Matching_Entity_Listener(Select_T* selector, 
            std::function<void(View_T)> func)
    : m_selector(selector)
    , m_func(func) {}
    
    Select_T* get_selector() {
        return m_selector;
    }
    
    void call(Runtime::Entity* ent) {
        View_T view = m_selector->match(ent);
        if (view) {
            m_func(view);
        }
    }
    
private:
    Select_T* m_selector;
    std::function<void(View_T)> m_func;
};

typedef Matching_Entity_Listener<Runtime::Arche> Arche_Entity_Listener;
typedef Matching_Entity_Listener<Runtime::Comp> Comp_Entity_Listener;
typedef Matching_Entity_Listener<Runtime::Genre> Genre_Entity_Listener;
    
class Entity_Tick_Event : public Schedu::Event {
public:

    Entity_Tick_Event();
    virtual ~Entity_Tick_Event();
    
    Listener_Handle hook(Arche_Entity_Listener listener);
    Listener_Handle hook(Comp_Entity_Listener listener);
    Listener_Handle hook(Genre_Entity_Listener listener);
    
    bool unhook(Listener_Handle handle);

    virtual Schedu::Event::Type get_type() const override;
    void trigger();
    
private:

    Algs::QIFU_Map<Listener_Handle, Arche_Entity_Listener> m_arche_listeners;
    Algs::QIFU_Map<Listener_Handle, Comp_Entity_Listener> m_comp_listeners;
    Algs::QIFU_Map<Listener_Handle, Genre_Entity_Listener> m_genre_listeners;
};

template <Schedu::Event::Type s_event_type>
class Entity_Event : public Schedu::Event {
public:
    
    Entity_Event()
    : Schedu::Event() {}
    virtual ~Entity_Event() {}
    
    Listener_Handle hook(Entity_Listener listener) {
        return m_listeners.add(listener);
    }
    
    bool unhook(Listener_Handle handle) {
        return m_listeners.remove(handle);
    }
    
    virtual Schedu::Event::Type get_type() const override {
        return s_event_type;
    }
    void trigger(Runtime::Entity* ent) {
        m_listeners.for_each([ent](Entity_Listener* listener) {
            listener->call(ent);
        });
    }
    
private:

    Algs::QIFU_Map<Listener_Handle, Entity_Listener> m_listeners;
};

typedef Entity_Event<Schedu::Event::Type::ENTITY_KILLED> Entity_Killed_Event;
typedef Entity_Event<Schedu::Event::Type::ENTITY_SPAWNED> Entity_Spawned_Event;

} // namespace Event
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_ENTITYEVENTS_HPP
