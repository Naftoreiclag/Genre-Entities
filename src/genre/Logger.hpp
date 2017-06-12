/*
 *  Copyright 2016-2017 James Fong
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

#ifndef PEGR_ENGINELOGGER_HPP
#define PEGR_ENGINELOGGER_HPP

#include <cstdint>
#include <iostream>
#include <sstream>

namespace pegr {
namespace Logger {

    class Channel;
    class OutBuffer;
    class Out : public std::ostream {
    public:
        //Out();
        Out(Channel* channel);
        Out(const Out& copyCtr);
        //operator=(const Out& assignment);
        ~Out();
        
        // Instead of re-assigning to an Out instance, switch between Channel pointers!
        
        void indent();
        void unindent();
    private:
        OutBuffer* mOutBuf;
        Channel* const mChannel;
    };
    
    class OutBuffer : public std::stringbuf {
    public:
        OutBuffer(Channel* channel);
        
        int sync();
    private:
        uint16_t mIndent;
        Channel* const mChannel;
        friend class Out;
    };

    class Channel {
    public:
        Channel(std::string id);
        const std::string mId;
        
        int sync(OutBuffer& buffer, uint16_t indent);
        
        void setEnabled(bool enabled);
        
        void setName(std::string name);
        std::string getName();
    private:
        std::string mName;
        bool mEnabled;
    };

    extern Channel* const VERBOSE;
    extern Channel* const INFO;
    extern Channel* const WARN;
    extern Channel* const SEVERE;
    extern Channel* const ADDON;

    Out log(std::string id);
    Out log(Channel* channel);
    Channel* getChannel(std::string id);

} // Logger
} // pgg

#endif // PEGR_ENGINELOGGER_HPP
