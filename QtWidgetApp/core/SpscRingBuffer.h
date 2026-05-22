#pragma once
#include <atomic>
#include <cstddef>
#include <new>
#include <type_traits>

// Lock-free Single-Producer Single-Consumer ring buffer
// Overwrite-old mode: when full, newest data overwrites oldest
template<typename T, size_t Capacity>
class SpscRingBuffer {
    static_assert(Capacity >= 2, "Capacity must be >= 2");
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

public:
    SpscRingBuffer() : m_readIdx(0), m_writeIdx(0) {}

    // Producer: push data (overwrites oldest if full)
    void push(const T& item) {
        const size_t write = m_writeIdx.load(std::memory_order_relaxed);
        m_buffer[write & kMask] = item;
        m_writeIdx.store(write + 1, std::memory_order_release);

        // If buffer is full, advance read pointer (discard oldest)
        size_t read = m_readIdx.load(std::memory_order_relaxed);
        if ((write + 1 - read) > Capacity) {
            m_readIdx.store(write + 1 - Capacity, std::memory_order_relaxed);
        }
    }

    // Consumer: pop newest data (skips older entries if multiple available)
    bool pop(T& item) {
        size_t read = m_readIdx.load(std::memory_order_relaxed);
        const size_t write = m_writeIdx.load(std::memory_order_acquire);

        if (read >= write) {
            return false; // empty
        }

        // Skip to newest: read from write-1
        item = m_buffer[(write - 1) & kMask];
        m_readIdx.store(write, std::memory_order_relaxed);
        return true;
    }

    // Consumer: pop oldest data (FIFO order)
    bool popOldest(T& item) {
        const size_t read = m_readIdx.load(std::memory_order_relaxed);
        const size_t write = m_writeIdx.load(std::memory_order_acquire);

        if (read >= write) {
            return false;
        }

        item = m_buffer[read & kMask];
        m_readIdx.store(read + 1, std::memory_order_release);
        return true;
    }

    size_t size() const {
        const size_t write = m_writeIdx.load(std::memory_order_relaxed);
        const size_t read = m_readIdx.load(std::memory_order_relaxed);
        return write - read;
    }

    bool empty() const {
        return size() == 0;
    }

    void reset() {
        m_writeIdx.store(0, std::memory_order_relaxed);
        m_readIdx.store(0, std::memory_order_relaxed);
    }

private:
    static constexpr size_t kMask = Capacity - 1;

    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> m_readIdx;
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> m_writeIdx;
    T m_buffer[Capacity];
};
