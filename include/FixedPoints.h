#pragma once

#include <Arduino.h>

#include <cstdint>
#include <type_traits>

template <int IntegerBits, int FractionBits>
class SFixed {
    using Storage = std::conditional_t<(IntegerBits + FractionBits + 1 <= 16), std::int16_t, std::int32_t>;

public:
    constexpr SFixed() = default;

    template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    constexpr SFixed(T value) : raw_(static_cast<Storage>(value * static_cast<T>(scale()))) {}

    static constexpr SFixed fromInternal(Storage raw) {
        SFixed value;
        value.raw_ = raw;
        return value;
    }
    static constexpr SFixed fromInternal(SFixed value) { return value; }

    constexpr Storage getInteger() const { return static_cast<Storage>(raw_ / scale()); }
    constexpr Storage getInternal() const { return raw_; }
    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    explicit constexpr operator T() const { return static_cast<T>(getInteger()); }
    constexpr SFixed operator-() const { return fromInternal(static_cast<Storage>(-raw_)); }
    constexpr SFixed& operator+=(SFixed other) { raw_ = static_cast<Storage>(raw_ + other.raw_); return *this; }
    constexpr SFixed& operator-=(SFixed other) { raw_ = static_cast<Storage>(raw_ - other.raw_); return *this; }

    friend constexpr SFixed operator+(SFixed left, SFixed right) { return fromInternal(static_cast<Storage>(left.raw_ + right.raw_)); }
    friend constexpr SFixed operator-(SFixed left, SFixed right) { return fromInternal(static_cast<Storage>(left.raw_ - right.raw_)); }
    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    friend constexpr SFixed operator*(T left, SFixed right) { return fromInternal(static_cast<Storage>(left * right.raw_)); }
    friend constexpr bool operator==(SFixed left, SFixed right) { return left.raw_ == right.raw_; }
    friend constexpr bool operator!=(SFixed left, SFixed right) { return left.raw_ != right.raw_; }
    friend constexpr bool operator<(SFixed left, SFixed right) { return left.raw_ < right.raw_; }
    friend constexpr bool operator<=(SFixed left, SFixed right) { return left.raw_ <= right.raw_; }
    friend constexpr bool operator>(SFixed left, SFixed right) { return left.raw_ > right.raw_; }
    friend constexpr bool operator>=(SFixed left, SFixed right) { return left.raw_ >= right.raw_; }

private:
    static constexpr std::int32_t scale() { return std::int32_t{1} << FractionBits; }
    Storage raw_ = 0;
};

template <int IntegerBits, int FractionBits>
constexpr SFixed<IntegerBits, FractionBits> abs(SFixed<IntegerBits, FractionBits> value) {
    return value < SFixed<IntegerBits, FractionBits>{0} ? -value : value;
}

using SQ7x8 = SFixed<7, 8>;

template <int IntegerBits, int FractionBits>
SFixed<IntegerBits, FractionBits> randomSFixed(SFixed<IntegerBits, FractionBits> minimum,
                                                SFixed<IntegerBits, FractionBits> maximum) {
    const auto low = minimum.getInternal();
    const auto high = maximum.getInternal();
    if (high <= low) return minimum;
    return SFixed<IntegerBits, FractionBits>::fromInternal(
        static_cast<decltype(low)>(random(static_cast<long>(low), static_cast<long>(high))));
}
