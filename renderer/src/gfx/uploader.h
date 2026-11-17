#pragma once

#include <volk.h>
//
#include <vk_mem_alloc.h>

namespace gfx {
    struct Buffer;
    struct Texture;

    struct Uploader {
        VmaAllocator allocator = nullptr;
        // these are currently pointers to another pool and buffer instead of owning a pool and buffer because
        // currently there is no need for a full uploader, but is something that will be implemented in the future
        VkCommandPool* command_pool = nullptr;
        VkCommandBuffer* command_buffer = nullptr;
    };

    void CreateStagingBuffer(const Uploader* uploader, const VkDeviceSize size, Buffer* staging_buffer);
    void CopyBetweenBuffers(const Uploader* uploader, const VkDeviceSize size, Buffer* src_buffer, Buffer* dst_buffer, const VkDeviceSize src_offset = 0, const VkDeviceSize dst_offset = 0);

    void CopyDataToBuffer(const Uploader* uploader, const void* data, const VkDeviceSize size, Buffer* staging_buffer, Buffer* buffer, const VkDeviceSize offset = 0);
    void CopyDataToTexture(const Uploader* uploader, const void* data, const VkDeviceSize size, Buffer* staging_buffer, Texture* texture, const VkDeviceSize offset = 0);
} // namespace gfx
