#pragma once

namespace platform {
    struct Window;
} // namespace platform

namespace renderer {
    struct RendererDesc {
        const char* required_validation_layers[1] = {
            "VK_LAYER_KHRONOS_validation",
        };
        const char* required_device_extensions[1] = {
            "VK_KHR_swapchain",
        };
    };

    struct Renderer;

    struct Camera {
        float pos_x = 0, pos_y = 0, pos_z = 0;
        float yaw = 0, pitch = 0;

        float fov = 90.0;
        float aspect_ratio = 4.0 / 3.0;
        float near_plane = 0.1;
        float far_plane = 1000;

        bool is_perspective = true;
    };

    bool Create(Renderer* resources, platform::Window* window);

    bool CreateResources(Renderer* resources);
    void DestroyResources(Renderer* resources);

    void DrawFrame(Renderer* resources, platform::Window* window);
    bool Resize(Renderer* resources, platform::Window* window);
    void Destroy(Renderer* resources, platform::Window* window);
}; // namespace renderer
