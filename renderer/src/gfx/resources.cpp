#include "resources.h"

#include "core/log.h"

bool gfx::CreateBuffer(const VmaAllocator allocator, const BufferDesc* desc, Buffer* buffer) {
    VkBufferCreateInfo buffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = desc->size,
        .usage = desc->buffer_flags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VmaAllocationCreateInfo allocation_create_info = {
        .flags = desc->allocation_flags,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    VmaAllocationInfo allocation_info;
    if (vmaCreateBuffer(allocator, &buffer_create_info, &allocation_create_info, &buffer->handle, &buffer->allocation, &allocation_info)) {
        evoLog(PrintSeverity::Debug, "Failed to create buffer");
        return false;
    }

    buffer->mapped_data = allocation_info.pMappedData;
    buffer->size = desc->size;

    evoLog(PrintSeverity::Debug, "Created buffer");
    return true;
}
void gfx::DestroyBuffer(const VmaAllocator allocator, Buffer* buffer) {
    vmaDestroyBuffer(allocator, buffer->handle, buffer->allocation);

    buffer->handle = nullptr;
    buffer->mapped_data = nullptr;
    buffer->allocation = nullptr;
    buffer->size = 0;
}

bool gfx::CreateTexture(const VkDevice logical_device, const VmaAllocator allocator, const TextureDesc* desc, Texture* texture) {
    texture->extent = {desc->width, desc->height, 1};

    VkImageCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = desc->format,
        .extent = texture->extent,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = desc->usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VmaAllocationCreateInfo allocation_info = {
        .flags = desc->allocation_flags,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    if (vmaCreateImage(allocator, &create_info, &allocation_info, &texture->handle, &texture->allocation, nullptr)) {
        evoLog(PrintSeverity::Error, "Failed to create texture");
        texture->extent = {};
        return false;
    }

    evoLog(PrintSeverity::Debug, "Created texture");
    return false;
}
void gfx::DestroyTexture(const VkDevice logical_device, const VmaAllocator allocator, Texture* texture) {
    if (texture->view != nullptr) {
        vkDestroyImageView(logical_device, texture->view, nullptr);
        texture->view = nullptr;
    }

    vmaDestroyImage(allocator, texture->handle, texture->allocation);
    texture->handle = nullptr;
    texture->allocation = nullptr;
}

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

bool gfx::CreateTextureView(const VkDevice logical_device, const VkFormat texture_format, const VkImageAspectFlags aspect_flags, Texture* texture) {
    VkImageViewCreateInfo view_create_info {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = texture->handle,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = texture_format,
        .components =
            {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange = {
            .aspectMask = aspect_flags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    if (vkCreateImageView(logical_device, &view_create_info, nullptr, &texture->view)) {
        evoLog(PrintSeverity::Error, "Failed to create texture view");
        return false;
    }

    evoLog(PrintSeverity::Debug, "Created texture view");
    return true;
}
void gfx::DestroyTextureView(const VkDevice logical_device, Texture* texture) {
    vkDestroyImageView(logical_device, texture->view, nullptr);
}

bool gfx::CreateSampler(const VkDevice logical_device, const VkPhysicalDeviceProperties2* device_properties, VkSampler* sampler) {
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
        .maxAnisotropy = device_properties->properties.limits.maxSamplerAnisotropy,
        .compareEnable = false,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = false,
    };

    if (vkCreateSampler(logical_device, &create_info, nullptr, sampler)) {
        evoLog(PrintSeverity::Error, "Failed creating sampler");
        return false;
    }

    evoLog(PrintSeverity::Debug, "Created sampler");
    return true;
}
void gfx::DestroySampler(const VkDevice logical_device, VkSampler* sampler) {
    vkDestroySampler(logical_device, *sampler, nullptr);
    evoLog(PrintSeverity::Debug, "Destroyed sampler");
}
