#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    T* p_;
    ControlBlockBase* block_;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() { WeakPtr().Swap(*this); }
    void Swap(WeakPtr& other) {
        std::swap(p_, other.p_);
        std::swap(block_, other.block_);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() noexcept : p_(nullptr), block_(nullptr) {}

    WeakPtr(const WeakPtr& other) : p_(other.p_), block_(other.block_) {
        if (block_) {
            block_->IncrementWeak();
        }
    }
    WeakPtr(WeakPtr&& other) : p_(other.p_), block_(other.block_) {
        other.p_ = nullptr;
        other.block_ = nullptr;
    }
    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) : p_(other.p_), block_(other.block_) {
        if (block_) {
            block_->IncrementWeak();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        WeakPtr(other).Swap(*this);
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) {
        WeakPtr(std::move(other)).Swap(*this);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        if (block_) {
            block_->DecrementWeak();
            if (block_->strong == 0 && block_->weak == 0) {
                block_->OnZeroWeak();
            }
        }
        p_ = nullptr;
        block_ = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const { return block_ ? block_->strong : 0;}
    bool Expired() const {return block_ == nullptr || block_->strong == 0;}
    SharedPtr<T> Lock() const {
        if(Expired()) {
            return SharedPtr<T>();
        }
        return SharedPtr<T>(*this);
    }
};
