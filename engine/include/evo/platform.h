#pragma once

struct SDL_Window;

namespace renderer {
    struct Camera;
}

namespace platform {
    struct Window {
        SDL_Window* window = nullptr;
        int width = 0;
        int height = 0;
        bool was_resized = false;
        bool should_close = false;
    };

    struct WindowDesc {
        const char* app_name = "Evolution Engine";
        const char* app_identifier = "eerikq.evolution_engine";
        const char* app_version = "0.0.1";

        int default_width = 1200;
        int default_height = 900;
    };

    bool Init(const WindowDesc* desc);
    void PollEvents(Window* window, renderer::Camera* camera);
    void Quit();

    bool CreateWindow(Window* window, const WindowDesc* desc);
    void ResizeWindow(Window* window);
    void DestroyWindow(Window* window);
} // namespace platform
