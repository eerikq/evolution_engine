#pragma once

#include "renderer_config.h"

#include <volk.h>

namespace platform {
    struct Window;
}

namespace gfx {
    struct Device;

    struct SwapchainDesc {
        uint32_t image_count = 2;
        VkFormat desired_format = VK_FORMAT_B8G8R8A8_SRGB;
        VkColorSpaceKHR desired_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        VkPresentModeKHR desired_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
    };

    struct Swapchain {
        SwapchainDesc desc = {};

        VkSwapchainKHR swapchain = nullptr;
        VkSurfaceFormatKHR surface_format = {};
        VkExtent2D extent = {};

        VkImage images[config::MAX_FRAMES_IN_FLIGHT] = {};
        VkImageView image_views[config::MAX_FRAMES_IN_FLIGHT] = {nullptr};

        // ↓↓↓ unclear if they belong here
        VkSemaphore present_complete_semaphores[config::MAX_FRAMES_IN_FLIGHT] = {nullptr};
        VkSemaphore render_finished_semaphores[config::MAX_FRAMES_IN_FLIGHT] = {nullptr};
        VkFence in_flight_fences[config::MAX_FRAMES_IN_FLIGHT] = {nullptr};
    };

    bool CreateSwapchain(Swapchain* swapchain, const platform::Window* window, const Device* device, const VkSurfaceKHR surface);
    void DestroySwapchain(Swapchain* swapchain, const VkDevice logical_device);

    bool GetSwapchainImages(Swapchain* swapchain, const VkDevice logical_device);

    bool CreateSwapchainImageViews(Swapchain* swapchain, const VkDevice logical_device);
    void DestroySwapchainImageViews(Swapchain* swapchain, const VkDevice logical_device);
} // namespace gfx
