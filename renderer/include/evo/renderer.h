#pragma once

namespace platform {
    struct Window;
} // namespace platform

namespace renderer {
    struct RendererDesc {
        const char* required_validation_layers[1] = {};
        const char* required_device_extensions[1] = {};
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

    bool Create(const platform::Window* window, const RendererDesc* desc, Renderer** renderer_handle);
    void Destroy(platform::Window* window, Renderer** renderer_handle);

    void CreateResources(Renderer* renderer);
    void DestroyResources(Renderer* renderer);

    void DrawFrame(const platform::Window* window, Renderer* renderer);
    bool Resize(const platform::Window* window, Renderer* renderer);
}; // namespace renderer
