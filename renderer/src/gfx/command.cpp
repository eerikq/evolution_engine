#include "command.h"

#include "core/log.h"
#include "gfx/device.h"
#include "gfx/draw.h"
#include "renderer_config.h"

bool gfx::CreateCommandPool(const Device* device, Frame* frame) {
    VkCommandPoolCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = device->queue_family_index,
    };

    if (vkCreateCommandPool(device->logical, &create_info, nullptr, &frame->command_pool)) {
        evoLog(PrintSeverity::Fatal, "Failed to create command pool");
        return false;
    }

    evoLog(PrintSeverity::Info, "Created command pool");
    return true;
}

void gfx::DestroyCommandPool(const VkDevice logical_device, Frame* frame) {
    vkDestroyCommandPool(logical_device, frame->command_pool, nullptr);
    frame->command_pool = nullptr;

    evoLog(PrintSeverity::Debug, "Destroyed command pool");
}

bool gfx::AllocateCommandBuffer(const VkDevice logical_device, Frame* frame) {
    VkCommandBufferAllocateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = (frame->command_pool),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    if (vkAllocateCommandBuffers(logical_device, &create_info, &frame->command_buffer)) {
        evoLog(PrintSeverity::Fatal, "Failed to allocate command buffer");
        return false;
    }

    evoLog(PrintSeverity::Info, "Allocated command buffer");
    return true;
}

void gfx::FreeCommandBuffer(const VkDevice logical_device, Frame* frame) {
    for (int i = 0; i < config::FRAMES_IN_FLIGHT; i++) {
        vkFreeCommandBuffers(logical_device, frame->command_pool, 1, &frame->command_buffer);
        frame->command_buffer = nullptr;
    }

    evoLog(PrintSeverity::Debug, "Freed command buffers");
}

static void transition_image_layout(
    VkImageLayout old_layout, VkImageLayout new_layout, VkAccessFlags2 src_access_mask, VkAccessFlags2 dst_access_mask, VkImageAspectFlags aspect_flags

);

void gfx::CmdTransitionImageLayout(
    const VkCommandBuffer command_buffer, const VkImage image, const VkImageLayout old_layout, const VkImageLayout new_layout, const VkPipelineStageFlags2 src_stage_mask,
    const VkPipelineStageFlags2 dst_stage_mask, const VkAccessFlags2 src_access_mask, const VkAccessFlags2 dst_access_mask, const VkImageAspectFlags aspect_flags
) {
    VkImageMemoryBarrier2 barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = aspect_flags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };
    VkDependencyInfo dependency_info = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier,
    };

    vkCmdPipelineBarrier2(command_buffer, &dependency_info);
}

// rest is getting removed probably
/*
void commandRecordBuffer(
    const uint32_t frame_index, const uint32_t image_index, const VkCommandBuffer* command_buffers, const VkImage* swapchain_images, const VkImageView* swapchain_image_views,
    const VkImage* depth_image, const VkImageView* depth_image_view, const VkPipeline graphics_pipeline, const VkPipelineLayout pipeline_layout, const VkDescriptorSet* descriptor_sets,
    const VkExtent2D swapchain_extent, const VkBuffer* vertex_buffer, const VkBuffer* index_buffer, const uint32_t index_buffer_size
) {
    VkCommandBuffer command_buffer = command_buffers[frame_index];

    VkCommandBufferBeginInfo begin_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };
    vkBeginCommandBuffer(command_buffer, &begin_info);

    transition_image_layout(
        command_buffers, swapchain_images, frame_index, image_index, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, {}, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_IMAGE_ASPECT_COLOR_BIT
    );

    transition_image_layout(
        command_buffers, depth_image, frame_index, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, VK_IMAGE_ASPECT_DEPTH_BIT
    );

    VkClearValue color_clear_value {0.0f, 0.0f, 0.0f, 0.0f};
    VkClearValue depth_clear_value {1.0f, 0};

    VkRenderingAttachmentInfo color_attachment_info {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = swapchain_image_views[image_index],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = color_clear_value
    };

    VkRenderingAttachmentInfo depth_attachment_info {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = *depth_image_view,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue = depth_clear_value
    };

    VkRenderingInfo rendering_info {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea =
            {
                .offset = {0, 0},
                .extent = swapchain_extent,
            },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_info,
        .pDepthAttachment = &depth_attachment_info,
    };

    vkCmdBeginRendering(command_buffer, &rendering_info);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

    transition_image_layout(
        command_buffers, swapchain_images, frame_index, image_index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, {},
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_IMAGE_ASPECT_COLOR_BIT
    );

    transition_image_layout(
        command_buffers, swapchain_images, frame_index, image_index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, {},
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_IMAGE_ASPECT_COLOR_BIT
    );

    VkViewport viewport = VkViewport(0.0f, 0.0f, static_cast<float>(swapchain_extent.width), static_cast<float>(swapchain_extent.height), 0.0f, 1.0f);
    VkRect2D offset = VkRect2D(VkOffset2D(0, 0), swapchain_extent);
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &offset);

    VkDeviceSize device_size = 0;
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffer, &device_size);
    vkCmdBindIndexBuffer(command_buffer, *index_buffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_sets[frame_index], 0, nullptr);

    vkCmdDrawIndexed(command_buffer, index_buffer_size, 1, 0, 0, 0);
    vkCmdEndRendering(command_buffer);

    transition_image_layout(
        command_buffers, swapchain_images, frame_index, image_index, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, {},
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_IMAGE_ASPECT_COLOR_BIT
    );
}
*/
