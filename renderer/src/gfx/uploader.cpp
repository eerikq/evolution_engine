#include "uploader.h"

#include "core/log.h"
#include "resources.h"

#include <cassert>

bool gfx::CreateStagingBuffer(const Uploader* uploader, const void* data, const VkDeviceSize size, Buffer* staging_buffer) {
    BufferDesc buffer_desc = {
        .size = size,
        .buffer_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .allocation_flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
    };
    if (!gfx::CreateBuffer(uploader->allocator, &buffer_desc, staging_buffer)) return false;

    vmaCopyMemoryToAllocation(uploader->allocator, data, staging_buffer->allocation, 0, size);

    // this ↑ apparently does the same as this ↓ but better in some way? pretty cool

    // vmaMapMemory(uploader->allocator, staging_buffer->allocation, &staging_buffer->mapped_data);
    // memcpy(staging_buffer->mapped_data, data, size);
    // vmaUnmapMemory(uploader->allocator, staging_buffer->allocation);

    return true;
}

void gfx::CmdCopyDataToBuffer(const Uploader* uploader, const Buffer* staging_buffer, Buffer* buffer, const VkDeviceSize src_offset, const VkDeviceSize dst_offset) {
    VkBufferCopy buffer_copy_region = {
        .srcOffset = src_offset,
        .dstOffset = dst_offset,
        .size = staging_buffer->size,
    };

    vkCmdCopyBuffer(*uploader->command_buffer, staging_buffer->handle, buffer->handle, 1, &buffer_copy_region);
}

void gfx::CmdCopyDataToTexture(const Uploader* uploader, const Buffer* staging_buffer, Texture* texture) {
    gfx::CmdTransitionImageLayout(
        *uploader->command_buffer, texture->handle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT, 0,
        VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT
    );

    VkBufferImageCopy region {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .imageOffset = {0, 0, 0},
        .imageExtent = texture->extent
    };
    vkCmdCopyBufferToImage(*uploader->command_buffer, staging_buffer->handle, texture->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    gfx::CmdTransitionImageLayout(
        *uploader->command_buffer, texture->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_IMAGE_ASPECT_COLOR_BIT
    );
}

/*
bool gfx::LoadImage(const VkDevice logical_device, const VkQueue graphics_queue, const VkCommandPool command_pool, const VmaAllocator allocator, const Image* image, Texture* texture) {
    Buffer staging_buffer = {};
    buffer::Create(
        image->size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, allocator, &staging_buffer
    );

    void* data;
    vmaMapMemory(allocator, staging_buffer.allocation, &data);
    memcpy(data, image->data, image->size);
    vmaUnmapMemory(allocator, staging_buffer.allocation);

    VkCommandBufferAllocateInfo command_buffer_create_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer temp_command_buffer;
    vkAllocateCommandBuffers(logical_device, &command_buffer_create_info, &temp_command_buffer);

    VkCommandBufferBeginInfo command_buffer_begin_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(temp_command_buffer, &command_buffer_begin_info);

    TransitionImageLayout(
        temp_command_buffer, texture->data, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT, 0, VK_ACCESS_2_TRANSFER_WRITE_BIT, VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
        VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );

    VkBufferImageCopy region {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource =
            {
                .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .imageOffset = {0, 0, 0},
        .imageExtent = {static_cast<uint32_t>(image->width), static_cast<uint32_t>(image->height), 1}
    };
    vkCmdCopyBufferToImage(temp_command_buffer, staging_buffer.data, texture->data, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    TransitionImageLayout(
        temp_command_buffer, texture->data, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT,
        VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    vkEndCommandBuffer(temp_command_buffer);

    VkSubmitInfo submit_info {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &temp_command_buffer,
    };
    vkQueueSubmit(graphics_queue, 1, &submit_info, nullptr);
    vkQueueWaitIdle(graphics_queue);

    buffer::Destroy(allocator, &staging_buffer);
    vkFreeCommandBuffers(logical_device, command_pool, 1, &temp_command_buffer);

    return VK_SUCCESS; // simplified return, doesn't know where it failed (it also can't lol). not important right now
}
*/
