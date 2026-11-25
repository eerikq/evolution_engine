#pragma once
#include <cstdint>

// ------------------ unsigned integer types ------------------
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using usize = std::size_t;

inline constexpr u8 u8_max = UINT8_MAX;
inline constexpr u16 u16_max = UINT16_MAX;
inline constexpr u32 u32_max = UINT32_MAX;
inline constexpr u64 u64_max = UINT64_MAX;
inline constexpr usize usize_max = SIZE_MAX;

// ------------------ signed integer types ------------------
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using isize = std::intptr_t;

inline constexpr i8 i8_min = INT8_MIN;
inline constexpr i8 i8_max = INT8_MAX;
inline constexpr i16 i16_min = INT16_MIN;
inline constexpr i16 i16_max = INT16_MAX;
inline constexpr i32 i32_min = INT32_MIN;
inline constexpr i32 i32_max = INT32_MAX;
inline constexpr i64 i64_min = INT64_MIN;
inline constexpr i64 i64_max = INT64_MAX;
inline constexpr isize isize_min = INTPTR_MIN;
inline constexpr isize isize_max = INTPTR_MAX;

//  ------------------ floating point types ------------------
using f32 = float;
using f64 = double;

// custom macros for min and max values of floats and doubles to avoid including <cfloat>
// taken from: https://clang.llvm.org/doxygen/InitPreprocessor_8cpp_source.html @ line 114
#ifndef FLT_MIN
#define FLT_MIN 1.17549435e-38F
#endif
#ifndef FLT_MAX
#define FLT_MAX 3.40282347e+38F
#endif
#ifndef DBL_MIN
#define DBL_MIN 2.2250738585072014e-308
#endif
#ifndef DBL_MAX
#define DBL_MAX 1.7976931348623157e+308
#endif

inline constexpr f32 f32_min = FLT_MIN;
inline constexpr f32 f32_max = FLT_MAX;
inline constexpr f64 f64_min = DBL_MIN;
inline constexpr f64 f64_max = DBL_MAX;
