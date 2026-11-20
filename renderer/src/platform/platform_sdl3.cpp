#include "SDL3/SDL_events.h"
#include "evo/renderer.h" // SHOULDNT BE HERE
//
#include "core/log.h"
#include "imgui_impl_sdl3.h"
#include "platform/platform_sdl3_vulkan.h"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <evo/platform.h>

bool platform::Init(const WindowDesc* description) {
    SDL_SetAppMetadata(description->app_name, description->app_version, description->app_identifier);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        evoLog(PrintSeverity::Error, "Failed to initialize SDL: {}", SDL_GetError());
        return false;
    }

    evoLog(PrintSeverity::Debug, "Initialized SDL");
    return true;
}

void platform::PollEvents(Window* window, renderer::Camera* camera) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        // window events
        if (event.type == SDL_EVENT_QUIT) {
            window->should_close = true;
        } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            window->was_resized = true;
        }

        // camera inputs
        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.scancode == SDL_SCANCODE_TAB) {
                camera->is_mouse_captured = !camera->is_mouse_captured;
                SDL_SetWindowRelativeMouseMode(window->window, camera->is_mouse_captured);
            } else if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                window->should_close = true;
            }
        }
        if (camera->is_mouse_captured && event.type == SDL_EVENT_MOUSE_MOTION) {
            camera->yaw -= event.motion.xrel * camera->sensitivity;
            // pitch clamp
            camera->pitch -= event.motion.yrel * camera->sensitivity;
            if (camera->pitch > 89.0f) camera->pitch = 89.0f;
            if (camera->pitch < -89.0f) camera->pitch = -89.0f;
        }
        if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            camera->camera_speed += 0.25 * event.wheel.y;
            if (camera->camera_speed < camera->camera_speed_min) camera->camera_speed = camera->camera_speed_min;
            if (camera->camera_speed > camera->camera_speed_max) camera->camera_speed = camera->camera_speed_max;
        }

        if (event.key.scancode == SDL_SCANCODE_A) {
            if (event.type == SDL_EVENT_KEY_DOWN) camera->input_direction[0] = -1;
            if (event.type == SDL_EVENT_KEY_UP) camera->input_direction[0] = 0;
        } else if (event.key.scancode == SDL_SCANCODE_D) {
            if (event.type == SDL_EVENT_KEY_DOWN) camera->input_direction[0] = 1;
            if (event.type == SDL_EVENT_KEY_UP) camera->input_direction[0] = 0;
        } else if (event.key.scancode == SDL_SCANCODE_W) {
            if (event.type == SDL_EVENT_KEY_DOWN) camera->input_direction[1] = 1;
            if (event.type == SDL_EVENT_KEY_UP) camera->input_direction[1] = 0;
        } else if (event.key.scancode == SDL_SCANCODE_S) {
            if (event.type == SDL_EVENT_KEY_DOWN) camera->input_direction[1] = -1;
            if (event.type == SDL_EVENT_KEY_UP) camera->input_direction[1] = 0;
        } else if (event.key.scancode == SDL_SCANCODE_Q) {
            if (event.type == SDL_EVENT_KEY_DOWN) camera->input_direction[2] = -1;
            if (event.type == SDL_EVENT_KEY_UP) camera->input_direction[2] = 0;
        } else if (event.key.scancode == SDL_SCANCODE_E) {
            if (event.type == SDL_EVENT_KEY_DOWN) camera->input_direction[2] = 1;
            if (event.type == SDL_EVENT_KEY_UP) camera->input_direction[2] = 0;
        }

        ImGui_ImplSDL3_ProcessEvent(&event);
    }
}

void platform::Quit() {
    SDL_Quit();
}

bool platform::CreateWindow(Window* window, const WindowDesc* description) {
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
    evoLog(PrintSeverity::Debug, "Resized Window. New dimensions: {}x{}", window->width, window->height);
}

void platform::DestroyWindow(Window* window) {
    SDL_DestroyWindow(window->window);
    evoLog(PrintSeverity::Debug, "Destroyed SDL window");
}

// ------------------ sdl3_vulkan ------------------

bool platform::CreateSurface(const Window* window, const VkInstance instance, VkSurfaceKHR* surface) {
    if (!SDL_Vulkan_CreateSurface(window->window, instance, nullptr, surface)) {
        evoLog(PrintSeverity::Error, "Failed to create a Vulkan surface: {}", SDL_GetError());
        return false;
    }

    evoLog(PrintSeverity::Info, "Created window surface");
    return true;
}

void platform::DestroySurface(Window* window, const VkInstance instance, VkSurfaceKHR* surface) {
    SDL_Vulkan_DestroySurface(instance, *surface, nullptr);
    *surface = nullptr;
    evoLog(PrintSeverity::Debug, "Destroyed window surface");
}

const char* const* platform::GetWindowInstanceExtensions(uint32_t* count) {
    return SDL_Vulkan_GetInstanceExtensions(count);
}
