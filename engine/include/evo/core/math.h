#pragma once
#include <evo/core/types.h>

// need to put these comments here until i figure out why the fuck clang-format is tweaking
// clang-format off
// ------------------ vec4 ------------------
struct vec4 {
    union { f32 x = 0.0f; f32 r; };
    union { f32 y = 0.0f; f32 g; };
    union { f32 z = 0.0f; f32 b; };
    union { f32 w = 0.0f; f32 a; };
};

constexpr vec4 operator + (vec4 a, vec4 b) { return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }
constexpr vec4 operator - (vec4 a, vec4 b) { return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; }
constexpr vec4 operator * (vec4 a, vec4 b) { return { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w }; }
constexpr vec4 operator * (f32 a, vec4 b) { return { a * b.x, a * b.y, a * b.z, a * b.w }; }
constexpr vec4 operator * (vec4 a, f32 b) { return { a.x * b, a.y * b, a.z * b, a.w * b }; }
constexpr vec4 operator / (vec4 a, vec4 b) { return { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w }; }
constexpr vec4 operator / (vec4 a, f32 b) { return { a.x / b, a.y / b, a.z / b, a.w / b }; }
constexpr vec4 operator - (vec4 v) { return { -v.x, -v.y, -v.z, -v.w }; }

constexpr vec4 &operator += (vec4 &a, vec4 b) { return a = a + b; }
constexpr vec4 &operator -= (vec4 &a, vec4 b) { return a = a - b; }
constexpr vec4 &operator *= (vec4 &v, vec4 b) { return v = v * b; }
constexpr vec4 &operator *= (vec4 &v, f32 b) { return v = v * b; }
constexpr vec4 &operator /= (vec4 &v, vec4 b) { return v = v / b; }
constexpr vec4 &operator /= (vec4 &v, f32 b) { return v = v / b; }

constexpr bool operator == (vec4 a, vec4 b) { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }
constexpr bool operator != (vec4 a, vec4 b) { return !(a == b); }

// ------------------ vec3 ------------------
struct vec3 {
    union { f32 x = 0.0f; f32 r; };
    union { f32 y = 0.0f; f32 g; };
    union { f32 z = 0.0f; f32 b; };
};

constexpr vec3 operator + (vec3 a, vec3 b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
constexpr vec3 operator - (vec3 a, vec3 b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
constexpr vec3 operator * (vec3 a, vec3 b) { return { a.x * b.x, a.y * b.y, a.z * b.z }; }
constexpr vec3 operator * (f32 a, vec3 b) { return { a * b.x, a * b.y, a * b.z  }; }
constexpr vec3 operator * (vec3 a, f32 b) { return { a.x * b, a.y * b, a.z * b }; }
constexpr vec3 operator / (vec3 a, vec3 b) { return { a.x / b.x, a.y / b.y, a.z / b.z }; }
constexpr vec3 operator / (vec3 a, f32 b) { return { a.x / b, a.y / b, a.z / b }; }
constexpr vec3 operator - (vec3 v) { return { -v.x, -v.y, -v.z }; }

constexpr vec3 &operator += (vec3 &a, vec3 b) { return a = a + b; }
constexpr vec3 &operator -= (vec3 &a, vec3 b) { return a = a - b; }
constexpr vec3 &operator *= (vec3 &v, vec3 b) { return v = v * b; }
constexpr vec3 &operator *= (vec3 &v, f32 b) { return v = v * b; }
constexpr vec3 &operator /= (vec3 &v, vec3 b) { return v = v / b; }
constexpr vec3 &operator /= (vec3 &v, f32 b) { return v = v / b; }

constexpr bool operator == (vec3 a, vec3 b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
constexpr bool operator != (vec3 a, vec3 b) { return !(a == b); }

// ------------------ vec2 ------------------
struct vec2 {
    union { f32 x = 0.0f; f32 u; };
    union { f32 y = 0.0f; f32 v; };
};

constexpr vec2 operator + (vec2 a, vec2 b) { return { a.x + b.x, a.y + b.y }; }
constexpr vec2 operator - (vec2 a, vec2 b) { return { a.x - b.x, a.y - b.y }; }
constexpr vec2 operator * (vec2 a, vec2 b) { return { a.x * b.x, a.y * b.y }; }
constexpr vec2 operator * (f32 a, vec2 b) { return { a * b.x, a * b.y }; }
constexpr vec2 operator * (vec2 a, f32 b) { return { a.x * b, a.y * b }; }
constexpr vec2 operator / (vec2 a, vec2 b) { return { a.x / b.x, a.y / b.y }; }
constexpr vec2 operator / (vec2 a, f32 b) { return { a.x / b, a.y / b }; }
constexpr vec2 operator - (vec2 v) { return { -v.x, -v.y }; }

constexpr vec2 &operator += (vec2 &a, vec2 b) { return a = a + b; }
constexpr vec2 &operator -= (vec2 &a, vec2 b) { return a = a - b; }
constexpr vec2 &operator *= (vec2 &v, vec2 b) { return v = v * b; }
constexpr vec2 &operator *= (vec2 &v, f32 b) { return v = v * b; }
constexpr vec2 &operator /= (vec2 &v, vec2 b) { return v = v / b; }
constexpr vec2 &operator /= (vec2 &v, f32 b) { return v = v / b; }

constexpr bool operator == (vec2 a, vec2 b) { return a.x == b.x && a.y == b.y; }
constexpr bool operator != (vec2 a, vec2 b) { return !(a == b); }

// ------------------ ivec4 ------------------
struct ivec4 {
    union { i32 x = 0; i32 r; };
    union { i32 y = 0; i32 g; };
    union { i32 z = 0; i32 b; };
    union { i32 w = 0; i32 a; };
};

constexpr ivec4 operator + (ivec4 a, ivec4 b) { return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }
constexpr ivec4 operator - (ivec4 a, ivec4 b) { return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; }
constexpr ivec4 operator * (ivec4 a, ivec4 b) { return { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w }; }
constexpr ivec4 operator * (i32 a, ivec4 b) { return { a * b.x, a * b.y, a * b.z, a * b.w }; }
constexpr ivec4 operator * (ivec4 a, i32 b) { return { a.x * b, a.y * b, a.z * b, a.w * b }; }
constexpr ivec4 operator / (ivec4 a, ivec4 b) { return { a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w }; }
constexpr ivec4 operator / (ivec4 a, i32 b) { return { a.x / b, a.y / b, a.z / b, a.w / b }; }
constexpr ivec4 operator - (ivec4 v) { return { -v.x, -v.y, -v.z, -v.w }; }

constexpr ivec4 &operator += (ivec4 &a, ivec4 b) { return a = a + b; }
constexpr ivec4 &operator -= (ivec4 &a, ivec4 b) { return a = a - b; }
constexpr ivec4 &operator *= (ivec4 &v, ivec4 b) { return v = v * b; }
constexpr ivec4 &operator *= (ivec4 &v, i32 b) { return v = v * b; }
constexpr ivec4 &operator /= (ivec4 &v, ivec4 b) { return v = v / b; }
constexpr ivec4 &operator /= (ivec4 &v, i32 b) { return v = v / b; }

constexpr bool operator == (ivec4 a, ivec4 b) { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }
constexpr bool operator != (ivec4 a, ivec4 b) { return !(a == b); }

// ------------------ ivec3 ------------------
struct ivec3 {
    union { i32 x = 0; i32 r; };
    union { i32 y = 0; i32 g; };
    union { i32 z = 0; i32 b; };
};

constexpr ivec3 operator + (ivec3 a, ivec3 b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
constexpr ivec3 operator - (ivec3 a, ivec3 b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
constexpr ivec3 operator * (ivec3 a, ivec3 b) { return { a.x * b.x, a.y * b.y, a.z * b.z }; }
constexpr ivec3 operator * (i32 a, ivec3 b) { return { a * b.x, a * b.y, a * b.z  }; }
constexpr ivec3 operator * (ivec3 a, i32 b) { return { a.x * b, a.y * b, a.z * b }; }
constexpr ivec3 operator / (ivec3 a, ivec3 b) { return { a.x / b.x, a.y / b.y, a.z / b.z }; }
constexpr ivec3 operator / (ivec3 a, i32 b) { return { a.x / b, a.y / b, a.z / b }; }
constexpr ivec3 operator - (ivec3 v) { return { -v.x, -v.y, -v.z }; }

constexpr ivec3 &operator += (ivec3 &a, ivec3 b) { return a = a + b; }
constexpr ivec3 &operator -= (ivec3 &a, ivec3 b) { return a = a - b; }
constexpr ivec3 &operator *= (ivec3 &v, ivec3 b) { return v = v * b; }
constexpr ivec3 &operator *= (ivec3 &v, i32 b) { return v = v * b; }
constexpr ivec3 &operator /= (ivec3 &v, ivec3 b) { return v = v / b; }
constexpr ivec3 &operator /= (ivec3 &v, i32 b) { return v = v / b; }

constexpr bool operator == (ivec3 a, ivec3 b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
constexpr bool operator != (ivec3 a, ivec3 b) { return !(a == b); }

// ------------------ ivec2 ------------------
struct ivec2 {
    union { i32 x = 0; i32 u; };
    union { i32 y = 0; i32 v; };
};

constexpr ivec2 operator + (ivec2 a, ivec2 b) { return { a.x + b.x, a.y + b.y }; }
constexpr ivec2 operator - (ivec2 a, ivec2 b) { return { a.x - b.x, a.y - b.y }; }
constexpr ivec2 operator * (ivec2 a, ivec2 b) { return { a.x * b.x, a.y * b.y }; }
constexpr ivec2 operator * (i32 a, ivec2 b) { return { a * b.x, a * b.y }; }
constexpr ivec2 operator * (ivec2 a, i32 b) { return { a.x * b, a.y * b }; }
constexpr ivec2 operator / (ivec2 a, ivec2 b) { return { a.x / b.x, a.y / b.y }; }
constexpr ivec2 operator / (ivec2 a, i32 b) { return { a.x / b, a.y / b }; }
constexpr ivec2 operator - (ivec2 v) { return { -v.x, -v.y }; }

constexpr ivec2 &operator += (ivec2 &a, ivec2 b) { return a = a + b; }
constexpr ivec2 &operator -= (ivec2 &a, ivec2 b) { return a = a - b; }
constexpr ivec2 &operator *= (ivec2 &v, ivec2 b) { return v = v * b; }
constexpr ivec2 &operator *= (ivec2 &v, i32 b) { return v = v * b; }
constexpr ivec2 &operator /= (ivec2 &v, ivec2 b) { return v = v / b; }
constexpr ivec2 &operator /= (ivec2 &v, i32 b) { return v = v / b; }

constexpr bool operator == (ivec2 a, ivec2 b) { return a.x == b.x && a.y == b.y; }
constexpr bool operator != (ivec2 a, ivec2 b) { return !(a == b); }
// clang-format on
