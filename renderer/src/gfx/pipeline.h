#pragma once

#include <volk.h>

namespace gfx {
    struct Device;

    struct Pipeline {
        VkPipeline* pipeline;
        VkPipelineLayout* layout;
    };

    bool CreateGraphicsPipeline(const Device* device, const VkSurfaceFormatKHR swapchain_surface_format, const VkDescriptorSetLayout descriptor_set_layout, Pipeline* pipeline);
    void DestroyGraphicsPipeline(const VkDevice logical_device, Pipeline* pipeline);
    void DestroyPipelineLayout(const VkDevice logical_device, Pipeline* pipeline);
} // namespace gfx
