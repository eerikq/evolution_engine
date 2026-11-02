#pragma once

struct SDL_Window;

namespace platform {
    struct Window {
        SDL_Window* window = nullptr;
        int width = 0;
        int height = 0;
        bool was_resized = false;
        bool should_close = false;
    };

    struct WindowDescription {
        const char* app_name = "Evolution Engine";
        const char* app_identifier = "eerikq.evolution_engine";
        const char* app_version = "0.0.1";

        int default_width = 1200;
        int default_height = 900;
    };

    bool Init(const WindowDescription* description);
    void PollEvents(Window* window);
    void Quit();

    bool CreateWindow(Window* window, const WindowDescription* description);
    void ResizeWindow(Window* window);
    void DestroyWindow(Window* window);

    /*
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
    */
} // namespace platform
