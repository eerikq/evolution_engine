#pragma once

#include <volk.h>

namespace gfx {
    struct Device;

    bool CreateAllocator(Device* device, const VkInstance instance);
    void DestroyAllocator(Device* device);
} // namespace gfx
