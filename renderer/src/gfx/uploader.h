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

    bool CreateStagingBuffer(const Uploader* uploader, const void* data, const VkDeviceSize size, Buffer* staging_buffer);

    void CmdCopyDataToBuffer(const Uploader* uploader, const Buffer* staging_buffer, Buffer* buffer, const VkDeviceSize src_offset = 0, const VkDeviceSize dst_offset = 0);
    void CmdCopyDataToTexture(const Uploader* uploader, const Buffer* staging_buffer, Texture* texture);
} // namespace gfx
