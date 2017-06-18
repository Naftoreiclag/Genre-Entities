#ifndef PEGR_TEST_MACROS_HPP
#define PEGR_TEST_MACROS_HPP

#include "pegr/logger/Logger.hpp"

#define test_assert(b, msg) \
    if (!b) { pegr::Logger::log()->warn(msg); return false; }

#endif // PEGR_TEST_MACROS_HPP
