#pragma once

#include "renderer_config.h"

#include <volk.h>

namespace gfx {
    struct Buffer;

    struct Descriptor {
        VkDescriptorSetLayout set_layout;
        VkDescriptorPool pool;
        VkDescriptorSet set[config::MAX_FRAMES_IN_FLIGHT];
    };

    bool CreateDescriptorSetLayout(const VkDevice logical_device, Descriptor* descriptor);
    void DestroyDescriptorSetLayout(const VkDevice logical_device, Descriptor* descriptor);

    bool CreateDescriptorPool(const VkDevice logical_device, const uint32_t frames_in_flight, Descriptor* descriptor);
    void DestroyDescriptorPool(const VkDevice logical_device, Descriptor* descriptor);

    bool CreateDescriptorSets(const VkDevice logical_device, Descriptor* descriptor);
    void ConfigureDescriptorSets(const VkDevice logical_device, const gfx::Buffer* uniform_buffers, const VkImageView texture_view, const VkSampler texture_sampler, Descriptor* descriptor);
    void DestroyDescriptorSets(const VkDevice logical_device, Descriptor* descriptor);
} // namespace gfx
