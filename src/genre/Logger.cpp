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

#include "Logger.hpp"

#include <iostream>
#include <map>

namespace pegr {
namespace Logger {
    /*
    Out::Out()
    : mChannel(Logger::VERBOSE)
    , std::ostream(new OutBuffer(Logger::VERBOSE)) {
        mOutBuf = static_cast<OutBuffer*>(rdbuf());
    }
    */
    
    Out::Out(Channel* channel)
    : mChannel(channel)
    , std::ostream(new OutBuffer(channel)) {
        mOutBuf = static_cast<OutBuffer*>(rdbuf());
    }

    Out::Out(const Out& copyCtr)
    : mChannel(copyCtr.mChannel)
    , std::ostream(new OutBuffer(copyCtr.mChannel)) {
        mOutBuf = static_cast<OutBuffer*>(rdbuf());
    }
    
    /*
    Out::operator=(const Out& assignment) {
        if(mChannel != assignment.mChannel) {
            mChannel = assignment.mChannel;
            delete mOutBuf;
            rdbuf(new OutBuffer(mChannel));
            mOutBuf = static_cast<OutBuffer*>(rdbuf());
        }
    }
    */

    Out::~Out() {
        delete mOutBuf;
    }
    
    void Out::indent() {
        ++ mOutBuf->mIndent;
    }
    
    void Out::unindent() {
        if(mOutBuf->mIndent > 0) -- mOutBuf->mIndent;
    }

    OutBuffer::OutBuffer(Channel* channel)
    : mIndent(0)
    , mChannel(channel) { }

    int OutBuffer::sync() {
        return mChannel->sync(*this, mIndent);
    }

    Channel::Channel(std::string id)
    : mId(id)
    , mName(id)
    , mEnabled(true) { }

    int Channel::sync(OutBuffer& buffer, uint16_t indent) {
        if(mEnabled) std::cout << mName << '\t' << std::string(indent * 2, ' ') << buffer.str();
        buffer.str("");
        return std::cout ? 0 : -1;
    }
    
    void Channel::setEnabled(bool enabled) { mEnabled = enabled; }

    void Channel::setName(std::string name) { mName = name; }
    std::string Channel::getName() { return mName; }

    std::map<std::string, Channel*> sChannels;
    Channel* getChannel(std::string id) {
        std::map<std::string, Channel*>::iterator iter = sChannels.find(id);
        if(iter != sChannels.end()) {
            return iter->second;
        } else {
            Channel* newChannel = new Channel(id);
            sChannels[id] = newChannel;
            return newChannel;
        }
    }

    Out log(std::string channel) {
        return Out(getChannel(channel));
    }
    Out log(Channel* channel) {
        return Out(channel);
    }

    extern Channel* const VERBOSE = new Channel("VERB");
    extern Channel* const INFO = new Channel("INFO");
    extern Channel* const WARN = new Channel("WARN!");
    extern Channel* const SEVERE = new Channel("SEVR!!");
    extern Channel* const ADDON = new Channel("ADON");

} // Logger
} // pgg
