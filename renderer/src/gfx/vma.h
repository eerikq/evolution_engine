#pragma once

#include <volk.h>
//
#include <vk_mem_alloc.h>

namespace gfx {
    struct Device;

    bool CreateAllocator(const Device* device, const VkInstance instance, VmaAllocator* allocator);
    void DestroyAllocator(VmaAllocator* allocator);
} // namespace gfx
