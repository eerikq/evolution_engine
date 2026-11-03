#pragma once

#ifdef NDEBUG
inline constexpr bool VALIDATION_LAYERS_DEFAULT = false;
#else
inline constexpr bool VALIDATION_LAYERS_DEFAULT = true;
#endif

namespace platform {
    struct Window;
} // namespace platform

namespace renderer {
    struct RendererResources;
    struct RendererDesc {
        int max_frames_in_flight = 2;
        const char* required_validation_layers[1] = {
            "VK_LAYER_KHRONOS_validation",
        };
    };

    bool Create(RendererResources* resources, const RendererDesc* desc, platform::Window* window);
    void DrawFrame();
    void Destroy(RendererResources* resources, platform::Window* window);
}; // namespace renderer
