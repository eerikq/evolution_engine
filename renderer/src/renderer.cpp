#include <SDL3/SDL_vulkan.h>
#include <evo/renderer.h>
#include <volk.h>
//
#include <vk_mem_alloc.h>

struct RendererResources {
    VkInstance instance = nullptr;
    VkDebugUtilsMessengerEXT debug_messenger = nullptr;
    VmaAllocator allocator = nullptr;
    VkSurfaceKHR surface;
};

bool renderer::Create(renderer::RendererResources* resources, renderer::RendererDesc* description, platform::Window* window) {
    return true;
}
void renderer::DrawFrame() {
    //
}
void renderer::Destroy() {
    //
}
