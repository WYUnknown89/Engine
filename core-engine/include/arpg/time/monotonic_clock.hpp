#pragma once

#include <chrono>

namespace arpg::time {

using MonotonicTime = std::chrono::duration<double>;

class IMonotonicClock {
  public:
    virtual ~IMonotonicClock() = default;

    [[nodiscard]] virtual auto now() noexcept -> MonotonicTime = 0;
};

class SteadyClock final : public IMonotonicClock {
  public:
    [[nodiscard]] auto now() noexcept -> MonotonicTime override;
};

} // namespace arpg::time
