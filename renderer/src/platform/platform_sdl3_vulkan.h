#pragma once

#include <SDL3/SDL_vulkan.h>

namespace platform {
    struct Window;

    bool CreateSurface(const Window* window, const VkInstance instance, VkSurfaceKHR* surface);
    void DestroySurface(Window* window, const VkInstance instance, VkSurfaceKHR* surface);
    const char* const* GetWindowInstanceExtensions(uint32_t* count);
} // namespace platform
