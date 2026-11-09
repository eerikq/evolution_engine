#pragma once

#include <volk.h>
//
#include <vk_mem_alloc.h>

namespace gfx {
    struct TextureDesc {
        uint32_t width = 0, height = 0;
        VkFormat format = {};
        VkImageUsageFlags usage = 0;
        VmaAllocationCreateFlags allocation_flags = 0;
    };

    struct Texture {
        VkImage data = nullptr;
        VkImageView view = nullptr;
        VmaAllocation allocation = nullptr;
    };

    bool CreateTexture(const VkDevice logical_device, const VmaAllocator allocator, const TextureDesc* desc, Texture* texture);
    void DestroyTexture(const VkDevice logical_device, const VmaAllocator allocator, Texture* texture);

    bool CreateTextureView(const VkDevice logical_device, const VkFormat texture_format, const VkImageAspectFlags aspect_flags, Texture* texture);
    void DestroyTextureView(const VkDevice logical_device, Texture* texture);

    void TransitionImageLayout(
        const VkCommandBuffer command_buffer, const VkImage image, VkPipelineStageFlags2 src_stage_mask, VkPipelineStageFlags2 dst_stage_mask, VkAccessFlags2 src_access_mask,
        VkAccessFlags2 dst_access_mask, VkImageLayout old_layout, VkImageLayout new_layout
    );

    // don't need to be here?
    // bool CreateTextureFromImage(const VkDevice logical_device, const VmaAllocator allocator, const VmaAllocationCreateFlags allocation_flags, const Image* image, Texture* texture);
    // bool LoadImage(const VkDevice logical_device, const VkQueue graphics_queue, const VkCommandPool command_pool, const VmaAllocator allocator, const Image* image, Texture* texture);

} // namespace gfx
