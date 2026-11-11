#include "core/log.h"
#include "gfx/command.h"
#include "gfx/descriptor.h"
#include "gfx/device.h"
#include "gfx/instance.h"
#include "gfx/resources/texture.h"
#include "gfx/swapchain.h"
#include "gfx/vma.h"
#include "platform/platform_sdl3_vulkan.h"

#include <evo/platform.h>
#include <evo/renderer.h>

struct renderer::Renderer {
    RendererDesc desc;

    gfx::Instance instance;
    gfx::Device device;
    gfx::Swapchain swapchain;
    gfx::Command command;
    gfx::Descriptor descriptor;
    gfx::Texture depth_texture;

    VkSurfaceKHR surface; // ← will probably be part of another struct.
    // or maybe not! surface is weird and doesnt *quite* have a well defined place it belongs to.
};

// temporarily keeping this here just to not have to think about opaque handles
static renderer::Renderer renderer_resources;

bool renderer::Create(Renderer* resources, platform::Window* window) {
    volkInitialize();

    uint32_t window_extension_count;
    const char* const* window_extensions = platform::GetWindowInstanceExtensions(&window_extension_count);
    gfx::InstanceDesc instance_desc = {
        .app_name = "evolution_engine",
        .engine_name = "evolution_engine",
        .application_version = VK_MAKE_API_VERSION(0, 0, 0, 1),
        .engine_version = VK_MAKE_API_VERSION(0, 0, 0, 1),
        .vulkan_api_version = VK_MAKE_API_VERSION(0, 1, 4, 0),
        .enable_validation_layers = renderer_resources.desc.enable_validation_layers,
        .validation_layers_count = 1,
        .validation_layers = renderer_resources.desc.required_validation_layers[0], // not really the best solution either, should revisit later
        .extensions_count = window_extension_count,
        .extensions = window_extensions,
    };
    gfx::CreateInstance(&renderer_resources.instance, &instance_desc);
    volkLoadInstance(renderer_resources.instance.instance);
    if (instance_desc.enable_validation_layers) gfx::CreateDebugMessenger(&renderer_resources.instance);

    platform::CreateSurface(window, renderer_resources.instance.instance, &renderer_resources.surface);

    gfx::DeviceDesc device_desc = {
        .device_extensions_count = 1,
        .device_extensions = renderer_resources.desc.required_device_extensions[0],
    };
    gfx::PickPhysicalDevice(&renderer_resources.device, &device_desc, renderer_resources.instance.instance);
    gfx::CreateLogicalDevice(&renderer_resources.device, &device_desc, renderer_resources.surface);
    volkLoadDevice(renderer_resources.device.logical);

    gfx::CreateAllocator(&renderer_resources.device, renderer_resources.instance.instance);

    gfx::SwapchainDesc swapchain_desc = {
        .image_count = 2,
        .desired_format = VK_FORMAT_B8G8R8A8_SRGB,
        .desired_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .desired_present_mode = VK_PRESENT_MODE_MAILBOX_KHR,
    };
    renderer_resources.swapchain.desc = swapchain_desc;
    gfx::CreateSwapchain(&renderer_resources.swapchain, window, &renderer_resources.device, renderer_resources.surface);
    gfx::GetSwapchainImages(&renderer_resources.swapchain, renderer_resources.device.logical);
    gfx::CreateSwapchainImageViews(&renderer_resources.swapchain, renderer_resources.device.logical);

    gfx::TextureDesc depth_texture_desc = {
        .width = renderer_resources.swapchain.extent.width,
        .height = renderer_resources.swapchain.extent.height,
        .format = renderer_resources.device.depth_foramt,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .allocation_flags = 0,
    };
    gfx::CreateTexture(renderer_resources.device.logical, renderer_resources.device.allocator, &depth_texture_desc, &renderer_resources.depth_texture);
    gfx::CreateTextureView(renderer_resources.device.logical, depth_texture_desc.format, VK_IMAGE_ASPECT_DEPTH_BIT, &renderer_resources.depth_texture);

    gfx::CreateDescriptorSetLayout(renderer_resources.device.logical, &renderer_resources.descriptor);
    // Create Pipeline Layout
    // Create Pipeline (Graphics)

    gfx::CreateCommandPool(&renderer_resources.device, &renderer_resources.command);
    gfx::AllocateCommandBuffers(&renderer_resources.device, &renderer_resources.command);

    // Create Uniforms (?)
    // Create Sync Objects

    return true;
}

void renderer::DrawFrame() {
    //
}

bool renderer::Resize(Renderer* resources, platform::Window* window) {
    gfx::DestroySwapchainImageViews(&renderer_resources.swapchain, renderer_resources.device.logical);
    gfx::DestroySwapchain(&renderer_resources.swapchain, renderer_resources.device.logical);

    gfx::CreateSwapchain(&renderer_resources.swapchain, window, &renderer_resources.device, renderer_resources.surface);
    gfx::GetSwapchainImages(&renderer_resources.swapchain, renderer_resources.device.logical);
    gfx::CreateSwapchainImageViews(&renderer_resources.swapchain, renderer_resources.device.logical);

    window->was_resized = false;
    evoLog(PrintSeverity::Info, "Resized renderer");

    return true;
}

void renderer::Destroy(Renderer* resources, platform::Window* window) {
    // Destroy Sync Objects
    // Destroy Uniforms (?)
    // Destroy Pipeline (Graphics)
    // Destroy Pipeline Layout
    gfx::DestroyDescriptorSetLayout(renderer_resources.device.logical, &renderer_resources.descriptor);
    gfx::DestroyTexture(renderer_resources.device.logical, renderer_resources.device.allocator, &renderer_resources.depth_texture);

    gfx::DestroySwapchainImageViews(&renderer_resources.swapchain, renderer_resources.device.logical);
    gfx::DestroySwapchain(&renderer_resources.swapchain, renderer_resources.device.logical);

    gfx::DestroyAllocator(&renderer_resources.device);

    gfx::DestroyLogicalDevice(&renderer_resources.device);
    platform::DestroySurface(window, renderer_resources.instance.instance, &renderer_resources.surface);

    if (renderer_resources.desc.enable_validation_layers) gfx::DestroyDebugMessenger(&renderer_resources.instance);
    gfx::DestroyInstance(&renderer_resources.instance);
}
