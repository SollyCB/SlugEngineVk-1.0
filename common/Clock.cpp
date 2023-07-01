#include "Clock.hpp"

namespace Sol {

static Clock sClock;
Clock *Clock::instance() { return &sClock; }

} // namespace Sol
