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
        const float up[3] = {0, 0, 1};
        const float right[3] = {1, 0, 0};
        const float forward[3] = {0, 1, 0};

        const float near_plane = 0.01, far_plane = 100.0;
        const float camera_speed_min = 0.25, camera_speed_max = 32.0;
        const float sensitivity_min = 0.01, sensitivity_max = 1;

        float pos[3] = {1.0, 1.0, 1.0};
        float yaw = -135.0, pitch = -22.5;
        float fov = 90.0;
        float orthographic_size = 2.0;
        bool is_orthographic = false;

        bool is_mouse_captured = false;
        int input_direction[3] = {0, 0, 0};
        float camera_speed = 8.0;
        float sensitivity = 0.125;
    };

    bool Create(const platform::Window* window, const RendererDesc* desc, Renderer** renderer_handle);
    void Destroy(platform::Window* window, Renderer** renderer_handle);

    namespace imgui {
        bool CreateContext(const platform::Window* window, const Renderer* renderer);
        void DestroyContext(const Renderer* renderer);

        void PollInput(const platform::Window* window);
        void Process(Renderer* renderer, Camera* camera);
        void Render(Renderer* renderer);
    } // namespace imgui

    bool CreateResources(Renderer* renderer);
    void DestroyResources(Renderer* renderer);

    void DrawFrame(platform::Window* window, Renderer* renderer, Camera* camera);
    bool Resize(platform::Window* window, Renderer* renderer);
}; // namespace renderer
