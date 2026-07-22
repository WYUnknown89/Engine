#include "arpg/memory/linear_arena.hpp"

#include "arpg/memory/detail/checked_arithmetic.hpp"

#include <algorithm>
#include <new>
#include <stdexcept>
#include <utility>

namespace arpg::memory {

LinearArena::LinearArena(const LinearArenaConfig config)
    : capacity_bytes_(config.capacity_bytes), maximum_alignment_(config.maximum_alignment) {
    if (capacity_bytes_ == 0U) {
        throw std::invalid_argument{"LinearArena capacity must be non-zero"};
    }
    if (!detail::is_power_of_two(maximum_alignment_)) {
        throw std::invalid_argument{"LinearArena maximum alignment must be a power of two"};
    }

    backing_ = static_cast<std::byte*>(::operator new(capacity_bytes_, std::align_val_t{maximum_alignment_}));
    statistics_.capacity_bytes = capacity_bytes_;
}

LinearArena::~LinearArena() { release_backing(); }

LinearArena::LinearArena(LinearArena&& other) noexcept { move_from(other); }

auto LinearArena::operator=(LinearArena&& other) noexcept -> LinearArena& {
    if (this != &other) {
        release_backing();
        move_from(other);
    }
    return *this;
}

auto LinearArena::try_allocate(const std::size_t size, const std::size_t alignment) noexcept -> AllocationResult {
    if (!has_backing()) {
        return {.error = AllocationError::allocator_unavailable};
    }
    if (size == 0U) {
        record_failure();
        return {.error = AllocationError::zero_size};
    }
    if (!detail::is_power_of_two(alignment) || alignment > maximum_alignment_) {
        record_failure();
        return {.error = AllocationError::invalid_alignment};
    }

    std::size_t aligned_offset = 0U;
    std::size_t end_offset = 0U;
    if (!detail::checked_align_up(used_bytes_, alignment, aligned_offset) ||
        !detail::checked_add(aligned_offset, size, end_offset)) {
        record_failure();
        return {.error = AllocationError::size_overflow};
    }
    if (end_offset > capacity_bytes_) {
        record_failure();
        return {.error = AllocationError::exhausted};
    }

    auto* const address = backing_ + aligned_offset;
    used_bytes_ = end_offset;
    statistics_.used_bytes = used_bytes_;
    statistics_.peak_used_bytes = std::max(statistics_.peak_used_bytes, used_bytes_);
    detail::saturating_increment(statistics_.successful_allocations);
    return {.address = address};
}

void LinearArena::reset() noexcept {
    if (!has_backing()) {
        return;
    }
    used_bytes_ = 0U;
    statistics_.used_bytes = 0U;
    detail::saturating_increment(statistics_.reset_count);
}

auto LinearArena::has_backing() const noexcept -> bool { return backing_ != nullptr; }

auto LinearArena::statistics() const noexcept -> ArenaStatistics { return statistics_; }

void LinearArena::release_backing() noexcept {
    if (backing_ != nullptr) {
        ::operator delete(backing_, std::align_val_t{maximum_alignment_});
    }
    backing_ = nullptr;
    capacity_bytes_ = 0U;
    maximum_alignment_ = 0U;
    used_bytes_ = 0U;
    statistics_ = {};
}

void LinearArena::move_from(LinearArena& other) noexcept {
    backing_ = std::exchange(other.backing_, nullptr);
    capacity_bytes_ = std::exchange(other.capacity_bytes_, 0U);
    maximum_alignment_ = std::exchange(other.maximum_alignment_, 0U);
    used_bytes_ = std::exchange(other.used_bytes_, 0U);
    statistics_ = std::exchange(other.statistics_, {});
}

void LinearArena::record_failure() noexcept { detail::saturating_increment(statistics_.failed_allocations); }

} // namespace arpg::memory
