#include "pegr/gensys/GensysIntermediate.hpp"

#include <cassert>

#include "pegr/logger/Logger.hpp"

namespace pegr {
namespace Gensys {
namespace Interm {

void Prim::set_type(Type type) { }
Prim::Type Prim::get_type() const {
    return m_type;
}

bool Prim::is_error() const {
    return m_type == Type::ERROR;
}

const std::string& Prim::get_string() const {
    assert(m_type == Type::STR);
    return m_str;
}
const Script::Regref_Guard& Prim::get_function() const {
    assert(m_type == Type::FUNC);
    return m_func;
}

float Prim::get_f32() const {
    assert(m_type == Type::F32);
    return m_f32;
}
double Prim::get_f64() const {
    assert(m_type == Type::F64);
    return m_f64;
}
int32_t Prim::get_i32() const {
    assert(m_type == Type::I32);
    return m_i32;
}
int64_t Prim::get_i64() const {
    assert(m_type == Type::I64);
    return m_i64;
}

Prim::Prim()
: m_type(Type::ERROR) { }

Prim::Prim(const Prim& other_p) {
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
        default: {
            deconstruct_current();
            switch (other_p.m_type) {
                case Type::FUNC: {
                    Script::push_reference(other_p.m_func);
                    m_func = Script::Regref_Guard(Script::grab_reference());
                    break;
                }
                case Type::STR: {
                    m_str = other_p.m_str;
                    break;
                }
            }
            break;
        }
    }
}

Prim::Prim(Prim&& other) {
}
Prim& Prim::operator =(const Prim& other) {
    return *this;
}
Prim& Prim::operator =(Prim&& other) {
    return *this;
}
Prim::~Prim() {
    
}
    
void Prim::deconstruct_current() {
    switch (m_type) {
        case Type::FUNC: {
            m_func.~Regref_Guard();
            break;
        }
        case Type::STR: {
            m_str.~basic_string();
            break;
        }
        default: {
            break;
        }
    }
    m_type = Type::ERROR;
}

void Prim::set_string(std::string str) {
    
}
void Prim::set_function(Script::Regref func) {
    
}
void Prim::set_f32(float f32) {
    
}
void Prim::set_f64(double f64) {
    
}
void Prim::set_i32(int32_t i32) {
    
}
void Prim::set_i64(int64_t i64) {
    
}

} // namespace Interm
} // namespace Gensys
} // namespace pegr
