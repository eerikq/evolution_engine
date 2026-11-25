#pragma once
#include "evo/core/math.h"
#include "evo/core/types.h"

namespace assets {
    struct Model {
        vec3* positions = nullptr;
        vec3* normals = nullptr;
        vec2* uvs = nullptr;
        u32* indices = nullptr;
        usize size = 0;
        u32 vertex_count = 0;
        u32 index_count = 0;
        // comes later with gltf:
        // vec4* colors
        // vec4* tangents
        // f32*  skinning_weights
        // u32*  bone_indices
    };

    // doesn't need a definition of a vertex struct anymore
    void LoadObj(const char* path, Model* model);
    void FreeModel(Model* model);
} // namespace assets
