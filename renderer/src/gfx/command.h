#pragma once

#include <volk.h>

namespace gfx {
    struct Device;

    struct CommandDesc {
        static constexpr uint32_t buffer_count = 3;
    };

    struct Command {
        CommandDesc desc;

        VkCommandPool pool;
        VkCommandBuffer buffers[CommandDesc::buffer_count];
    };

    bool CreateCommandPool(const Device* device, Command* command);
    void DestroyCommandPool(const Device* device, Command* command);

    bool AllocateCommandBuffers(const Device* device, Command* command);
    void FreeCommandBuffers(const Device* device, Command* command);

    // this shit gotta go man, what the hell
    // void commandRecordBuffer(
    //     const uint32_t frame_index, const uint32_t image_index, const VkCommandBuffer* command_buffers, const VkImage* swapchain_images, const VkImageView* swapchain_image_views,
    //     const VkImage* depth_image, const VkImageView* depth_image_view, const VkPipeline graphics_pipeline, const VkPipelineLayout pipeline_layout, const VkDescriptorSet* descriptor_sets,
    //     const VkExtent2D swapchain_extent, const VkBuffer* vertex_buffer, const VkBuffer* index_buffer, const uint32_t index_buffer_size
    // );
} // namespace gfx
