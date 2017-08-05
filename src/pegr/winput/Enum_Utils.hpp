#ifndef PEGR_WINPUT_ENUMUTILS_HPP
#define PEGR_WINPUT_ENUMUTILS_HPP

#include <SDL.h>
#include <SDL_syswm.h>

namespace pegr {
namespace Winput {
namespace Util {

const char* to_string_syswm_type(SDL_SYSWM_TYPE t);
    
} // namespace Util
} // namespace Winput
} // namespace pegr

#endif // PEGR_WINPUT_ENUMUTILS_HPP
