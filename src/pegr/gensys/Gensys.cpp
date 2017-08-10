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

#include "pegr/gensys/Gensys.hpp"

#include "pegr/gensys/Compiler.hpp"
#include "pegr/gensys/Util.hpp"
#include "pegr/gensys/Runtime.hpp"

namespace pegr {
namespace Gensys {

GlobalState m_global_state = GlobalState::UNINITIALIZED;

GlobalState get_global_state() {
    return m_global_state;
}

void initialize() {
    assert(m_global_state == GlobalState::UNINITIALIZED);
    Runtime::initialize();
    Compiler::initialize();
    m_global_state = GlobalState::MUTABLE;
}
void compile() {
    assert(m_global_state == GlobalState::MUTABLE);
    Compiler::compile();
    m_global_state = GlobalState::EXECUTABLE;
}
void cleanup() {
    assert(m_global_state != GlobalState::UNINITIALIZED);
    Compiler::cleanup();
    Runtime::cleanup();
    m_global_state = GlobalState::UNINITIALIZED;
}
    
} // namespace Gensys
} // namespace pegr

