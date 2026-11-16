#include "core/image.h"
#include "core/log.h"
#include "core/model.h"
#include "gfx/command.h"
#include "gfx/descriptor.h"
#include "gfx/device.h"
#include "gfx/draw.h"
#include "gfx/instance.h"
#include "gfx/pipeline.h"
#include "gfx/resources/buffer.h"
#include "gfx/resources/texture.h"
#include "gfx/swapchain.h"
#include "gfx/vma.h"
#include "platform/platform_sdl3_vulkan.h"
#include "renderer_config.h"
#include "types/ubo.h"
#include "types/vertex.h"
#include "vulkan/vulkan_core.h"

#include <evo/platform.h>
#include <evo/renderer.h>
#include <vector>

struct renderer::Renderer {
    // this is NOT staying here, kill on sight after finishing course report
    Image image;
    gfx::Texture texture;
    VkSampler sampler;

    gfx::Buffer vertex_buffer;
    std::vector<Vertex> vertex_array;

    gfx::Buffer index_buffer;
    std::vector<uint16_t> index_array;
    // this is NOT staying here, kill on sight after finishing course report

    RendererDesc desc;

    VkInstance instance = nullptr;
    VkDebugUtilsMessengerEXT debug_messenger = nullptr;

    gfx::Device device;
    gfx::Swapchain swapchain;
    gfx::Pipeline pipeline;

    uint32_t current_frame = 0;
    gfx::Frame frames[config::FRAMES_IN_FLIGHT];

    // ↓↓↓ questionable palcements ↓↓↓
    gfx::Texture depth_texture;
    VkSurfaceKHR surface; // ← will probably be part of another struct.
    // or maybe not! surface is weird and doesnt *quite* have a well defined place it belongs to.
};

// temporarily keeping this here just to not have to think about opaque handles or object ownership
static renderer::Renderer renderer_resources;
static UniformBufferObject ubo;

// terrible temp solution but i dont really know where to put this yet
static void CreateSyncObjects() {
    VkSemaphoreCreateInfo semaphore_create_info {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    VkFenceCreateInfo fence_create_info {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (int i = 0; i < config::FRAMES_IN_FLIGHT; i++) {
        vkCreateFence(renderer_resources.device.logical, &fence_create_info, nullptr, &renderer_resources.frames[i].in_flight_fence);
        vkCreateSemaphore(renderer_resources.device.logical, &semaphore_create_info, nullptr, &renderer_resources.frames[i].present_complete_semaphore);
    }
    for (int i = 0; i < renderer_resources.swapchain.image_count; i++) {
        vkCreateSemaphore(renderer_resources.device.logical, &semaphore_create_info, nullptr, &renderer_resources.swapchain.render_finished_semaphores[i]);
    }

    evoLog(PrintSeverity::Info, "Created sync objects");
}
static void DestroySyncObjects() {
    for (int i = 0; i < config::FRAMES_IN_FLIGHT; i++) {
        vkDestroyFence(renderer_resources.device.logical, renderer_resources.frames[i].in_flight_fence, nullptr);
        vkDestroySemaphore(renderer_resources.device.logical, renderer_resources.frames[i].present_complete_semaphore, nullptr);
    }
    for (int i = 0; i < renderer_resources.swapchain.image_count; i++) {
        vkDestroySemaphore(renderer_resources.device.logical, renderer_resources.swapchain.render_finished_semaphores[i], nullptr);
    }

    evoLog(PrintSeverity::Debug, "Destroyed sync objects");
}

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
        .validation_layers_count = 1,
        .validation_layers = renderer_resources.desc.required_validation_layers[0], // not really the best solution either, should revisit later
        .extensions_count = window_extension_count,
        .extensions = window_extensions,
    };
    gfx::CreateInstance(&instance_desc, &renderer_resources.instance);

    volkLoadInstance(renderer_resources.instance);
    if (config::VALIDATION_LAYERS_ENABLED) gfx::CreateDebugMessenger(renderer_resources.instance, &renderer_resources.debug_messenger);

    platform::CreateSurface(window, renderer_resources.instance, &renderer_resources.surface);

    gfx::DeviceDesc device_desc = {
        .device_extensions_count = 1,
        .device_extensions = renderer_resources.desc.required_device_extensions[0],
    };
    gfx::PickPhysicalDevice(&renderer_resources.device, &device_desc, renderer_resources.instance);
    gfx::CreateLogicalDevice(&renderer_resources.device, &device_desc, renderer_resources.surface);
    volkLoadDevice(renderer_resources.device.logical);
    gfx::GetLogicalQueue(&renderer_resources.device, &renderer_resources.device.queue_family_index);

    gfx::CreateAllocator(&renderer_resources.device, renderer_resources.instance);

    gfx::SwapchainDesc swapchain_desc = {
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

    gfx::CreateDescriptorSetLayout(renderer_resources.device.logical, &renderer_resources.pipeline);
    gfx::CreatePipelineLayout(renderer_resources.device.logical, &renderer_resources.pipeline);
    gfx::CreateGraphicsPipeline(renderer_resources.device.logical, renderer_resources.device.depth_foramt, renderer_resources.swapchain.surface_format, &renderer_resources.pipeline);

    gfx::BufferDesc uniform_buffer_desc = {
        .size = sizeof(UniformBufferObject),
        .buffer_flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .allocation_flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
    };
    for (int i = 0; i < config::FRAMES_IN_FLIGHT; i++) {
        gfx::CreateCommandPool(&renderer_resources.device, &renderer_resources.frames[i]);
        gfx::AllocateCommandBuffer(renderer_resources.device.logical, &renderer_resources.frames[i]);

        gfx::CreateBuffer(renderer_resources.device.allocator, &uniform_buffer_desc, &renderer_resources.frames[i].uniform_buffer);
    }
    evoLog(PrintSeverity::Info, "Created depth buffers");

    CreateSyncObjects();

    return true;
}

// temporary place just to store resource loading for course loading
bool renderer::CreateResources(Renderer* resources) {
    model::Load("assets/viking_room.obj", &renderer_resources.vertex_array, &renderer_resources.index_array);

    gfx::BufferDesc vertex_buffer_desc = {
        .size = sizeof(renderer_resources.vertex_array[0]) * renderer_resources.vertex_array.size(),
        .buffer_flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .allocation_flags = 0,
    };
    gfx::CreateBuffer(renderer_resources.device.allocator, &vertex_buffer_desc, &renderer_resources.vertex_buffer);
    // write to buffer

    gfx::BufferDesc index_buffer_desc = {
        .size = sizeof(renderer_resources.index_array[0]) * renderer_resources.index_array.size(),
        .buffer_flags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .allocation_flags = 0,
    };
    gfx::CreateBuffer(renderer_resources.device.allocator, &vertex_buffer_desc, &renderer_resources.vertex_buffer);
    // write to buffer

    image::LoadFromFile("assets/viking_room.png", &renderer_resources.image);
    gfx::TextureDesc texture_desc = {
        .width = static_cast<uint32_t>(renderer_resources.image.width),
        .height = static_cast<uint32_t>(renderer_resources.image.height),
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .allocation_flags = 0,
    };
    gfx::CreateTexture(renderer_resources.device.logical, renderer_resources.device.allocator, &texture_desc, &renderer_resources.texture);
    // write to texture (based on image data)
    gfx::CreateTextureView(renderer_resources.device.logical, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, &renderer_resources.texture);

    VkSamplerCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = true,
        .maxAnisotropy = renderer_resources.device.device_properties.properties.limits.maxSamplerAnisotropy,
        .compareEnable = false,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = false,
    };
    vkCreateSampler(renderer_resources.device.logical, &create_info, nullptr, &renderer_resources.sampler);

    // gfx::CreateDescriptorSets
    // create descriptor sets

    return true;
}
void renderer::DestroyResources(Renderer* resources) {
    // destroy descriptor sets

    vkDestroySampler(renderer_resources.device.logical, renderer_resources.sampler, nullptr);

    gfx::DestroyTexture(renderer_resources.device.logical, renderer_resources.device.allocator, &renderer_resources.texture);
    image::Free(&renderer_resources.image);

    gfx::DestroyBuffer(renderer_resources.device.allocator, &renderer_resources.index_buffer);
    gfx::DestroyBuffer(renderer_resources.device.allocator, &renderer_resources.vertex_buffer);

    renderer_resources.index_array.clear();
    renderer_resources.index_array.clear();
}

void renderer::DrawFrame(Renderer* resources, platform::Window* window) {
    const gfx::Frame* frame = &renderer_resources.frames[renderer_resources.current_frame];
    uint32_t image_index = 0;

    // check that the GPU isn't doing anything as to not interrupt it
    if (vkWaitForFences(renderer_resources.device.logical, 1, &frame->in_flight_fence, true, UINT32_MAX)) {
        evoLog(PrintSeverity::Error, "Failed waiting for fence");
        return;
    }

    // acquire the next swapchain image that can be drawn to
    VkAcquireNextImageInfoKHR image_acquisition_info = {
        .sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
        .swapchain = renderer_resources.swapchain.handle,
        .timeout = UINT32_MAX,
        .semaphore = frame->present_complete_semaphore,
        .fence = nullptr,
        .deviceMask = 1,
    };
    VkResult image_acquisition_result = vkAcquireNextImage2KHR(renderer_resources.device.logical, &image_acquisition_info, &image_index);
    if (image_acquisition_result == VK_ERROR_OUT_OF_DATE_KHR || window->was_resized) {
        vkDeviceWaitIdle(renderer_resources.device.logical);

        renderer::Resize(&renderer_resources, window);
        return;
    }

    // reset fence so it can be signaled to again
    vkResetFences(renderer_resources.device.logical, 1, &frame->in_flight_fence);

    // copy uniform buffer object data into the mapped data area in memory
    memcpy(frame->uniform_buffer.mapped_data, &ubo, sizeof(UniformBufferObject));

    // reset command pool (which wipes all commands its command buffer previously stored) and being command buffer to prepare for a new frame
    vkResetCommandPool(renderer_resources.device.logical, frame->command_pool, 0);

    VkCommandBufferBeginInfo command_buffer_info = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(frame->command_buffer, &command_buffer_info);

    // ----------------------------------------------------------------------------------------------------------------------
    // parameter       | color                  | depth                   | my understanding
    // ----------------------------------------------------------------------------------------------------------------------
    // old layout      | undefined              | undefined               | doesn't matter, start from scratch
    // new layout      | color attachment       | depth attachment        | self-explanatory
    // src stage mask  | color output           | none                    | "who am i waiting on?", which is "wait_destionation_stage_mask" below saying "im done, you can use color output"
    // dst stage mask  | color output           | early & late frag tests | "who needs to wait for me?" or "which other stages need to wait for me to finish writing color and depth testing?"
    // src access mask | 0                      | 0                       | no prior write caches need flushing
    // dst access mask | color attachment write | depth attachment write  | invalidate/prepare the attachment write caches so clears and fragment outputs can write cleanly
    // ----------------------------------------------------------------------------------------------------------------------
    gfx::CmdTransitionImageLayout(
        frame->command_buffer, renderer_resources.swapchain.images[image_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT
    );
    gfx::CmdTransitionImageLayout(
        frame->command_buffer, renderer_resources.depth_texture.handle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_NONE,
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, 0, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_DEPTH_BIT
    );

    VkClearValue color_clear_value = {0.0f, 0.0f, 0.0f, 0.0f};
    VkClearValue depth_clear_value = {1.0f, 0};

    VkRenderingAttachmentInfo color_attachment_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = renderer_resources.swapchain.image_views[image_index],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = color_clear_value
    };
    VkRenderingAttachmentInfo depth_attachment_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = renderer_resources.depth_texture.view,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue = depth_clear_value
    };

    VkRenderingInfo rendering_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea =
            {
                .offset = {0, 0},
                .extent = renderer_resources.swapchain.extent,
            },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_info,
        .pDepthAttachment = &depth_attachment_info,
    };

    vkCmdBeginRendering(frame->command_buffer, &rendering_info);
    vkCmdBindPipeline(frame->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer_resources.pipeline.handle);

    VkViewport viewport = VkViewport(0.0f, 0.0f, static_cast<float>(renderer_resources.swapchain.extent.width), static_cast<float>(renderer_resources.swapchain.extent.height), 0.0f, 1.0f);
    VkRect2D offset = VkRect2D(VkOffset2D(0, 0), renderer_resources.swapchain.extent);
    vkCmdSetViewport(frame->command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(frame->command_buffer, 0, 1, &offset);

    VkDeviceSize device_size = 0;
    vkCmdBindVertexBuffers(frame->command_buffer, 0, 1, &renderer_resources.vertex_buffer.handle, &device_size);
    vkCmdBindIndexBuffer(frame->command_buffer, renderer_resources.index_buffer.handle, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(frame->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer_resources.pipeline.layout, 0, 1, &frame->descriptor_set, 0, nullptr);

    vkCmdDrawIndexed(frame->command_buffer, renderer_resources.vertex_array.size(), 1, 0, 0, 0);

    vkCmdEndRendering(frame->command_buffer);

    gfx::CmdTransitionImageLayout(
        frame->command_buffer, renderer_resources.swapchain.images[image_index], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT
    );

    // not done yet, but assumedly ImGui goes after everything
    // drawImgui(command_buffers[*frame_index], swapchain_extent, swapchain_image_views, image_index);

    vkEndCommandBuffer(frame->command_buffer);

    VkPipelineStageFlags wait_destionation_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frame->present_complete_semaphore, // wait for the previous frame to complete presenting before this frame gets presented
        .pWaitDstStageMask = &wait_destionation_stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &frame->command_buffer, // submit the right command buffer for this frame
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &renderer_resources.swapchain.render_finished_semaphores[image_index] // signal that rendering for this frame is complete
    };
    vkQueueSubmit(renderer_resources.device.graphics_queue, 1, &submit_info, frame->in_flight_fence); // the fence becomes signaled once the queue is submitted

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderer_resources.swapchain.render_finished_semaphores[image_index],
        .swapchainCount = 1,
        .pSwapchains = &renderer_resources.swapchain.handle,
        .pImageIndices = &image_index
    };
    VkResult queue_present_result = vkQueuePresentKHR(renderer_resources.device.graphics_queue, &present_info);
    switch (queue_present_result) {
        case VK_SUCCESS: break;
        case VK_SUBOPTIMAL_KHR: evoLog(PrintSeverity::Warn, "vkQueuePresentKHR returned VK_SUBOPTIMAL_KHR!"); break;
        default: break; // an unexpected result is returned!
    }

    renderer_resources.current_frame = (renderer_resources.current_frame + 1) % config::FRAMES_IN_FLIGHT;
}

bool renderer::Resize(Renderer* resources, platform::Window* window) {
    gfx::DestroySwapchainImageViews(&renderer_resources.swapchain, renderer_resources.device.logical);
    gfx::DestroySwapchain(&renderer_resources.swapchain, renderer_resources.device.logical);

    gfx::CreateSwapchain(&renderer_resources.swapchain, window, &renderer_resources.device, renderer_resources.surface);
    gfx::GetSwapchainImages(&renderer_resources.swapchain, renderer_resources.device.logical);
    gfx::CreateSwapchainImageViews(&renderer_resources.swapchain, renderer_resources.device.logical);

    // resize (recreate probably) depth texture here as well

    window->was_resized = false;
    evoLog(PrintSeverity::Info, "Resized renderer");

    return true;
}

void renderer::Destroy(Renderer* resources, platform::Window* window) {
    vkDeviceWaitIdle(renderer_resources.device.logical);

    DestroySyncObjects();

    for (int i = 0; i < config::FRAMES_IN_FLIGHT; i++) {
        gfx::DestroyBuffer(renderer_resources.device.allocator, &renderer_resources.frames[i].uniform_buffer);

        gfx::FreeCommandBuffer(renderer_resources.device.logical, &renderer_resources.frames[i]);
        gfx::DestroyCommandPool(renderer_resources.device.logical, &renderer_resources.frames[i]);
    }

    gfx::DestroyGraphicsPipeline(renderer_resources.device.logical, &renderer_resources.pipeline);
    gfx::DestroyPipelineLayout(renderer_resources.device.logical, &renderer_resources.pipeline);

    gfx::DestroyDescriptorSetLayout(renderer_resources.device.logical, &renderer_resources.pipeline);
    gfx::DestroyTexture(renderer_resources.device.logical, renderer_resources.device.allocator, &renderer_resources.depth_texture);

    gfx::DestroySwapchainImageViews(&renderer_resources.swapchain, renderer_resources.device.logical);
    gfx::DestroySwapchain(&renderer_resources.swapchain, renderer_resources.device.logical);

    gfx::DestroyAllocator(&renderer_resources.device);

    gfx::DestroyLogicalDevice(&renderer_resources.device);
    platform::DestroySurface(window, renderer_resources.instance, &renderer_resources.surface);

    if (config::VALIDATION_LAYERS_ENABLED) gfx::DestroyDebugMessenger(renderer_resources.instance, &renderer_resources.debug_messenger);
    gfx::DestroyInstance(&renderer_resources.instance);
}
