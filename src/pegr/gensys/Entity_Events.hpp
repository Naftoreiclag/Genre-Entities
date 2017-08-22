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

#include <functional>

#include "pegr/gensys/Runtime_Types.hpp"
#include "pegr/scheduler/Sched.hpp"

namespace pegr {
namespace Gensys {
namespace Runtime {

class Entity_Listener {
public:
    Entity_Listener(std::function<void(Entity*)> func);
    
    void call(Runtime::Entity* ent);
    
private:
    std::function<void(Entity*)> m_func;
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
    
class Entity_Tick_Event : public Schedu::Event {
public:

    Entity_Tick_Event();
    virtual ~Entity_Tick_Event();
    
    void add_listener(Matching_Entity_Listener<Arche> listener);
    void add_listener(Matching_Entity_Listener<Comp> listener);
    void add_listener(Matching_Entity_Listener<Genre> listener);

    virtual Schedu::Event::Type get_type() const override;
    void trigger();
    
private:

    std::vector<Matching_Entity_Listener<Arche> > m_arche_listeners;
    std::vector<Matching_Entity_Listener<Comp> > m_comp_listeners;
    std::vector<Matching_Entity_Listener<Genre> > m_genre_listeners;
};

template <Schedu::Event::Type s_event_type>
class Entity_Event : public Schedu::Event {
public:
    
    Entity_Event()
    : Schedu::Event() {}
    virtual ~Entity_Event() {}
    
    void add_listener(Entity_Listener listener) {
        m_listeners.push_back(listener);
    }
    
    virtual Schedu::Event::Type get_type() const override {
        return s_event_type;
    }
    void trigger(Entity* ent) {
        for (Entity_Listener listener : m_listeners) {
            listener.call(ent);
        }
    }
    
private:

    std::vector<Entity_Listener> m_listeners;
};

typedef Entity_Event<Schedu::Event::Type::ENTITY_KILLED> Entity_Killed_Event;
typedef Entity_Event<Schedu::Event::Type::ENTITY_SPAWNED> Entity_Spawned_Event;

} // namespace Runtime
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_ENTITYEVENTS_HPP
