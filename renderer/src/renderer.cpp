#include "gfx/device.h"
#include "gfx/instance.h"
#include "gfx/vma.h"
#include "platform/platform_sdl3_vulkan.h"

#include <evo/renderer.h>
#include <volk.h>

struct renderer::Renderer {
    gfx::Instance instance;
    gfx::Device device;

    VkSurfaceKHR surface; // ← will probably be part of another struct.
    // or maybe not! surface is weird and doesnt *quite* have a well defined place it belongs to.
};

// temporarily keeping this here just to not have to think about opaque handles
static renderer::Renderer renderer_resources;

bool renderer::Create(Renderer* resources, const RendererDesc* desc, platform::Window* window) {
    volkInitialize();

    uint32_t window_extension_count;
    const char* const* window_extensions = platform::GetWindowInstanceExtensions(&window_extension_count);
    gfx::InstanceDesc instance_desc = {
        .app_name = "evolution_engine",
        .engine_name = "evolution_engine",
        .application_version = VK_MAKE_API_VERSION(0, 0, 0, 1),
        .engine_version = VK_MAKE_API_VERSION(0, 0, 0, 1),
        .vulkan_api_version = VK_MAKE_API_VERSION(0, 1, 4, 0),
        .enable_validation_layers = desc->enable_validation_layers,
        .validation_layers_count = 1,
        .validation_layers = desc->required_validation_layers[0], // not really the best solution either, should revisit later
        .extensions_count = window_extension_count,
        .extensions = window_extensions,
    };
    gfx::CreateInstance(&renderer_resources.instance, &instance_desc);
    volkLoadInstance(renderer_resources.instance.instance);
    if (instance_desc.enable_validation_layers) gfx::CreateDebugMessenger(&renderer_resources.instance);

    platform::CreateSurface(window, renderer_resources.instance.instance, &renderer_resources.surface);

    gfx::DeviceDesc device_desc = {
        .device_extensions_count = 1,
        .device_extensions = desc->required_device_extensions[0],
    };
    gfx::PickPhysicalDevice(&renderer_resources.device, &device_desc, renderer_resources.instance.instance);
    gfx::CreateLogicalDevice(&renderer_resources.device, &device_desc, renderer_resources.surface);
    volkLoadDevice(renderer_resources.device.logical);

    gfx::CreateAllocator(&renderer_resources.device, renderer_resources.instance.instance);

    return true;
}
void renderer::DrawFrame() {
    //
}
void renderer::Destroy(Renderer* resources, const RendererDesc* desc, platform::Window* window) {
    gfx::DestroyAllocator(&renderer_resources.device);

    gfx::DestroyLogicalDevice(&renderer_resources.device);
    platform::DestroySurface(window, renderer_resources.instance.instance, &renderer_resources.surface);

    if (desc->enable_validation_layers) gfx::DestroyDebugMessenger(&renderer_resources.instance);
    gfx::DestroyInstance(&renderer_resources.instance);
}
