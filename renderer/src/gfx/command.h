#pragma once

#include <volk.h>

namespace gfx {
    struct Frame;
    struct Device;

    bool CreateCommandPool(const Device* device, Frame* frame);
    void DestroyCommandPool(const VkDevice logical_device, Frame* frame);

    bool AllocateCommandBuffer(const VkDevice logical_device, Frame* frame);
    void FreeCommandBuffer(const VkDevice logical_device, Frame* frame);

    void CmdTransitionImageLayout(
        const VkCommandBuffer command_buffer, const VkImage image, const VkImageLayout old_layout, const VkImageLayout new_layout, const VkPipelineStageFlags2 src_stage_mask,
        const VkPipelineStageFlags2 dst_stage_mask, const VkAccessFlags2 src_access_mask, const VkAccessFlags2 dst_access_mask, const VkImageAspectFlags aspect_flags
    );
} // namespace gfx
