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

#ifndef PEGR_EXCEPT_EXCEPT_HPP
#define PEGR_EXCEPT_EXCEPT_HPP

#include <string>
#include <stdexcept>

namespace pegr {
namespace Except {
    
class Runtime : public std::runtime_error {
public:
    Runtime(const std::string& msg);
};

} // namespace Except
} // namespace pegr

#endif // PEGR_EXCEPT_EXCEPT_HPP
