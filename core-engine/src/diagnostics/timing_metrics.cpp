#include "arpg/diagnostics/timing_metrics.hpp"

#include <algorithm>
#include <limits>

namespace arpg::diagnostics {

auto TimingAccumulator::record(const std::chrono::nanoseconds sample) noexcept -> bool {
    if (sample.count() < 0) {
        if (summary_.rejected_sample_count != std::numeric_limits<std::uint64_t>::max()) {
            ++summary_.rejected_sample_count;
        }
        return false;
    }
    if (summary_.sample_count == 0U) {
        summary_.minimum = sample;
        summary_.maximum = sample;
    } else {
        summary_.minimum = std::min(summary_.minimum, sample);
        summary_.maximum = std::max(summary_.maximum, sample);
    }
    summary_.current = sample;
    if (sample.count() > std::numeric_limits<std::int64_t>::max() - summary_.total.count()) {
        summary_.total = std::chrono::nanoseconds{std::numeric_limits<std::int64_t>::max()};
        summary_.total_overflowed = true;
    } else {
        summary_.total += sample;
    }
    if (summary_.sample_count != std::numeric_limits<std::uint64_t>::max()) {
        ++summary_.sample_count;
    }
    summary_.average =
        std::chrono::nanoseconds{summary_.total.count() / static_cast<std::int64_t>(summary_.sample_count)};
    return true;
}

void TimingAccumulator::reset() noexcept { summary_ = {}; }

auto TimingAccumulator::summary() const noexcept -> TimingSummary { return summary_; }

} // namespace arpg::diagnostics
