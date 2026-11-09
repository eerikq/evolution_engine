#pragma once

#include <volk.h>
//
#include <vk_mem_alloc.h>

namespace gfx {
    struct BufferDesc {
        VkDeviceSize size;
        VkBufferUsageFlags buffer_flags;
        VmaAllocationCreateFlags allocation_flags;
    };

    struct Buffer {
        VkBuffer data = nullptr;
        VmaAllocation allocation = nullptr;
    };

    bool CreateBuffer(const VmaAllocator allocator, const BufferDesc* desc, Buffer* buffer);
    void DestroyBuffer(const VmaAllocator allocator, Buffer* buffer);

    // these also will probably move to the same place which implements upload to texture
    // bool CopyFromBuffer(const VkQueue graphics_queue, const VkCommandPool command_pool, const VkDevice logical_device, const Buffer source, const Buffer destination, const VkDeviceSize
    // size); bool WriteToBuffer(
    //     const VmaAllocator allocator, const VkQueue graphics_queue, const VkCommandPool command_pool, const VkDevice logical_device, const VkDeviceSize data_size, const void* data,
    //     Buffer* buffer
    // );
} // namespace gfx
