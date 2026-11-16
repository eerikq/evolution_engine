#pragma once

#include <volk.h>

namespace gfx {
    struct Buffer;
    struct Texture;

    void CopyBetweenBuffers(const VkCommandBuffer command_buffer, const Buffer* src_buffer, Buffer* dst_buffer);
    void CopyDataToBuffer(const VkCommandBuffer command_buffer, const void* data, Buffer* buffer);
    void CopyDataToTexture(const VkCommandBuffer command_buffer, const void* data, Texture* buffer);
} // namespace gfx
