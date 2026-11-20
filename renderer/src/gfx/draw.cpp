#include "draw.h"

#include "core/log.h"
#include "evo/platform.h"
#include "evo/renderer.h"
#include "renderer_internal.h"

static void UpdateUniformBuffer(renderer::Renderer* renderer, renderer::Camera* camera) {
    float aspect_ratio = (float)renderer->swapchain.extent.width / (float)renderer->swapchain.extent.height;
    float orthographic_size = camera->orthographic_size;
    float move_velocity = camera->camera_speed * renderer->delta_time;

    glm::vec3 camera_forward = {
        cos(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch)),
        sin(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch)),
        sin(glm::radians(camera->pitch)),
    };
    camera_forward = glm::normalize(camera_forward);

    glm::vec3 camera_up = {camera->up[0], camera->up[1], camera->up[2]};
    glm::vec3 camera_right = {glm::normalize(glm::cross(camera_forward, camera_up))};
    glm::vec3 camera_pos = {camera->pos[0], camera->pos[1], camera->pos[2]};

    if (camera->input_direction[0] != 0) camera_pos += camera->input_direction[0] * move_velocity * camera_right;
    if (camera->input_direction[1] != 0) camera_pos += camera->input_direction[1] * move_velocity * camera_forward;
    if (camera->input_direction[2] != 0) camera_pos += camera->input_direction[2] * move_velocity * camera_up;

    camera->pos[0] = camera_pos.x;
    camera->pos[1] = camera_pos.y;
    camera->pos[2] = camera_pos.z;

    renderer->ubo.camera_position = camera_pos;

    // movel view projection matrices

    renderer->ubo.model = glm::mat4(1.0f);
    // ↓ patrick edge case ↓
    renderer->ubo.model *= glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(camera->right[0], camera->right[1], camera->right[2]));

    renderer->ubo.view = glm::lookAt(camera_pos, camera_pos + camera_forward, camera_up);
    if (camera->is_orthographic) {
        renderer->ubo.proj = glm::ortho(-orthographic_size * aspect_ratio, orthographic_size * aspect_ratio, -orthographic_size, orthographic_size, camera->near_plane, camera->far_plane);
    } else {
        renderer->ubo.proj = glm::perspective(glm::radians(camera->fov), aspect_ratio, camera->near_plane, camera->far_plane);
    }
    renderer->ubo.proj[1][1] *= -1;

    // directional light
    renderer->ubo.sun_direction.x = cos(glm::radians(renderer->sun_direction[0])) * cos(glm::radians(renderer->sun_direction[1]));
    renderer->ubo.sun_direction.y = sin(glm::radians(renderer->sun_direction[0])) * cos(glm::radians(renderer->sun_direction[1]));
    renderer->ubo.sun_direction.z = sin(glm::radians(renderer->sun_direction[1]));
    renderer->ubo.sun_direction = glm::normalize(renderer->ubo.sun_direction);
}

bool gfx::BeginFrame(platform::Window* window, renderer::Renderer* renderer, renderer::Camera* camera, Frame* frame) {
    // check that the GPU isn't doing anything as to not interrupt it
    if (vkWaitForFences(renderer->device.logical, 1, &frame->in_flight_fence, true, UINT64_MAX)) {
        evoLog(PrintSeverity::Error, "Failed waiting for fence");
        return false;
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
    VkResult image_acquisition_result = vkAcquireNextImage2KHR(renderer->device.logical, &image_acquisition_info, &renderer->current_image_index);
    if (image_acquisition_result == VK_ERROR_OUT_OF_DATE_KHR || window->was_resized) {
        renderer::Resize(window, renderer);
        return false;
    }

    // reset fence so it can be signaled to again
    vkResetFences(renderer->device.logical, 1, &frame->in_flight_fence);

    UpdateUniformBuffer(renderer, camera);
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
        frame->command_buffer, renderer->swapchain.images[renderer->current_image_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT
    );
    gfx::CmdTransitionImageLayout(
        frame->command_buffer, renderer->depth_texture.handle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_NONE,
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, 0, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_ASPECT_DEPTH_BIT
    );

    return true;
};

void gfx::BeginRendering(renderer::Renderer* renderer, Frame* frame) {
    VkClearValue color_clear_value = {renderer->clear_color[0], renderer->clear_color[1], renderer->clear_color[2], 1.0};
    VkClearValue depth_clear_value = {1.0f, 0};

    VkRenderingAttachmentInfo color_attachment_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = renderer->swapchain.image_views[renderer->current_image_index],
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
};

void gfx::DrawScene(renderer::Renderer* renderer, Frame* frame) {
    vkCmdBindPipeline(frame->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline.handle);

    VkViewport viewport = VkViewport(0.0f, 0.0f, static_cast<float>(renderer->swapchain.extent.width), static_cast<float>(renderer->swapchain.extent.height), 0.0f, 1.0f);
    VkRect2D offset = VkRect2D(VkOffset2D(0, 0), renderer->swapchain.extent);
    vkCmdSetViewport(frame->command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(frame->command_buffer, 0, 1, &offset);

    VkDeviceSize device_size = 0;
    vkCmdBindVertexBuffers(frame->command_buffer, 0, 1, &renderer->vertex_buffer.handle, &device_size);
    vkCmdBindIndexBuffer(frame->command_buffer, renderer->index_buffer.handle, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(frame->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline.layout, 0, 1, &frame->descriptor_set, 0, nullptr);

    vkCmdDrawIndexed(frame->command_buffer, renderer->index_array.size(), 1, 0, 0, 0);
};

void gfx::EndFrame(renderer::Renderer* renderer, Frame* frame) {
    vkCmdEndRendering(frame->command_buffer);

    gfx::CmdTransitionImageLayout(
        frame->command_buffer, renderer->swapchain.images[renderer->current_image_index], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, 0, VK_IMAGE_ASPECT_COLOR_BIT
    );

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
        .pSignalSemaphores = &renderer->swapchain.render_finished_semaphores[renderer->current_image_index] // signal that rendering for this frame is complete
    };
    vkQueueSubmit(renderer->device.graphics_queue, 1, &submit_info, frame->in_flight_fence); // the fence becomes signaled once the queue is submitted

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderer->swapchain.render_finished_semaphores[renderer->current_image_index],
        .swapchainCount = 1,
        .pSwapchains = &renderer->swapchain.handle,
        .pImageIndices = &renderer->current_image_index
    };
    VkResult queue_present_result = vkQueuePresentKHR(renderer->device.graphics_queue, &present_info);
    switch (queue_present_result) {
        case VK_SUCCESS: break;
        case VK_SUBOPTIMAL_KHR: evoLog(PrintSeverity::Warn, "vkQueuePresentKHR returned VK_SUBOPTIMAL_KHR!"); break;
        default: break; // an unexpected result is returned!
    }

    renderer->current_frame = (renderer->current_frame + 1) % config::FRAMES_IN_FLIGHT;
};
