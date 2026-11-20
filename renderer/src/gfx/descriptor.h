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

    bool AllocateDescriptorSets(const VkDevice logical_device, const Pipeline* pipeline, VkDescriptorSet* descriptor_sets, const uint32_t descriptor_set_count = 1);
    void ConfigureDescriptorSet(const VkDevice logical_device, const Buffer* uniform_buffer, const VkImageView texture_view, const VkSampler texture_sampler, VkDescriptorSet* descriptor);
    void FreeDescriptorSets(const VkDevice logical_device, const VkDescriptorPool descriptor_pool, VkDescriptorSet* descriptor_sets, const uint32_t descriptor_set_count = 1);
} // namespace gfx
