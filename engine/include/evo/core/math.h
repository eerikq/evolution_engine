#pragma once
#include <evo/core/types.h>

// need to put these comments here until i figure out why the fuck clang-format is tweaking
// clang-format off
struct vec4 {
    union { f32 x = 0.0f; f32 r; };
    union { f32 y = 0.0f; f32 g; };
    union { f32 z = 0.0f; f32 b; };
    union { f32 w = 0.0f; f32 a; };
};

struct vec3 {
    union { f32 x = 0.0f; f32 r; };
    union { f32 y = 0.0f; f32 g; };
    union { f32 z = 0.0f; f32 b; };
};

struct vec2 {
    union { f32 x = 0.0f; f32 u; };
    union { f32 y = 0.0f; f32 v; };
};

struct ivec4 {
    union { u32 x = 0.0f; };
    union { u32 y = 0.0f; };
    union { u32 z = 0.0f; };
    union { u32 w = 0.0f; };
};

struct ivec3 {
    union { u32 x = 0.0f; };
    union { u32 y = 0.0f; };
    union { u32 z = 0.0f; };
};

struct ivec2 {
    union { u32 x = 0.0f; };
    union { u32 y = 0.0f; };
};
// clang-format on
