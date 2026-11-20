#include "core/log.h"
#include "core/model.h"
#include "gfx/command.h"
#include "gfx/descriptor.h"
#include "gfx/draw.h"
#include "gfx/instance.h"
#include "gfx/resources.h"
#include "gfx/vma.h"
#include "platform/platform_sdl3_vulkan.h"
#include "renderer_internal.h"

#include <chrono>
#include <evo/platform.h>
#include <evo/renderer.h>
#include <glm/trigonometric.hpp>

// terrible temp solution but i dont really know where to put this yet
static void CreateSyncObjects(renderer::Renderer* renderer) {
    VkSemaphoreCreateInfo semaphore_create_info {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    VkFenceCreateInfo fence_create_info {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (int i = 0; i < config::FRAMES_IN_FLIGHT; i++) {
        vkCreateFence(renderer->device.logical, &fence_create_info, nullptr, &renderer->frames[i].in_flight_fence);
        vkCreateSemaphore(renderer->device.logical, &semaphore_create_info, nullptr, &renderer->frames[i].present_complete_semaphore);
    }
    for (int i = 0; i < renderer->swapchain.image_count; i++) {
        vkCreateSemaphore(renderer->device.logical, &semaphore_create_info, nullptr, &renderer->swapchain.render_finished_semaphores[i]);
    }

    evoLog(PrintSeverity::Info, "Created sync objects");
}
static void DestroySyncObjects(renderer::Renderer* renderer) {
    for (int i = 0; i < config::FRAMES_IN_FLIGHT; i++) {
        vkDestroyFence(renderer->device.logical, renderer->frames[i].in_flight_fence, nullptr);
        vkDestroySemaphore(renderer->device.logical, renderer->frames[i].present_complete_semaphore, nullptr);
    }
    for (int i = 0; i < renderer->swapchain.image_count; i++) {
        vkDestroySemaphore(renderer->device.logical, renderer->swapchain.render_finished_semaphores[i], nullptr);
    }

    evoLog(PrintSeverity::Debug, "Destroyed sync objects");
}

bool renderer::Create(const platform::Window* window, const RendererDesc* desc, Renderer** renderer_handle) {
    if (renderer_handle == nullptr) return false;

    Renderer* renderer = new Renderer();

    volkInitialize();

    uint32_t window_extension_count;
    const char* const* window_extensions = platform::GetWindowInstanceExtensions(&window_extension_count);
    gfx::InstanceDesc instance_desc = {
        .app_name = "evolution_engine",
        .engine_name = "evolution_engine",
        .application_version = VK_MAKE_API_VERSION(0, 0, 0, 1),
        .engine_version = VK_MAKE_API_VERSION(0, 0, 0, 1),
        .vulkan_api_version = VK_MAKE_API_VERSION(0, 1, 4, 0),
        .validation_layers_count = 1,
        .validation_layers = desc->required_validation_layers[0], // not really the best solution either, should revisit later
        .extensions_count = window_extension_count,
        .extensions = window_extensions,
    };
    gfx::CreateInstance(&instance_desc, &renderer->instance);

    volkLoadInstance(renderer->instance);
    if (config::VALIDATION_LAYERS_ENABLED) gfx::CreateDebugMessenger(renderer->instance, &renderer->debug_messenger);

    platform::CreateSurface(window, renderer->instance, &renderer->surface);

    gfx::DeviceDesc device_desc = {
        .device_extensions_count = 1,
        .device_extensions = desc->required_device_extensions[0],
    };
    gfx::PickPhysicalDevice(&renderer->device, &device_desc, renderer->instance);
    gfx::CreateLogicalDevice(&renderer->device, &device_desc, renderer->surface);
    volkLoadDevice(renderer->device.logical);
    gfx::GetLogicalQueue(&renderer->device, &renderer->device.queue_family_index);

    gfx::CreateAllocator(&renderer->device, renderer->instance, &renderer->uploader.allocator);

    gfx::SwapchainDesc swapchain_desc = {
        .desired_format = VK_FORMAT_B8G8R8A8_SRGB,
        .desired_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .desired_present_mode = VK_PRESENT_MODE_MAILBOX_KHR,
    };
    renderer->swapchain.desc = swapchain_desc;
    gfx::CreateSwapchain(&renderer->swapchain, window, &renderer->device, renderer->surface);
    gfx::GetSwapchainImages(&renderer->swapchain, renderer->device.logical);
    gfx::CreateSwapchainImageViews(&renderer->swapchain, renderer->device.logical);

    gfx::TextureDesc depth_texture_desc = {
        .width = renderer->swapchain.extent.width,
        .height = renderer->swapchain.extent.height,
        .format = renderer->device.depth_foramt,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .allocation_flags = 0,
    };
    gfx::CreateTexture(renderer->device.logical, renderer->uploader.allocator, &depth_texture_desc, &renderer->depth_texture);
    gfx::CreateTextureView(renderer->device.logical, depth_texture_desc.format, VK_IMAGE_ASPECT_DEPTH_BIT, &renderer->depth_texture);

    gfx::CreateDescriptorSetLayout(renderer->device.logical, &renderer->pipeline);
    gfx::CreatePipelineLayout(renderer->device.logical, &renderer->pipeline);
    gfx::CreateGraphicsPipeline(renderer->device.logical, renderer->device.depth_foramt, renderer->swapchain.surface_format, &renderer->pipeline);

    gfx::BufferDesc uniform_buffer_desc = {
        .size = sizeof(UniformBufferObject),
        .buffer_flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .allocation_flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
    };
    for (int i = 0; i < config::FRAMES_IN_FLIGHT; i++) {
        gfx::CreateCommandPool(&renderer->device, &renderer->frames[i]);
        gfx::AllocateCommandBuffer(renderer->device.logical, &renderer->frames[i]);

        gfx::CreateBuffer(renderer->uploader.allocator, &uniform_buffer_desc, &renderer->frames[i].uniform_buffer);
    }
    evoLog(PrintSeverity::Info, "Created depth buffers");

    CreateSyncObjects(renderer);

    *renderer_handle = renderer;
    return true;
}

// temporary place just to store resource loading for course loading
bool renderer::CreateResources(Renderer* renderer) {
    renderer->uploader.command_pool = &renderer->frames[0].command_pool;
    renderer->uploader.command_buffer = &renderer->frames[0].command_buffer;

    model::Load("assets/patrick.obj", &renderer->vertex_array, &renderer->index_array);

    // vertex buffer
    gfx::BufferDesc vertex_buffer_desc = {
        .size = sizeof(renderer->vertex_array[0]) * renderer->vertex_array.size(),
        .buffer_flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .allocation_flags = 0,
    };
    gfx::CreateBuffer(renderer->uploader.allocator, &vertex_buffer_desc, &renderer->vertex_buffer);

    // index buffer
    gfx::BufferDesc index_buffer_desc = {
        .size = sizeof(renderer->index_array[0]) * renderer->index_array.size(),
        .buffer_flags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .allocation_flags = 0,
    };
    gfx::CreateBuffer(renderer->uploader.allocator, &index_buffer_desc, &renderer->index_buffer);

    // texture
    image::LoadFromFile("assets/patrick.png", &renderer->image);
    gfx::TextureDesc texture_desc = {
        .width = static_cast<uint32_t>(renderer->image.width),
        .height = static_cast<uint32_t>(renderer->image.height),
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .allocation_flags = 0,
    };
    gfx::CreateTexture(renderer->device.logical, renderer->uploader.allocator, &texture_desc, &renderer->texture);
    gfx::CreateTextureView(renderer->device.logical, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, &renderer->texture);

    // sampler
    gfx::CreateSampler(renderer->device.logical, &renderer->device.device_properties, &renderer->sampler);

    gfx::CreateDescriptorPool(renderer->device.logical, &renderer->pipeline);
    for (int i = 0; i < config::FRAMES_IN_FLIGHT; i++) {
        gfx::AllocateDescriptorSets(renderer->device.logical, &renderer->pipeline, &renderer->frames[i].descriptor_set);
        gfx::ConfigureDescriptorSet(renderer->device.logical, &renderer->frames[i].uniform_buffer, renderer->texture.view, renderer->sampler, &renderer->frames[i].descriptor_set);
    }

    // prepare the command pool
    vkResetCommandPool(renderer->device.logical, renderer->frames[0].command_pool, 0);
    VkCommandBufferBeginInfo command_buffer_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(renderer->frames[0].command_buffer, &command_buffer_info);

    // do the actual buffer transfers to fill the texture and buffers.
    // which requires creating texture and buffer handles above, filling them below and submitting them at the end
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    gfx::Buffer vertex_staging_buffer;
    gfx::CreateStagingBuffer(&renderer->uploader, renderer->vertex_array.data(), vertex_buffer_desc.size, &vertex_staging_buffer);
    gfx::CmdCopyDataToBuffer(&renderer->uploader, &vertex_staging_buffer, &renderer->vertex_buffer);

    gfx::Buffer index_staging_buffer;
    gfx::CreateStagingBuffer(&renderer->uploader, renderer->index_array.data(), index_buffer_desc.size, &index_staging_buffer);
    gfx::CmdCopyDataToBuffer(&renderer->uploader, &index_staging_buffer, &renderer->index_buffer);

    gfx::Buffer texture_staging_buffer;
    gfx::CreateStagingBuffer(&renderer->uploader, renderer->image.data, renderer->image.size, &texture_staging_buffer);
    gfx::CmdCopyDataToTexture(&renderer->uploader, &texture_staging_buffer, &renderer->texture);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    vkEndCommandBuffer(renderer->frames[0].command_buffer);
    VkSubmitInfo submit = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &renderer->frames[0].command_buffer,
    };
    vkQueueSubmit(renderer->device.graphics_queue, 1, &submit, nullptr);
    vkQueueWaitIdle(renderer->device.graphics_queue); // wait until submission is 100% complete
    vkResetCommandPool(renderer->device.logical, renderer->frames[0].command_pool, 0);

    gfx::DestroyBuffer(renderer->uploader.allocator, &vertex_staging_buffer);
    gfx::DestroyBuffer(renderer->uploader.allocator, &index_staging_buffer);
    gfx::DestroyBuffer(renderer->uploader.allocator, &texture_staging_buffer);

    return true;
}
void renderer::DestroyResources(Renderer* renderer) {
    vkDeviceWaitIdle(renderer->device.logical);

    gfx::FreeDescriptorSets(renderer->device.logical, renderer->pipeline.descriptor_pool, &renderer->frames[0].descriptor_set);
    gfx::DestroyDescriptorPool(renderer->device.logical, &renderer->pipeline);

    gfx::DestroySampler(renderer->device.logical, &renderer->sampler);
    gfx::DestroyTexture(renderer->device.logical, renderer->uploader.allocator, &renderer->texture);
    image::Free(&renderer->image);

    gfx::DestroyBuffer(renderer->uploader.allocator, &renderer->index_buffer);
    gfx::DestroyBuffer(renderer->uploader.allocator, &renderer->vertex_buffer);

    renderer->index_array.clear();
    renderer->index_array.clear();
}

void renderer::DrawFrame(platform::Window* window, Renderer* renderer, Camera* camera) {
    renderer->delta_time = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count() - renderer->last_frame_time;
    renderer->last_frame_time = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
    // these ↑↑↑ are temporary, delta_time and time in general will be implemented in the engine layer later

    gfx::Frame* frame = &renderer->frames[renderer->current_frame];

    if (!gfx::BeginFrame(window, renderer, camera, frame)) return;

    gfx::BeginRendering(renderer, frame);
    gfx::DrawScene(renderer, frame);
    imgui::Render(renderer);
    gfx::EndFrame(renderer, frame);
}

bool renderer::Resize(platform::Window* window, Renderer* renderer) {
    vkDeviceWaitIdle(renderer->device.logical);

    gfx::DestroySwapchainImageViews(&renderer->swapchain, renderer->device.logical);
    gfx::DestroySwapchain(&renderer->swapchain, renderer->device.logical);
    gfx::DestroyTexture(renderer->device.logical, renderer->uploader.allocator, &renderer->depth_texture);

    gfx::CreateSwapchain(&renderer->swapchain, window, &renderer->device, renderer->surface);
    gfx::GetSwapchainImages(&renderer->swapchain, renderer->device.logical);
    gfx::CreateSwapchainImageViews(&renderer->swapchain, renderer->device.logical);

    // recreate depth texture
    gfx::TextureDesc depth_texture_desc = {
        .width = renderer->swapchain.extent.width,
        .height = renderer->swapchain.extent.height,
        .format = renderer->device.depth_foramt,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .allocation_flags = 0,
    };
    gfx::CreateTexture(renderer->device.logical, renderer->uploader.allocator, &depth_texture_desc, &renderer->depth_texture);
    gfx::CreateTextureView(renderer->device.logical, depth_texture_desc.format, VK_IMAGE_ASPECT_DEPTH_BIT, &renderer->depth_texture);

    // resize (recreate probably) depth texture here as well
    evoLog(PrintSeverity::Info, "Resized renderer");

    window->was_resized = false;

    return true;
}

void renderer::Destroy(platform::Window* window, Renderer** renderer_handle) {
    Renderer* renderer = *renderer_handle;

    vkDeviceWaitIdle(renderer->device.logical);

    DestroySyncObjects(renderer);

    for (int i = 0; i < config::FRAMES_IN_FLIGHT; i++) {
        gfx::DestroyBuffer(renderer->uploader.allocator, &renderer->frames[i].uniform_buffer);

        gfx::FreeCommandBuffer(renderer->device.logical, &renderer->frames[i]);
        gfx::DestroyCommandPool(renderer->device.logical, &renderer->frames[i]);
    }

    gfx::DestroyGraphicsPipeline(renderer->device.logical, &renderer->pipeline);
    gfx::DestroyPipelineLayout(renderer->device.logical, &renderer->pipeline);

    gfx::DestroyDescriptorSetLayout(renderer->device.logical, &renderer->pipeline);
    gfx::DestroyTexture(renderer->device.logical, renderer->uploader.allocator, &renderer->depth_texture);

    gfx::DestroySwapchainImageViews(&renderer->swapchain, renderer->device.logical);
    gfx::DestroySwapchain(&renderer->swapchain, renderer->device.logical);

    gfx::DestroyAllocator(&renderer->uploader.allocator);

    gfx::DestroyLogicalDevice(&renderer->device);
    platform::DestroySurface(window, renderer->instance, &renderer->surface);

    if (config::VALIDATION_LAYERS_ENABLED) gfx::DestroyDebugMessenger(renderer->instance, &renderer->debug_messenger);
    gfx::DestroyInstance(&renderer->instance);

    delete renderer;
    *renderer_handle = nullptr;
}
