#pragma once

#include "gfx/resources.h"

#include <volk.h>

namespace gfx {
    struct Frame {
        VkCommandPool command_pool;
        VkCommandBuffer command_buffer;
        VkDescriptorSet descriptor_set;
        VkSemaphore present_complete_semaphore;
        VkFence in_flight_fence;
        Buffer uniform_buffer;
    };
}; // namespace gfx

/*
void drawFrame(
    int* frame_index,
    const VkDevice logical_device,
    const VkFence* in_flight_fences,
    const uint32_t in_flight_fences_count,
    const VkSemaphore* present_complete_semaphores,
    const VkSemaphore* render_finished_semaphores,
    const VkSwapchainKHR swapchain,
    const VkQueue graphics_queue,
    const VkCommandBuffer* command_buffers,
    const VkImage* swapchain_images,
    const VkImageView* swapchain_image_views,
    const VkImage* depth_image,
    const VkImageView* depth_image_view,
    const VkPipeline graphics_pipeline,
    const VkPipelineLayout pipeline_layout,
    const VkDescriptorSet* descriptor_sets,
    const VkExtent2D swapchain_extent,
    const VkBuffer* vertex_buffer,
    const VkBuffer* index_buffer,
    void** uniform_buffers_mapped,
    const uint32_t index_buffer_size,
    const ShaderVariables shader_variables
);
*/
