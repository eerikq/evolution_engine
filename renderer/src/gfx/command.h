#pragma once

#include <volk.h>

namespace gfx {
    struct Frame;
    struct Device;

    bool CreateCommandPool(const Device* device, Frame* frame);
    void DestroyCommandPool(const VkDevice logical_device, Frame* frame);

    bool AllocateCommandBuffer(const VkDevice logical_device, Frame* frame);
    void FreeCommandBuffer(const VkDevice logical_device, Frame* frame);
} // namespace gfx
