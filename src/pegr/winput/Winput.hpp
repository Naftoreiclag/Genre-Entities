#ifndef PEGR_WINPUT_WINPUT_HPP
#define PEGR_WINPUT_WINPUT_HPP

#include <cstdint>

namespace pegr {
namespace Winput {

extern const char* const WINDOW_DEFAULT_TITLE;
extern const int32_t WINDOW_DEFAULT_WIDTH;
extern const int32_t WINDOW_DEFAULT_HEIGHT;

void initialize();
void pollEvents();
void cleanup();

int32_t get_window_width();
int32_t get_window_height();

} // namespace Winput
} // namespace pegr

#endif // PEGR_WINPUT_WINPUT_HPP
