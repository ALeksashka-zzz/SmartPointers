#pragma once

#include <type_traits>
#include <utility>

template <typename T, std::size_t I, bool = std::is_empty_v<T> && !std::is_final_v<T>>
class CompressedPairElement {
public:
    CompressedPairElement() = default;
    template <class T_>
    explicit CompressedPairElement(T_&& elem) : value_(std::forward<T_>(elem)) {}
    const T& Get() const {
        return value_;
    }
    T& Get() {
        return value_;
    }
    T value_;
};

template <typename T, std::size_t I>
class CompressedPairElement<T, I, true> : public T {
public:
    CompressedPairElement() = default;
    template <class T_>
    explicit CompressedPairElement(T_&& elem) : T(std::forward<T_>(elem)) {}
    const T& Get() const {
        return *this;
    }
    T& Get() {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : private CompressedPairElement<F, 0>, private CompressedPairElement<S, 1> {
public:
    using First = CompressedPairElement<F, 0>;
    using Second = CompressedPairElement<S, 1>;
    CompressedPair() : First(), Second() {}
    template <class F_, class S_>
    CompressedPair(F_&& f, S_&& s) : First(std::forward<F_>(f)), Second(std::forward<S_>(s)) {}
    F& GetFirst() {
        return First::Get();
    }
    S& GetSecond() {
        return Second::Get();
    }
    const F& GetFirst() const {
        return First::Get();
    }
    const S& GetSecond() const {
        return Second::Get();
    }
};
