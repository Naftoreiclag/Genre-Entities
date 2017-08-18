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

#include "pegr/engine/App_State_Machine.hpp"
#include "pegr/test/Test_Util.hpp"

namespace pegr {
namespace Test {
    
namespace ASMT {
    
class Weird_State : public Engine::App_State {
public:
    Weird_State(std::vector<std::string>& msgs)
    : m_msgs(msgs)
    , Engine::App_State("Weird") {}
    virtual ~Weird_State() {}
    
    virtual void initialize() override {
        m_msgs.push_back("Wi");
    }
    virtual void pause(App_State* to) override {
        m_msgs.push_back("Wp");
    }
    virtual void unpause(App_State* from) override {
        m_msgs.push_back("Wu");
    }
    virtual void cleanup() override {
        m_msgs.push_back("Wc");
    }
private:
    std::vector<std::string>& m_msgs;
};
    
class Macbeth_State : public Engine::App_State {
public:
    Macbeth_State(std::vector<std::string>& msgs)
    : m_msgs(msgs)
    , Engine::App_State("Macbeth") {}
    virtual ~Macbeth_State() {}
    
    virtual void initialize() override {
        m_msgs.push_back("Mi");
    }
    virtual void pause(App_State* to) override {
        m_msgs.push_back("Mp");
    }
    virtual void unpause(App_State* from) override {
        m_msgs.push_back("Mu");
    }
    virtual void cleanup() override {
        m_msgs.push_back("Mc");
    }
private:
    std::vector<std::string>& m_msgs;
};

class Duncan_State : public Engine::App_State {
public:
    Duncan_State(std::vector<std::string>& msgs)
    : m_msgs(msgs)
    , Engine::App_State("Duncan") {}
    virtual ~Duncan_State() {}
    
    virtual void initialize() override {
        m_msgs.push_back("Di");
    }
    virtual void pause(App_State* to) override {
        m_msgs.push_back("Dp");
    }
    virtual void unpause(App_State* from) override {
        m_msgs.push_back("Du");
    }
    virtual void cleanup() override {
        m_msgs.push_back("Dc");
    }
private:
    std::vector<std::string>& m_msgs;
};

class Banquo_State : public Engine::App_State {
public:
    Banquo_State(std::vector<std::string>& msgs)
    : m_msgs(msgs)
    , Engine::App_State("Banquo") {}
    virtual ~Banquo_State() {}
    
    virtual void initialize() override {
        m_msgs.push_back("Bi");
    }
    virtual void pause(App_State* to) override {
        m_msgs.push_back("Bp");
    }
    virtual void unpause(App_State* from) override {
        m_msgs.push_back("Bu");
    }
    virtual void cleanup() override {
        m_msgs.push_back("Bc");
    }
private:
    std::vector<std::string>& m_msgs;
};

};

//@Test App State Machine Test
void test_0002_app_state_machine_test() {
    std::vector<std::string> msgs;
    
    Engine::App_State_Machine app_sm;
    verify_equals(0, app_sm.count(), "Nothing should be active");
    
    app_sm.push_state(std::make_unique<ASMT::Weird_State>(msgs));
    app_sm.push_state(std::make_unique<ASMT::Macbeth_State>(msgs));
    app_sm.push_state(std::make_unique<ASMT::Duncan_State>(msgs));
    app_sm.pop_state();
    app_sm.swap_state(std::make_unique<ASMT::Banquo_State>(msgs));
    app_sm.clear_all();
    
    std::vector<std::string> expected_msgs = {
        "Wi",
        "Wp",
        "Mi",
        "Mp",
        "Di",
        "Dc",
        "Mu",
        "Mc",
        "Bi",
        "Bc",
        "Wu",
        "Wc"
    };
    
    verify_equals(expected_msgs, msgs, "Incorrect output!");
}

} // namespace Test
} // namespace pegr
