#pragma once

#include <volk.h>
//
#include <vk_mem_alloc.h>

namespace gfx {
    struct BufferDesc {
        VkDeviceSize size = 0;
        VkBufferUsageFlags buffer_flags = 0;
        VmaAllocationCreateFlags allocation_flags = 0;
    };

    struct TextureDesc {
        const uint32_t width = 0, height = 0;
        const VkFormat format = {};
        const VkImageUsageFlags usage = 0;
        const VmaAllocationCreateFlags allocation_flags = 0;
    };

    struct Buffer {
        VkBuffer handle = nullptr;
        void* mapped_data = nullptr;
        VmaAllocation allocation = nullptr;
        VkDeviceSize size = 0;
    };

    struct Texture {
        VkImage handle = nullptr;
        VkImageView view = nullptr;
        VmaAllocation allocation = nullptr;
    };

    bool CreateBuffer(const VmaAllocator allocator, const BufferDesc* desc, Buffer* buffer);
    void DestroyBuffer(const VmaAllocator allocator, Buffer* buffer);

    bool CreateTexture(const VkDevice logical_device, const VmaAllocator allocator, const TextureDesc* desc, Texture* texture);
    void DestroyTexture(const VkDevice logical_device, const VmaAllocator allocator, Texture* texture);

    bool CreateTextureView(const VkDevice logical_device, const VkFormat texture_format, const VkImageAspectFlags aspect_flags, Texture* texture);
    void DestroyTextureView(const VkDevice logical_device, Texture* texture);

    bool CreateSampler(const VkDevice logical_device, const VkPhysicalDeviceProperties2* device_properties, VkSampler* sampler);
    void DestroySampler(const VkDevice logical_device, VkSampler* sampler);
} // namespace gfx
