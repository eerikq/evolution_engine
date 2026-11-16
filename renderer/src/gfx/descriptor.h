#pragma once

#include <volk.h>

namespace gfx {
    struct Pipeline;
    struct Buffer;
    struct Frame;

    bool CreateDescriptorSetLayout(const VkDevice logical_device, Pipeline* pipeline);
    void DestroyDescriptorSetLayout(const VkDevice logical_device, Pipeline* pipeline);

    bool CreateDescriptorPool(const VkDevice logical_device, Pipeline* pipeline);
    void DestroyDescriptorPool(const VkDevice logical_device, Pipeline* pipeline);
    /*
    bool CreateDescriptorSets(const VkDevice logical_device, const Pipeline* pipeline, Frame* frame);
    void ConfigureDescriptorSets(const VkDevice logical_device, const gfx::Buffer* uniform_buffers, const VkImageView texture_view, const VkSampler texture_sampler, Pipeline* pipeline);
    void DestroyDescriptorSets(const VkDevice logical_device, const Pipeline* pipeline, Frame* frame);
    */
} // namespace gfx
