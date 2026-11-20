#pragma once
#include "core/image.h"
#include "gfx/device.h"
#include "gfx/draw.h"
#include "gfx/pipeline.h"
#include "gfx/resources.h"
#include "gfx/swapchain.h"
#include "gfx/uploader.h"
#include "types/ubo.h"
#include "types/vertex.h"

namespace renderer {
    struct Renderer {
        // this is NOT staying here, kill on sight after finishing course report
        Image image;
        gfx::Texture texture;
        VkSampler sampler;

        gfx::Buffer vertex_buffer;
        std::vector<Vertex> vertex_array;

        gfx::Buffer index_buffer;
        std::vector<uint16_t> index_array;

        UniformBufferObject ubo;

        double last_frame_time = 0;
        double delta_time = 0;
        float clear_color[3] = {0.2157f, 0.5490f, 1.0f};
        float sun_direction[2] = {0}; // 1: yaw. 2: pitch.
        // this is NOT staying here, kill on sight after finishing course report

        // these three might get bundled into a gfx::instance?
        VkSurfaceKHR surface = nullptr;
        VkInstance instance = nullptr;
        VkDebugUtilsMessengerEXT debug_messenger = nullptr;

        gfx::Device device;
        gfx::Swapchain swapchain;
        gfx::Pipeline pipeline;
        gfx::Uploader uploader;

        uint32_t current_frame = 0;
        uint32_t current_image_index = 0;
        gfx::Frame frames[config::FRAMES_IN_FLIGHT];

        // should technically be inside gfx::Swapchain since their lifetimes are very similar, both get resized and all.
        // but i need to figure out a way to have an instance of a gfx::Texture in gfx::Swapchain without including resources.h
        gfx::Texture depth_texture;
    };
} // namespace renderer
