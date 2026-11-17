#include "core/image.h"
#include "core/log.h"
#include "core/model.h"
#include "gfx/command.h"
#include "gfx/descriptor.h"
#include "gfx/device.h"
#include "gfx/draw.h"
#include "gfx/instance.h"
#include "gfx/pipeline.h"
#include "gfx/resources.h"
#include "gfx/swapchain.h"
#include "gfx/uploader.h"
#include "gfx/vma.h"
#include "platform/platform_sdl3_vulkan.h"
#include "renderer_config.h"
#include "types/ubo.h"
#include "types/vertex.h"

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

    UniformBufferObject ubo;
    // this is NOT staying here, kill on sight after finishing course report
    VkInstance instance = nullptr;
    VkDebugUtilsMessengerEXT debug_messenger = nullptr;

    gfx::Device device;
    gfx::Swapchain swapchain;
    gfx::Pipeline pipeline;
    gfx::Uploader uploader;

    uint32_t current_frame = 0;
    gfx::Frame frames[config::FRAMES_IN_FLIGHT];

    // ↓↓↓ questionable palcements ↓↓↓
    gfx::Texture depth_texture;
    VkSurfaceKHR surface; // ← will probably be part of another struct.
    // or maybe not! surface is weird and doesnt *quite* have a well defined place it belongs to.
};

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

    renderer = *renderer_handle;
    return true;
}

// temporary place just to store resource loading for course loading
void renderer::CreateResources(Renderer* renderer) {
    model::Load("assets/viking_room.obj", &renderer->vertex_array, &renderer->index_array);

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
    gfx::CreateBuffer(renderer->uploader.allocator, &vertex_buffer_desc, &renderer->vertex_buffer);

    // texture
    image::LoadFromFile("assets/viking_room.png", &renderer->image);
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

    gfx::CreateDescriptorPool(renderer->device.logical, &renderer->pipeline); // (required to allocate sets)
    gfx::AllocateDescriptorSets(renderer->device.logical, &renderer->pipeline, &renderer->frames[0].descriptor_set);
    // might need to do gfx::ConfigureDescriptorSets(), which needs fixing as well

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
    vertex_buffer_desc.buffer_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vertex_buffer_desc.allocation_flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    gfx::Buffer vertex_staging_buffer;
    gfx::CreateBuffer(renderer->uploader.allocator, &vertex_buffer_desc, &vertex_staging_buffer);
    // write to buffer

    index_buffer_desc.buffer_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    index_buffer_desc.allocation_flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    gfx::Buffer index_staging_buffer;
    gfx::CreateBuffer(renderer->uploader.allocator, &index_buffer_desc, &index_staging_buffer);
    // write to buffer

    gfx::BufferDesc texture_buffer_desc = {
        .buffer_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .allocation_flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
    };
    gfx::Buffer texture_staging_buffer;
    gfx::CreateBuffer(renderer->uploader.allocator, &index_buffer_desc, &index_staging_buffer);
    // write to texture
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

void renderer::DrawFrame(const platform::Window* window, Renderer* renderer) {
    const gfx::Frame* frame = &renderer->frames[renderer->current_frame];
    uint32_t image_index = 0;

    // check that the GPU isn't doing anything as to not interrupt it
    if (vkWaitForFences(renderer->device.logical, 1, &frame->in_flight_fence, true, UINT32_MAX)) {
        evoLog(PrintSeverity::Error, "Failed waiting for fence");
        return;
    }

    // acquire the next swapchain image that can be drawn to
    VkAcquireNextImageInfoKHR image_acquisition_info = {
        .sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR,
        .swapchain = renderer->swapchain.handle,
        .timeout = UINT32_MAX,
        .semaphore = frame->present_complete_semaphore,
        .fence = nullptr,
        .deviceMask = 1,
    };
    VkResult image_acquisition_result = vkAcquireNextImage2KHR(renderer->device.logical, &image_acquisition_info, &image_index);
    if (image_acquisition_result == VK_ERROR_OUT_OF_DATE_KHR || window->was_resized) {
        vkDeviceWaitIdle(renderer->device.logical);

        renderer::Resize(window, renderer);
        return;
    }

    // reset fence so it can be signaled to again
    vkResetFences(renderer->device.logical, 1, &frame->in_flight_fence);

    // copy uniform buffer object data into the mapped data area in memory
    memcpy(frame->uniform_buffer.mapped_data, &renderer->ubo, sizeof(UniformBufferObject));

    // reset command pool (which wipes all commands its command buffer previously stored) and being command buffer to prepare for a new frame
    vkResetCommandPool(renderer->device.logical, frame->command_pool, 0);

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
        frame->command_buffer, renderer->swapchain.images[image_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT
    );
    gfx::CmdTransitionImageLayout(
        frame->command_buffer, renderer->depth_texture.handle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_NONE,
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, 0, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_DEPTH_BIT
    );

    VkClearValue color_clear_value = {0.0f, 0.0f, 0.0f, 0.0f};
    VkClearValue depth_clear_value = {1.0f, 0};

    VkRenderingAttachmentInfo color_attachment_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = renderer->swapchain.image_views[image_index],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = color_clear_value
    };
    VkRenderingAttachmentInfo depth_attachment_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = renderer->depth_texture.view,
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
                .extent = renderer->swapchain.extent,
            },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_info,
        .pDepthAttachment = &depth_attachment_info,
    };

    vkCmdBeginRendering(frame->command_buffer, &rendering_info);
    vkCmdBindPipeline(frame->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline.handle);

    VkViewport viewport = VkViewport(0.0f, 0.0f, static_cast<float>(renderer->swapchain.extent.width), static_cast<float>(renderer->swapchain.extent.height), 0.0f, 1.0f);
    VkRect2D offset = VkRect2D(VkOffset2D(0, 0), renderer->swapchain.extent);
    vkCmdSetViewport(frame->command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(frame->command_buffer, 0, 1, &offset);

    VkDeviceSize device_size = 0;
    vkCmdBindVertexBuffers(frame->command_buffer, 0, 1, &renderer->vertex_buffer.handle, &device_size);
    vkCmdBindIndexBuffer(frame->command_buffer, renderer->index_buffer.handle, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(frame->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline.layout, 0, 1, &frame->descriptor_set, 0, nullptr);

    vkCmdDrawIndexed(frame->command_buffer, renderer->vertex_array.size(), 1, 0, 0, 0);

    vkCmdEndRendering(frame->command_buffer);

    gfx::CmdTransitionImageLayout(
        frame->command_buffer, renderer->swapchain.images[image_index], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
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
        .pSignalSemaphores = &renderer->swapchain.render_finished_semaphores[image_index] // signal that rendering for this frame is complete
    };
    vkQueueSubmit(renderer->device.graphics_queue, 1, &submit_info, frame->in_flight_fence); // the fence becomes signaled once the queue is submitted

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderer->swapchain.render_finished_semaphores[image_index],
        .swapchainCount = 1,
        .pSwapchains = &renderer->swapchain.handle,
        .pImageIndices = &image_index
    };
    VkResult queue_present_result = vkQueuePresentKHR(renderer->device.graphics_queue, &present_info);
    switch (queue_present_result) {
        case VK_SUCCESS: break;
        case VK_SUBOPTIMAL_KHR: evoLog(PrintSeverity::Warn, "vkQueuePresentKHR returned VK_SUBOPTIMAL_KHR!"); break;
        default: break; // an unexpected result is returned!
    }

    renderer->current_frame = (renderer->current_frame + 1) % config::FRAMES_IN_FLIGHT;
}

bool renderer::Resize(const platform::Window* window, Renderer* renderer) {
    gfx::DestroySwapchainImageViews(&renderer->swapchain, renderer->device.logical);
    gfx::DestroySwapchain(&renderer->swapchain, renderer->device.logical);

    gfx::CreateSwapchain(&renderer->swapchain, window, &renderer->device, renderer->surface);
    gfx::GetSwapchainImages(&renderer->swapchain, renderer->device.logical);
    gfx::CreateSwapchainImageViews(&renderer->swapchain, renderer->device.logical);

    // resize (recreate probably) depth texture here as well
    evoLog(PrintSeverity::Info, "Resized renderer");

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

    delete renderer_handle;
    *renderer_handle = nullptr;
}
