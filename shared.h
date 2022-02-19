#pragma once

#include "sw_fwd.h"  // Forward declaration
#include <iostream>

#include <cstddef>  // std::nullptr_t

template <typename T>
struct ControlBlockMakeShared: ControlBlockBase {
public:
    std::aligned_storage_t<sizeof(T), alignof(T)> storage;

    ~ControlBlockMakeShared() override = default;
    template <typename ...Args>
    ControlBlockMakeShared(Args&& ...args) {
        new (&storage) T(std::forward<Args>(args)...);
    }

    void OnZeroStrong() override {
        reinterpret_cast<T*>(&storage)->~T();
    };
    void OnZeroWeak() override {
        delete this;
    };

    T* GetRawPtr() {
        return reinterpret_cast<T*>(&storage);
    }
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    T* p_;
    ControlBlockBase* block_;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers
    void Swap(SharedPtr& other) {
        std::swap(p_, other.p_);
        std::swap(block_, other.block_);
    }
    void Reset() { SharedPtr().Swap(*this); }
    template<class Y>
    void Reset(Y* ptr) { SharedPtr<T>(ptr).Swap(*this); }
    void Reset(T* ptr) { SharedPtr(ptr).Swap(*this); }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    SharedPtr() noexcept : p_(nullptr), block_(nullptr) {}
    SharedPtr(std::nullptr_t) noexcept : p_(nullptr), block_(nullptr) {}
    template<class Y>
    explicit SharedPtr(Y* ptr) : p_(ptr), block_(new ControlBlockPointer<Y>(ptr)) {
    }
    explicit SharedPtr(T* ptr) : p_(ptr), block_(new ControlBlockPointer<T>(ptr)) {
    }
    SharedPtr(T* ptr, ControlBlockBase* block) : p_(ptr), block_(block) {}
    template<class Y>
    SharedPtr(const SharedPtr<Y>& other) : p_(other.p_), block_(other.block_) {
        if (block_) {
            block_->IncrementStrong();
        }
    }
    SharedPtr(const SharedPtr& other) : p_(other.p_), block_(other.block_) {
        if (block_) {
            block_->IncrementStrong();
        }
    }
    template<class Y>
    SharedPtr(SharedPtr<Y>&& other) : p_(other.p_), block_(other.block_) {
        other.p_ = nullptr;
        other.block_ = nullptr;
    }
    SharedPtr(SharedPtr&& other) : p_(other.p_), block_(other.block_) {
        other.p_ = nullptr;
        other.block_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template<typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : p_(ptr), block_(other.block_) {
        if (block_) {
            block_->IncrementStrong();
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        SharedPtr(other).Swap(*this);
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        SharedPtr(std::move(other)).Swap(*this);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        if (block_) {
            block_->DecrementStrong();
            if (block_->strong == 0) {
                block_->OnZeroStrong();
                delete block_;
            }
        }
        p_ = nullptr;
        block_ = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const { return p_; }
    typename std::add_lvalue_reference<T>::type
    operator*() const {return *p_;}
    T* operator->() const { return p_; }
    size_t UseCount() const { return block_ ? block_->strong : 0;}
    explicit operator bool() const { return Get() != nullptr; }
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right);

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    ControlBlockMakeShared<T>* block = new ControlBlockMakeShared<T>(std::forward<decltype(args)>(args)...);
    return SharedPtr<T>(block->GetRawPtr(), block);
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};
