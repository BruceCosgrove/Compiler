#pragma once

#include "common/for_each.hpp"
#include <utility>

#define _DEFINE_RANGED_ENUM_NAME(name) name,

#define _DEFINE_RANGED_ENUM_RANGE_(range, begin, end) \
    _range_##range##s_begin = begin, \
    _range_##range##s_end = end,
#define _DEFINE_RANGED_ENUM_RANGE(args) _DEFINE_RANGED_ENUM_RANGE_ args

#define _DEFINE_RANGED_ENUM_RANGE_CHECK_(name, range, begin, end) \
    [[nodiscard]] constexpr bool is_##range(const name e) noexcept \
    { return +name::_range_##range##s_begin <= +e && +e < +name::_range_##range##s_end; }
#define _DEFINE_RANGED_ENUM_RANGE_CHECK(name, args2) CALL(_DEFINE_RANGED_ENUM_RANGE_CHECK_, name, EXPAND args2)

#define DEFINE_RANGED_ENUM(name, enums, ranges) \
    enum class name \
    { \
        FOR_EACH(_DEFINE_RANGED_ENUM_NAME, EXPAND enums) \
        _count, \
        FOR_EACH(_DEFINE_RANGED_ENUM_RANGE, EXPAND ranges) \
    }; \
    [[nodiscard]] constexpr auto operator+(const name e) noexcept \
    { return std::to_underlying(e); } \
    FOR_EACH_ARGS(_DEFINE_RANGED_ENUM_RANGE_CHECK, (name), EXPAND ranges)
