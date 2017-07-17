#include "pegr/gensys/Interm_Types.hpp"

#include <cassert>

#include "pegr/logger/Logger.hpp"

namespace pegr {
namespace Gensys {
namespace Interm {

void Prim::set_type(Type type) {
    if (m_type != type) {
        deconstruct_current();
    }
    m_type = type;
}
Prim::Type Prim::get_type() const {
    return m_type;
}

bool Prim::is_error() const {
    return m_type == Type::UNKNOWN;
}

const std::string& Prim::get_string() const {
    assert(!m_empty);
    assert(m_type == Type::STR);
    return m_str;
}
const Script::Regref_Shared& Prim::get_function() const {
    assert(!m_empty);
    assert(m_type == Type::FUNC);
    return m_func;
}

float Prim::get_f32() const {
    assert(!m_empty);
    assert(m_type == Type::F32);
    return m_f32;
}
double Prim::get_f64() const {
    assert(!m_empty);
    assert(m_type == Type::F64);
    return m_f64;
}
int32_t Prim::get_i32() const {
    assert(!m_empty);
    assert(m_type == Type::I32);
    return m_i32;
}
int64_t Prim::get_i64() const {
    assert(!m_empty);
    assert(m_type == Type::I64);
    return m_i64;
}

Prim::Prim()
: m_type(Type::UNKNOWN)
, m_empty(true) { }

Prim::Prim(const Prim& other_p)
: m_type(Type::UNKNOWN)
, m_empty(true) {
    copy_from(other_p);
}
Prim::Prim(Prim&& other_p)
: m_type(Type::UNKNOWN)
, m_empty(true) {
    move_from(std::move(other_p));
}
Prim& Prim::operator =(const Prim& other_p) {
    deconstruct_current();
    copy_from(other_p);
    return *this;
}
Prim& Prim::operator =(Prim&& other_p) {
    deconstruct_current();
    move_from(std::move(other_p));
    return *this;
}
Prim::~Prim() {
    deconstruct_current();
}
    
void Prim::copy_from(const Prim& other_p) {
    set_type(other_p.m_type);
    switch (other_p.m_type) {
        case Type::F32: {
            m_f32 = other_p.m_f32;
            break;
        }
        case Type::F64: {
            m_f64 = other_p.m_f64;
            break;
        }
        case Type::I32: {
            m_i32 = other_p.m_i32;
            break;
        }
        case Type::I64: {
            m_i64 = other_p.m_i64;
            break;
        }
        case Type::FUNC: {
            m_func = other_p.m_func;
            break;
        }
        case Type::STR: {
            m_str = other_p.m_str;
            break;
        }
    }
    m_empty = other_p.m_empty;
}
void Prim::move_from(Prim&& other_p) {
    set_type(other_p.m_type);
    switch (other_p.m_type) {
        case Type::F32: {
            m_f32 = other_p.m_f32;
            break;
        }
        case Type::F64: {
            m_f64 = other_p.m_f64;
            break;
        }
        case Type::I32: {
            m_i32 = other_p.m_i32;
            break;
        }
        case Type::I64: {
            m_i64 = other_p.m_i64;
            break;
        }
        case Type::FUNC: {
            std::swap(m_func, other_p.m_func);
            break;
        }
        case Type::STR: {
            std::swap(m_str, other_p.m_str);
            break;
        }
    }
    m_empty = other_p.m_empty;
}
void Prim::deconstruct_current() {
    switch (m_type) {
        case Type::FUNC: {
            m_func.reset();
            break;
        }
        case Type::STR: {
            m_str.clear();
            break;
        }
        default: {
            break;
        }
    }
    m_type = Type::UNKNOWN;
    m_empty = true;
}

void Prim::set_string(std::string str) {
    set_type(Type::STR);
    m_str = str;
    m_empty = false;
}
void Prim::set_function(Script::Regref_Shared func) {
    set_type(Type::FUNC);
    m_func = func;
    m_empty = false;
}
void Prim::set_f32(float f32) {
    set_type(Type::F32);
    m_f32 = f32;
    m_empty = false;
}
void Prim::set_f64(double f64) {
    set_type(Type::F64);
    m_f64 = f64;
    m_empty = false;
}
void Prim::set_i32(int32_t i32) {
    set_type(Type::I32);
    m_i32 = i32;
    m_empty = false;
}
void Prim::set_i64(int64_t i64) {
    set_type(Type::I64);
    m_i64 = i64;
    m_empty = false;
}
bool Prim::is_empty() const {
    return m_empty;
}
void Prim::set_empty() {
    m_empty = true;
}

const char* prim_type_to_debug_str(Prim::Type t) {
    switch(t) {
        case Prim::Type::F32:
            return "F32";
        case Prim::Type::F64:
            return "F64";
        case Prim::Type::I32:
            return "I32";
        case Prim::Type::I64:
            return "I64";
        case Prim::Type::STR:
            return "STR";
        case Prim::Type::FUNC:
            return "FUNC";
        case Prim::Type::UNKNOWN:
            return "UNKNOWN";
        default:
            assert(false);
            return "ERROR";
    }
}

} // namespace Interm
} // namespace Gensys
} // namespace pegr
