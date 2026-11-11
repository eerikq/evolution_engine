#pragma once

namespace platform {
    struct Window;
} // namespace platform

namespace renderer {
    struct RendererDesc {
        const int max_frames_in_flight = 2;
#ifdef NDEBUG
        const bool enable_validation_layers = true;
#else
        const bool enable_validation_layers = false;
#endif
        const char* required_validation_layers[1] = {
            "VK_LAYER_KHRONOS_validation",
        };
        const char* required_device_extensions[1] = {
            "VK_KHR_swapchain",
        };
    };

    struct Renderer;

    bool Create(Renderer* resources, platform::Window* window);
    void DrawFrame();
    bool Resize(Renderer* resources, platform::Window* window);
    void Destroy(Renderer* resources, platform::Window* window);
}; // namespace renderer
