#include "gfx/instance.h"
#include "platform/platform_sdl3_vulkan.h"
#include "vulkan/vulkan_core.h"

#include <evo/renderer.h>
#include <volk.h>
//
#include <vk_mem_alloc.h>

struct renderer::RendererResources {
    gfx::Instance instance;

    VkSurfaceKHR surface; // ← will probably be part of another struct.
    // or maybe not! surface is weird and doesnt *quite* have a well defined place it belongs to.
};

bool renderer::Create(RendererResources* resources, const RendererDesc* desc, platform::Window* window) {
    volkInitialize();

    uint32_t window_extension_count;
    const char* const* window_extensions = platform::GetWindowInstanceExtensions(&window_extension_count);
    gfx::InstanceDesc instance_desc = {
        .app_name = "evolution_engine",
        .engine_name = "evolution_engine",
        .application_version = VK_MAKE_API_VERSION(0, 0, 0, 1),
        .engine_version = VK_MAKE_API_VERSION(0, 0, 0, 1),
        .vulkan_api_version = VK_MAKE_API_VERSION(0, 1, 4, 0),
#ifdef NDEBUG // not too fond of this. maybe have it pass through renderer desc?
        .enable_validation_layers = false,
#else
        .enable_validation_layers = true,
#endif
        .validation_layers_count = 1,
        .validation_layers = desc->required_validation_layers[0], // not really the best solution either, should revisit later
        .extensions_count = window_extension_count,
        .extensions = window_extensions,
    };
    gfx::CreateInstance(&resources->instance, &instance_desc); // TODO: add proper logs inside these
    volkLoadInstance(resources->instance.instance);
    gfx::CreateDebugMessenger(&resources->instance); // TODO: add proper logs inside these

    platform::CreateSurface(window, resources->instance.instance, &resources->surface);

    // pick physical device
    // create logical device
    // load logical device into volk

    return true;
}
void renderer::DrawFrame() {
    //
}
void renderer::Destroy(RendererResources* resources, platform::Window* window) {
    platform::DestroySurface(window, resources->instance.instance, &resources->surface);

    gfx::DestroyDebugMessenger(&resources->instance);
    gfx::DestroyInstance(&resources->instance);
}
