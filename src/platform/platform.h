#pragma once

#include <SDL3/SDL_vulkan.h>

namespace platform {
    struct Window {
        SDL_Window* window;
        VkSurfaceKHR surface;
        int width, height;
        bool was_resized;
        bool should_close;
    };

    struct WindowDescription {
        const char* app_name = "Evolution Renderer";
        const char* app_identifier = "eerikq.evolution_renderer";
        const char* app_version = "0.0.1";

        int default_width = 1200;
        int default_height = 900;
    };

    bool Init(const WindowDescription* description);
    void Quit();

    bool CreateWindow(Window* window, const WindowDescription* description);
    void DestroyWindow(Window* window);

    bool CreateSurface(Window* window, const VkInstance instance);
    void DestroySurface(Window* window, const VkInstance instance);

    const char* const* GetWindowInstanceExtensions(uint32_t* count);

    // not strictly relevant right now, mostly for the future.
    // this also doesn't belong here, will 100% need to be moved out to the engine.
    struct Time {
        int frame_count;   // total number of frames since the start
        float time;        // time of the current frame since the start in seconds
        double delta_time; // time difference between the last frame and the current one in seconds
        float fps;         // current frames per second compares to the last frame
        float fps_smooth;  // current frames per second compared to the average of the previous second
    };

    void UpdateTime(Time* time);
} // namespace platform
