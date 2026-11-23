#pragma once

#include <volk.h>

namespace gfx {
    struct Device;

    struct Pipeline {
        VkPipeline handle;
        VkPipelineLayout layout;

        VkDescriptorPool descriptor_pool;
        VkDescriptorSetLayout descriptor_set_layout;
    };

    bool CreatePipelineLayout(const VkDevice logical_device, Pipeline* pipeline);
    void DestroyPipelineLayout(const VkDevice logical_device, Pipeline* pipeline);

    bool CreateGraphicsPipeline(const VkDevice logical_device, const VkFormat depth_format, const VkSurfaceFormatKHR swapchain_surface_format, Pipeline* pipeline);
    void DestroyGraphicsPipeline(const VkDevice logical_device, Pipeline* pipeline);
} // namespace gfx
