#pragma once

#include "renderer/resources/buffer.h"
#include "renderer/resources/texture.h"
#include "types/vertex.h"

#include <vector>
#include <volk.h>

typedef struct BufferContext {
    std::vector<Vertex> vertices = {
        // House Body
        {{-0.4f, -0.1f, 0.0f}, {0.9f, 0.9f, 0.9f}, {0.0f, 0.0f}}, // 0
        {{0.4f, -0.1f, 0.0f}, {0.9f, 0.9f, 0.9f}, {1.0f, 0.0f}},  // 1
        {{0.4f, 0.5f, 0.0f}, {0.9f, 0.9f, 0.9f}, {1.0f, 1.0f}},   // 2
        {{-0.4f, 0.5f, 0.0f}, {0.9f, 0.9f, 0.9f}, {0.0f, 1.0f}},  // 3

        // Roof
        {{-0.5f, -0.1f, 0.0f}, {0.8f, 0.1f, 0.1f}, {0.0f, 0.0f}},   // 4
        {{0.5f, -0.1f, 0.0f}, {0.8f, 0.1f, 0.1f}, {1.0f, 0.0f}},    // 5
        {{-0.25f, -0.33f, 0.0f}, {0.8f, 0.1f, 0.1f}, {0.5f, 1.0f}}, // 6

        // Door
        {{-0.1f, 0.5f, 0.0f}, {0.4f, 0.2f, 0.1f}, {0.0f, 0.0f}}, // 7
        {{0.1f, 0.5f, 0.0f}, {0.4f, 0.2f, 0.1f}, {1.0f, 0.0f}},  // 8
        {{0.1f, 0.2f, 0.0f}, {0.4f, 0.2f, 0.1f}, {1.0f, 1.0f}},  // 9
        {{-0.1f, 0.2f, 0.0f}, {0.4f, 0.2f, 0.1f}, {0.0f, 1.0f}}, // 10

        // Left Window
        {{-0.3f, 0.2f, 0.0f}, {0.2f, 0.6f, 0.9f}, {0.0f, 0.0f}},  // 11
        {{-0.15f, 0.2f, 0.0f}, {0.2f, 0.6f, 0.9f}, {1.0f, 0.0f}}, // 12
        {{-0.15f, 0.0f, 0.0f}, {0.2f, 0.6f, 0.9f}, {1.0f, 1.0f}}, // 13
        {{-0.3f, 0.0f, 0.0f}, {0.2f, 0.6f, 0.9f}, {0.0f, 1.0f}},  // 14

        // Right Window
        {{0.15f, 0.2f, 0.0f}, {0.2f, 0.6f, 0.9f}, {0.0f, 0.0f}}, // 15
        {{0.3f, 0.2f, 0.0f}, {0.2f, 0.6f, 0.9f}, {1.0f, 0.0f}},  // 16
        {{0.3f, 0.0f, 0.0f}, {0.2f, 0.6f, 0.9f}, {1.0f, 1.0f}},  // 17
        {{0.15f, 0.0f, 0.0f}, {0.2f, 0.6f, 0.9f}, {0.0f, 1.0f}}  // 18
    };

    std::vector<uint16_t> indices = {
        0,  1,  2,  2,  3,  0,  // House Body
        4,  5,  6,              // Roof
        7,  8,  9,  9,  10, 7,  // Door
        11, 12, 13, 13, 14, 11, // Left window
        15, 16, 17, 17, 18, 15  // Right window
    };

    Buffer vertex_buffer {};
    Buffer index_buffer {};

    std::vector<Buffer> uniform_buffers {};
    std::vector<void*> uniform_buffers_mapped {};

    Image image {};
    Texture texture {};
    Texture depth {};
    VkSampler sampler = nullptr;
} BufferContext;
