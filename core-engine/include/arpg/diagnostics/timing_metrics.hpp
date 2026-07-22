#pragma once

#include <chrono>
#include <cstdint>

namespace arpg::diagnostics {

struct TimingSummary {
    std::uint64_t sample_count{0U};
    std::uint64_t rejected_sample_count{0U};
    std::chrono::nanoseconds current{};
    std::chrono::nanoseconds minimum{};
    std::chrono::nanoseconds maximum{};
    std::chrono::nanoseconds total{};
    std::chrono::nanoseconds average{};
    bool total_overflowed{false};
};

class TimingAccumulator {
  public:
    [[nodiscard]] auto record(std::chrono::nanoseconds sample) noexcept -> bool;
    void reset() noexcept;
    [[nodiscard]] auto summary() const noexcept -> TimingSummary;

  private:
    TimingSummary summary_{};
};

} // namespace arpg::diagnostics
