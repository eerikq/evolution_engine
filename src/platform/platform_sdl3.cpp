#include "core/log.h"
#include "platform.h"

#include <SDL3/SDL_init.h>

bool platform::Init(const WindowDescription* description) {
    SDL_SetAppMetadata(description->app_name, description->app_version, description->app_identifier);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        evoLog(PrintSeverity::Error, "Failed to initialize SDL: {}", SDL_GetError());
        return false;
    }

    evoLog(PrintSeverity::Debug, "Initialized SDL");
    return true;
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

    evoLog(PrintSeverity::Debug, "Created SDL window");
    return true;
}

void platform::DestroyWindow(Window* window) {
    SDL_DestroyWindow(window->window);
    evoLog(PrintSeverity::Debug, "Destroyed SDL window");
}

bool platform::CreateSurface(Window* window, const VkInstance instance) {
    if (!SDL_Vulkan_CreateSurface(window->window, instance, nullptr, &window->surface)) {
        evoLog(PrintSeverity::Error, "Failed to create a Vulkan surface: {}", SDL_GetError());
        return false;
    }

    evoLog(PrintSeverity::Debug, "Created window surface");
    return true;
}

void platform::DestroySurface(Window* window, const VkInstance instance) {
    SDL_Vulkan_DestroySurface(instance, window->surface, nullptr);
    window->surface = nullptr;
}

const char* const* platform::GetWindowInstanceExtensions(uint32_t* count) {
    return SDL_Vulkan_GetInstanceExtensions(count);
}
