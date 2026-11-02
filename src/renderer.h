#pragma once

#ifdef NDEBUG
inline constexpr bool VALIDATION_LAYERS_DEFAULT = false;
#else
inline constexpr bool VALIDATION_LAYERS_DEFAULT = true;
#endif

namespace renderer {
    struct RendererDesc {
        int max_frames_in_flight = 2;
        bool use_validation_layers = VALIDATION_LAYERS_DEFAULT;
        const char* required_validation_layers[1] = {
            "VK_LAYER_KHRONOS_validation",
        };
    };
}; // namespace renderer
