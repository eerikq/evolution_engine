#pragma once

#include <evo/core/types.h>

struct SDL_Window;

namespace renderer {
    struct Camera;
}

namespace platform {
    struct WindowDesc {
        const char* app_name = "Evolution Engine";
        const char* app_identifier = "eerikq.evolution_engine";
        const char* app_version = "0.0.1";

        const u32 default_width = 1200;
        const u32 default_height = 900;
    };

    struct Window {
        SDL_Window* window = nullptr;
        u32 width = 0;
        u32 height = 0;
        bool was_resized = false;
        bool should_close = false;
        // 6 bytes padding
    };

    bool Init(const WindowDesc* desc);
    void PollEvents(Window* window, renderer::Camera* camera);
    void Quit();

    bool CreateWindow(Window* window, const WindowDesc* desc);
    void ResizeWindow(Window* window);
    void DestroyWindow(Window* window);
} // namespace platform
