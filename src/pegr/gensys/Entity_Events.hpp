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

#ifndef PEGR_GENSYS_ENTITYTICKEVENT_HPP
#define PEGR_GENSYS_ENTITYTICKEVENT_HPP

#include <functional>

#include "pegr/scheduler/Sched.hpp"
#include "pegr/gensys/Runtime_Types.hpp"

namespace pegr {
namespace Gensys {
    
class Ete_Listener {
public:
    enum Selector_Type {
        COMP,
        ARCHE,
        GENRE
    };
    
    union Selector {
        Runtime::Comp* m_comp;
        Runtime::Arche* m_arche;
        Runtime::Genre* m_genre;
    };
    
private:
    Selector_Type m_selector;
    Selector m_selector_union;
    
    friend class Entity_Tick_Event;
};
    
class Entity_Tick_Event : public Schedu::Event {
public:

    Entity_Tick_Event();
    virtual ~Entity_Tick_Event();
    
    void add_listener(Ete_Listener listener);

    virtual Schedu::Event::Type get_type() const override;
    virtual void trigger() override;
    
private:

    std::vector<Ete_Listener> m_listeners;
};

} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_ENTITYTICKEVENT_HPP
