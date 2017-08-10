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

#ifndef PEGR_GENSYS_GENSYS_HPP
#define PEGR_GENSYS_GENSYS_HPP

namespace pegr {
namespace Gensys {

enum struct GlobalState {
    // Stage before initialize() is called for the first time
    UNINITIALIZED,
    
    // Stage where new elements can be added into the system
    MUTABLE,
    
    // Stage after "compiling," entities can be created and processed now
    EXECUTABLE,
    
    ENUM_SIZE /*Number of valid enum values*/
};

GlobalState get_global_state();

void initialize();
void compile();
void cleanup();
    
} // namespace Gensys
} // namespace pegr

#endif // PEGR_GENSYS_GENSYS_HPP
