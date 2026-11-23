#include "core/log.h"
#include "gfx/draw.h"
#include "imgui_impl_sdlrenderer3.h"
#include "renderer_internal.h"

#include <evo/platform.h>
#include <evo/renderer.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <volk.h>

bool renderer::imgui::CreateContext(const platform::Window* window, const Renderer* renderer) {
    ImGui::CreateContext();
    if (!ImGui_ImplSDL3_InitForVulkan(window->window)) {
        evoLog(PrintSeverity::Error, "Failed initializing ImGui");
        return false;
    }

    VkDescriptorPoolSize pool_sizes[11] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
    };

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000,
        .poolSizeCount = 11,
        .pPoolSizes = pool_sizes,
    };

    VkDescriptorPool imgui_pool;
    if (vkCreateDescriptorPool(renderer->device.logical, &pool_info, nullptr, &imgui_pool)) return false;

    ImGui_ImplVulkan_InitInfo init_info = {
        .Instance = renderer->instance,
        .PhysicalDevice = renderer->device.physical,
        .Device = renderer->device.logical,
        .Queue = renderer->device.graphics_queue,
        .DescriptorPool = imgui_pool,
        .MinImageCount = 3,
        .ImageCount = 3,
        .PipelineInfoMain =
            {
                .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
                .PipelineRenderingCreateInfo =
                    {
                        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                        .colorAttachmentCount = 1,
                        .pColorAttachmentFormats = &renderer->swapchain.surface_format.format,
                    },
            },
        .UseDynamicRendering = true
    };

    ImGui_ImplVulkan_Init(&init_info);
    return true;
}
void renderer::imgui::DestroyContext(const Renderer* renderer) {
    ImGui::DestroyContext();
    ImGui_ImplSDLRenderer3_Shutdown();
}

void renderer::imgui::Process(Renderer* renderer, Camera* camera) {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Debug Info", nullptr, 0);
    ////////////////////////////////////////////////////////////////
    ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);

    ImGui::Separator();

    ImGui::Text("Position: %.2f, %.2f, %.2f", camera->pos[0], camera->pos[1], camera->pos[2]);
    ImGui::Text("Pitch: %.2f", camera->pitch);
    ImGui::Text("Yaw: %.2f", camera->yaw);
    ImGui::Text("Input Direction: %d,%d,%d", camera->input_direction[0], camera->input_direction[1], camera->input_direction[2]);

    ImGui::Separator();

    ImGui::Checkbox("Is Orthographic", &camera->is_orthographic);
    if (camera->is_orthographic) {
        ImGui::SliderFloat("Orthographic Size", &camera->orthographic_size, 0.0f, 8.0f, "%.2f");
    } else {
        ImGui::SliderFloat("FOV", &camera->fov, 0.0f, 120.0f, "%.2f");
    }
    ImGui::SliderFloat("Sensitivity", &camera->sensitivity, camera->sensitivity_min, camera->sensitivity_max, "%.2f");
    ImGui::SliderFloat("Camera Speed", &camera->camera_speed, camera->camera_speed_min, camera->camera_speed_max, "%.2f");

    ImGui::Separator();
    ImGui::ColorEdit3("Sky Color", &renderer->clear_color[0]);
    ImGui::ColorEdit3("Sun Color", &renderer->ubo.sun_color.x);
    ImGui::SliderFloat2("Sun Direction", &renderer->sun_direction[0], -180.0f, 180.0f, "%.2f");
    ImGui::SliderFloat("Sun Intensity", &renderer->ubo.sun_intensity, 0.0f, 2.0f, "%.2f");
    ImGui::SliderFloat("Ambient Intensity", &renderer->ubo.ambient_intensity, 0.0f, 2.0f, "%.2f");
    ImGui::SliderFloat("Shininess", &renderer->ubo.shininess, 0.0f, 8.0f, "%.2f");
    ////////////////////////////////////////////////////////////////
    ImGui::End();
    ImGui::Render();
}

void renderer::imgui::Render(Renderer* renderer) {
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderer->frames[renderer->current_frame].command_buffer);
};
