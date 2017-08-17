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

#include "pegr/engine/Engine.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "pegr/engine/App_State_Machine.hpp"
#include "pegr/except/Except.hpp"
#include "pegr/gensys/Gensys.hpp"
#include "pegr/gensys/Lua_Interf.hpp"
#include "pegr/logger/Logger.hpp"
#include "pegr/resource/Resources.hpp"
#include "pegr/scheduler/Lua_Interf.hpp"
#include "pegr/script/Script.hpp"
#include "pegr/winput/Winput.hpp"

namespace pegr {
namespace Engine {
    
typedef std::chrono::time_point<std::chrono::steady_clock> Steady_Time_Point;
boost::asio::io_service n_io;
Steady_Time_Point n_last_tick_timestamp;

int64_t n_tick_period_ns = 1e6;
int32_t n_tick_freq = 1;
uint64_t n_tick_id = 0;
double n_tick_lag = 0;

const uint16_t INIT_FLAG_LOGGER = 0x0001;
const uint16_t INIT_FLAG_SCRIPT = 0x0002 | INIT_FLAG_LOGGER;
const uint16_t INIT_FLAG_GENSYS = 0x0004 | INIT_FLAG_SCRIPT;
const uint16_t INIT_FLAG_SCHEDU = 0x0008 | INIT_FLAG_SCRIPT;
const uint16_t INIT_FLAG_WINPUT = 0x0010 | INIT_FLAG_LOGGER;
const uint16_t INIT_FLAG_RESOUR = 0x0020 | INIT_FLAG_LOGGER;
const uint16_t INIT_FLAG_ALL = 0xFFFF;
const uint16_t INIT_FLAG_NONE = 0x0000;

uint16_t n_flags;

bool logger_used() {
    return (n_flags & INIT_FLAG_LOGGER) == INIT_FLAG_LOGGER;
}
bool script_used() {
    return (n_flags & INIT_FLAG_SCRIPT) == INIT_FLAG_SCRIPT;
}
bool gensys_used() {
    return (n_flags & INIT_FLAG_GENSYS) == INIT_FLAG_GENSYS;
}
bool schedu_used() {
    return (n_flags & INIT_FLAG_SCHEDU) == INIT_FLAG_SCHEDU;
}
bool winput_used() {
    return (n_flags & INIT_FLAG_WINPUT) == INIT_FLAG_WINPUT;
}
bool resour_used() {
    return (n_flags & INIT_FLAG_RESOUR) == INIT_FLAG_RESOUR;
}

App_State_Machine n_asm;

void initialize(uint16_t flags) {
    n_flags = flags;
    
    if (logger_used()) {
        Logger::initialize();
        Logger::log()->info("Logger: %v", logger_used());
        Logger::log()->info("Script: %v", script_used());
        Logger::log()->info("Gensys: %v", gensys_used());
        Logger::log()->info("Scheduler: %v", schedu_used());
        Logger::log()->info("Window/Input: %v", winput_used());
        Logger::log()->info("Resources: %v", resour_used());
    }
    
    if (script_used()) {
        try {
            Script::initialize();
        } catch (Except::Runtime& e) {
            std::stringstream sss;
            sss << "Error while initializing scripting: "
                << e.what();
            throw Except::Runtime(sss.str());
        }
    }
    
    if (gensys_used()) {
        try {
            Gensys::initialize();
            Gensys::LI::initialize();
        } catch (Except::Runtime& e) {
            std::stringstream sss;
            sss << "Error while initializing gensys: "
                << e.what();
            throw Except::Runtime(sss.str());
        }
    }
    
    if (schedu_used()) {
        try {
            Schedu::LI::initialize();
        } catch (Except::Runtime& e) {
            std::stringstream sss;
            sss << "Error while initializing scheduler: "
                << e.what();
            throw Except::Runtime(sss.str());
        }
    }
    
    if (winput_used()) {
        try {
            Winput::initialize();
        } catch (Except::Runtime& e) {
            std::stringstream sss;
            sss << "Error while initializing window/input: "
                << e.what();
            throw Except::Runtime(sss.str());
        }
    }
    
    if (resour_used()) {
        try {
            Resour::initialize();
        } catch (Except::Runtime& e) {
            std::stringstream sss;
            sss << "Error while initializing resources: "
                << e.what();
            throw Except::Runtime(sss.str());
        }
    }
}

void push_state(std::unique_ptr<App_State>&& unique_state) {
    n_asm.push_state(std::move(unique_state));
}

std::unique_ptr<App_State> pop_state() {
    return n_asm.pop_state();
}

std::unique_ptr<App_State> swap_state(std::unique_ptr<App_State>&& u_state) {
    return n_asm.swap_state(std::move(u_state));
}

void on_window_resize(int32_t width, int32_t height) {
    n_asm->on_window_resize(width, height);
}

void reset_lag_time() {
    n_last_tick_timestamp = std::chrono::steady_clock::now();
    n_tick_lag = 0;
}

void update_lag_time() {
    std::chrono::duration<double> diff = 
            std::chrono::steady_clock::now() - n_last_tick_timestamp;
    double newlag = diff.count() * n_tick_freq;
    if (newlag < n_tick_lag) return;
    if (newlag < 0) {
        newlag = 0;
    }
    if (newlag > 1) {
        newlag = 1;
    }
    n_tick_lag = newlag;
}

void async_max_speed(const boost::system::error_code& asio_err,
        boost::asio::deadline_timer* timer) {
    update_lag_time();
    if (winput_used()) {
        Winput::pollEvents();
    }
    
    if (is_main_loop_running()) {
        timer->async_wait(boost::bind(
                async_max_speed, boost::asio::placeholders::error,
                timer));
    }
}

void async_tick(const boost::system::error_code& asio_err,
        boost::asio::deadline_timer* timer) {
    reset_lag_time();
    n_asm->do_tick();
    ++n_tick_id;
    if (is_main_loop_running()) {
        timer->expires_at(timer->expires_at() + 
                boost::posix_time::microseconds(n_tick_period_ns));
        timer->async_wait(boost::bind(
                async_tick, boost::asio::placeholders::error,
                timer));
    }
}

void async_render(const boost::system::error_code& asio_err,
        boost::asio::deadline_timer* timer) {
    
    update_lag_time();
    n_asm->do_frame();
    if (winput_used()) {
        Winput::submit_frame();
    }
    
    if (is_main_loop_running()) {
        timer->async_wait(boost::bind(
                async_render, boost::asio::placeholders::error,
                timer));
    }
}

void run() {
    boost::asio::deadline_timer tick_timer(
            n_io, boost::posix_time::seconds(0));
    boost::asio::deadline_timer render_timer(
            n_io, boost::posix_time::seconds(0));
    boost::asio::deadline_timer max_speed_timer(
            n_io, boost::posix_time::seconds(0));
    tick_timer.async_wait(boost::bind(
            async_tick, boost::asio::placeholders::error,
            &tick_timer));
    render_timer.async_wait(boost::bind(
            async_render, boost::asio::placeholders::error,
            &render_timer));
    max_speed_timer.async_wait(boost::bind(
            async_max_speed, boost::asio::placeholders::error,
            &max_speed_timer));
    n_last_tick_timestamp = std::chrono::steady_clock::now();
    n_io.run();
}

void cleanup() {
    n_asm.clear_all();
    n_tick_id = 0;
    
    if (resour_used()) {
        Resour::cleanup();
    }
    
    if (winput_used()) {
        Winput::cleanup();
    }
    
    if (schedu_used()) {
        Schedu::LI::cleanup();
    }
    
    if (gensys_used()) {
        Gensys::LI::cleanup();
        Gensys::cleanup();
    }
    
    if (script_used()) {
        Script::cleanup();
    }
    
    if (logger_used()) {
        Logger::cleanup();
    }
}

void set_tick_frequency(int32_t hertz) {
    assert(hertz > 0);
    n_tick_freq = hertz;
    // TODO: more precision
    n_tick_period_ns = 1e6d / ((double) hertz);
}

uint64_t get_tick_id() {
    return n_tick_id;
}
double get_tick_lag() {
    return n_tick_lag;
}

bool is_main_loop_running() {
    return n_asm.has_active();
}

void quit() {
    n_asm.clear_all();
    n_io.stop();
}

} // namespace Engine
} // namespace pegr
