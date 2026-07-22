#include "arpg/time/monotonic_clock.hpp"

namespace arpg::time {

auto SteadyClock::now() noexcept -> MonotonicTime {
    static_assert(std::chrono::steady_clock::is_steady);
    return std::chrono::duration_cast<MonotonicTime>(std::chrono::steady_clock::now().time_since_epoch());
}

} // namespace arpg::time
