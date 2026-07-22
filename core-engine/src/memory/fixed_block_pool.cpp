#include "arpg/memory/fixed_block_pool.hpp"

#include "arpg/memory/detail/checked_arithmetic.hpp"

#include <algorithm>
#include <cstdint>
#include <new>
#include <stdexcept>
#include <utility>

namespace arpg::memory {

FixedBlockPool::FixedBlockPool(const std::size_t block_size, const std::size_t block_alignment,
                               const std::size_t block_count)
    : block_size_(block_size), block_alignment_(block_alignment), block_count_(block_count) {
    if (block_size_ == 0U || block_count_ == 0U) {
        throw std::invalid_argument{"FixedBlockPool block size and count must be non-zero"};
    }
    if (!detail::is_power_of_two(block_alignment_)) {
        throw std::invalid_argument{"FixedBlockPool alignment must be a power of two"};
    }
    if (!detail::checked_align_up(block_size_, block_alignment_, stride_) ||
        !detail::checked_multiply(stride_, block_count_, backing_bytes_)) {
        throw std::overflow_error{"FixedBlockPool size calculation overflowed"};
    }

    slots_ = std::make_unique<Slot[]>(block_count_);
    try {
        backing_ = static_cast<std::byte*>(::operator new(backing_bytes_, std::align_val_t{block_alignment_}));
    } catch (...) {
        slots_.reset();
        throw;
    }
    statistics_.block_size = block_size_;
    statistics_.block_alignment = block_alignment_;
    statistics_.block_capacity = block_count_;
    initialize_free_list();
}

FixedBlockPool::~FixedBlockPool() { release_backing(); }

FixedBlockPool::FixedBlockPool(FixedBlockPool&& other) noexcept { move_from(other); }

auto FixedBlockPool::operator=(FixedBlockPool&& other) noexcept -> FixedBlockPool& {
    if (this != &other) {
        release_backing();
        move_from(other);
    }
    return *this;
}

auto FixedBlockPool::try_allocate() noexcept -> AllocationResult {
    if (!has_backing()) {
        return {.error = AllocationError::allocator_unavailable};
    }
    if (free_head_ == no_slot) {
        record_failed_allocation();
        return {.error = AllocationError::exhausted};
    }

    const auto index = free_head_;
    Slot& slot = slots_[index];
    free_head_ = slot.next;
    slot.allocated = true;
    statistics_.live_blocks += 1U;
    statistics_.peak_live_blocks = std::max(statistics_.peak_live_blocks, statistics_.live_blocks);
    detail::saturating_increment(statistics_.successful_allocations);
    return {.address = backing_ + index * stride_};
}

auto FixedBlockPool::release(void* const address) noexcept -> PoolReleaseResult {
    if (!has_backing()) {
        return PoolReleaseResult::allocator_unavailable;
    }
    if (address == nullptr) {
        record_invalid_release();
        return PoolReleaseResult::null_pointer;
    }

    const auto base = reinterpret_cast<std::uintptr_t>(backing_);
    const auto candidate = reinterpret_cast<std::uintptr_t>(address);
    if (candidate < base) {
        record_invalid_release();
        return PoolReleaseResult::foreign_pointer;
    }
    const auto offset = static_cast<std::size_t>(candidate - base);
    if (offset >= backing_bytes_) {
        record_invalid_release();
        return PoolReleaseResult::foreign_pointer;
    }
    if (offset % stride_ != 0U) {
        record_invalid_release();
        return PoolReleaseResult::interior_pointer;
    }

    const auto index = offset / stride_;
    Slot& slot = slots_[index];
    if (!slot.allocated) {
        record_invalid_release();
        return PoolReleaseResult::double_free;
    }
    slot.allocated = false;
    slot.next = free_head_;
    free_head_ = index;
    statistics_.live_blocks -= 1U;
    detail::saturating_increment(statistics_.successful_releases);
    return PoolReleaseResult::released;
}

void FixedBlockPool::reset() noexcept {
    if (!has_backing()) {
        return;
    }
    initialize_free_list();
    statistics_.live_blocks = 0U;
    detail::saturating_increment(statistics_.reset_count);
}

auto FixedBlockPool::has_backing() const noexcept -> bool { return backing_ != nullptr; }

auto FixedBlockPool::statistics() const noexcept -> PoolStatistics { return statistics_; }

void FixedBlockPool::release_backing() noexcept {
    if (backing_ != nullptr) {
        ::operator delete(backing_, std::align_val_t{block_alignment_});
    }
    backing_ = nullptr;
    slots_.reset();
    backing_bytes_ = 0U;
    block_size_ = 0U;
    block_alignment_ = 0U;
    stride_ = 0U;
    block_count_ = 0U;
    free_head_ = no_slot;
    statistics_ = {};
}

void FixedBlockPool::move_from(FixedBlockPool& other) noexcept {
    backing_ = std::exchange(other.backing_, nullptr);
    slots_ = std::move(other.slots_);
    backing_bytes_ = std::exchange(other.backing_bytes_, 0U);
    block_size_ = std::exchange(other.block_size_, 0U);
    block_alignment_ = std::exchange(other.block_alignment_, 0U);
    stride_ = std::exchange(other.stride_, 0U);
    block_count_ = std::exchange(other.block_count_, 0U);
    free_head_ = std::exchange(other.free_head_, no_slot);
    statistics_ = std::exchange(other.statistics_, {});
}

void FixedBlockPool::initialize_free_list() noexcept {
    for (std::size_t index = 0U; index < block_count_; ++index) {
        slots_[index].allocated = false;
        slots_[index].next = index + 1U < block_count_ ? index + 1U : no_slot;
    }
    free_head_ = 0U;
}

void FixedBlockPool::record_invalid_release() noexcept { detail::saturating_increment(statistics_.invalid_releases); }

void FixedBlockPool::record_failed_allocation() noexcept {
    detail::saturating_increment(statistics_.failed_allocations);
}

} // namespace arpg::memory
