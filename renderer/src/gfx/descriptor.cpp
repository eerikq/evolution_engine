#include "descriptor.h"

#include "core/log.h"
#include "gfx/pipeline.h"
#include "renderer_config.h"
#include "vulkan/vulkan_core.h"

bool gfx::CreateDescriptorSetLayout(const VkDevice logical_device, Pipeline* pipeline) {
    VkDescriptorSetLayoutBinding ubo_layout_binding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr
    };

    VkDescriptorSetLayoutBinding sampler_layout_binding = {
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr,
    };

    VkDescriptorSetLayoutBinding bindings[2] = {
        ubo_layout_binding,
        sampler_layout_binding,
    };

    VkDescriptorSetLayoutCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 2,
        .pBindings = bindings,
    };

    if (vkCreateDescriptorSetLayout(logical_device, &create_info, nullptr, &pipeline->descriptor_set_layout)) {
        evoLog(PrintSeverity::Fatal, "Failed creating descriptor set layout");
        return false;
    }

    evoLog(PrintSeverity::Info, "Created descriptor set layout");
    return false;
}

void gfx::DestroyDescriptorSetLayout(const VkDevice logical_device, Pipeline* pipeline) {
    vkDestroyDescriptorSetLayout(logical_device, pipeline->descriptor_set_layout, nullptr);
    evoLog(PrintSeverity::Debug, "Destroyed descriptor set layout");
}

bool gfx::CreateDescriptorPool(const VkDevice logical_device, Pipeline* pipeline) {
    // the way i understand this is basically "how many x thing do you have?",
    // in my case for example, its "how many uniforms and samplers are there ever gonna be at one time?".
    // there is only one uniform buffer (mvp matrices and direcitonal lighting/sun), one sampler (one per material),
    // but since there are 2 frames in flight, the pool needs to accomodate n * frames_in_flight
    VkDescriptorPoolSize pool_sizes[2] = {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1 * config::FRAMES_IN_FLIGHT,
        },
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1 * config::FRAMES_IN_FLIGHT,
        },
    };

    VkDescriptorPoolCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = config::FRAMES_IN_FLIGHT,
        .poolSizeCount = 2,
        .pPoolSizes = pool_sizes,
    };

    if (vkCreateDescriptorPool(logical_device, &create_info, nullptr, &pipeline->descriptor_pool)) {
        evoLog(PrintSeverity::Fatal, "Failed to create descriptor pool");
        return false;
    }

    evoLog(PrintSeverity::Info, "Created descriptor pool");
    return true;
}

void gfx::DestroyDescriptorPool(const VkDevice logical_device, Pipeline* pipeline) {
    vkDestroyDescriptorPool(logical_device, pipeline->descriptor_pool, nullptr);
    evoLog(PrintSeverity::Debug, "Destroyed descriptor pool");
}

bool gfx::AllocateDescriptorSets(const VkDevice logical_device, const Pipeline* pipeline, VkDescriptorSet* descriptor_sets, const uint32_t descriptor_set_count) {
    VkDescriptorSetAllocateInfo alloc_info {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pipeline->descriptor_pool,
        .descriptorSetCount = descriptor_set_count,
        .pSetLayouts = &pipeline->descriptor_set_layout,
    };

    if (vkAllocateDescriptorSets(logical_device, &alloc_info, descriptor_sets)) {
        evoLog(PrintSeverity::Fatal, "Failed to allocate descriptor sets");
        return false;
    }

    evoLog(PrintSeverity::Info, "Allocated descriptor sets");
    return true;
}

/*
void gfx::ConfigureDescriptorSets(const VkDevice logical_device, const Buffer* uniform_buffers, const VkImageView texture_view, const VkSampler texture_sampler, Pipeline* descriptor) {
    for (size_t i = 0; i < config::FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo buffer_info {
            .buffer = VkBuffer(),
            .offset = 0,
            .range = sizeof(1),
        };

        VkDescriptorImageInfo image_info {
            .sampler = texture_sampler,
            .imageView = texture_view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };

        VkWriteDescriptorSet descriptor_writes[2] = {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptor->set[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = &buffer_info,
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptor->set[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &image_info,
            },
        };

        vkUpdateDescriptorSets(logical_device, 2, descriptor_writes, 0, nullptr);
    }
}
*/

void gfx::FreeDescriptorSets(const VkDevice logical_device, const VkDescriptorPool descriptor_pool, VkDescriptorSet* descriptor_sets, const uint32_t descriptor_set_count) {
    vkFreeDescriptorSets(logical_device, descriptor_pool, descriptor_set_count, descriptor_sets);
}
