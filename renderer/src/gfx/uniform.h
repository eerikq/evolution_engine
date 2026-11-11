#pragma once

#include "gfx/resources/buffer.h"
#include "types/shader_variables.h"

#include <volk.h>

namespace gfx {
    // VkResult CreateBuffers(const VmaAllocator allocator, Buffer* uniform_buffers, void** uniform_buffers_mapped);
    void UpdateUniformBuffers(const ShaderVariables variables, const int frame_index, const VkExtent2D swapchain_extent, void** uniform_buffers_mapped);
} // namespace gfx
