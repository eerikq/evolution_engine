#include "SDL3/SDL_video.h"
#include "core/log.h"
#include "evo/platform.h"

#include <SDL3/SDL.h>

bool platform::Init(const WindowDescription* description) {
    SDL_SetAppMetadata(description->app_name, description->app_version, description->app_identifier);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        evoLog(PrintSeverity::Error, "Failed to initialize SDL: {}", SDL_GetError());
        return false;
    }

    evoLog(PrintSeverity::Debug, "Initialized SDL");
    return true;
}

void platform::PollEvents(Window* window) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EventType::SDL_EVENT_QUIT) {
            window->should_close = true;
        } else if (event.type == SDL_EventType::SDL_EVENT_WINDOW_RESIZED) {
            window->was_resized = true;
        }
    }
}

void platform::Quit() {
    SDL_Quit();
}

bool platform::CreateWindow(Window* window, const WindowDescription* description) {
    window->window = SDL_CreateWindow(description->app_name, description->default_width, description->default_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

    if (!window->window) {
        evoLog(PrintSeverity::Error, "Failed to create window: {}", SDL_GetError());
        return false;
    }

    SDL_GetWindowSizeInPixels(window->window, &window->width, &window->height);
    evoLog(PrintSeverity::Debug, "Created SDL window with dimensions {}x{}", window->width, window->height);
    return true;
}

void platform::ResizeWindow(Window* window) {
    SDL_GetWindowSizeInPixels(window->window, &window->width, &window->height);
    window->was_resized = false;
    evoLog(PrintSeverity::Debug, "Resized Window. New dimensions: {}x{}", window->width, window->height);
}

void platform::DestroyWindow(Window* window) {
    SDL_DestroyWindow(window->window);
    evoLog(PrintSeverity::Debug, "Destroyed SDL window");
}

// bool platform::CreateSurface(Window* window, const VkInstance instance) {
//     if (!SDL_Vulkan_CreateSurface(window->window, instance, nullptr, &window->surface)) {
//         evoLog(PrintSeverity::Error, "Failed to create a Vulkan surface: {}", SDL_GetError());
//         return false;
//     }

//     evoLog(PrintSeverity::Debug, "Created window surface");
//     return true;
// }

// void platform::DestroySurface(Window* window, const VkInstance instance) {
//     SDL_Vulkan_DestroySurface(instance, window->surface, nullptr);
//     window->surface = nullptr;
//     evoLog(PrintSeverity::Debug, "Destroyed window surface");
// }

// const char* const* platform::GetWindowInstanceExtensions(uint32_t* count) {
//     return SDL_Vulkan_GetInstanceExtensions(count);
// }
