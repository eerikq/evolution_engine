#pragma once

#include "renderer_config.h"

#include <volk.h>

namespace platform {
    struct Window;
}

namespace gfx {
    struct Device;

    struct SwapchainDesc {
        VkFormat desired_format = VK_FORMAT_UNDEFINED;
        VkColorSpaceKHR desired_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        VkPresentModeKHR desired_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    };

    struct Swapchain {
        // desc saved in the struct itself so it can recreate itself
        SwapchainDesc desc = {};

        uint32_t image_count = 0;
        VkSwapchainKHR handle = nullptr;
        VkSurfaceFormatKHR surface_format = {};
        VkExtent2D extent = {};

        VkImage images[config::MAX_SWAPCHAIN_IMAGES] = {};
        VkImageView image_views[config::MAX_SWAPCHAIN_IMAGES] = {nullptr};
        VkSemaphore render_finished_semaphores[config::MAX_SWAPCHAIN_IMAGES] = {nullptr};
    };

    bool CreateSwapchain(Swapchain* swapchain, const platform::Window* window, const Device* device, const VkSurfaceKHR surface);
    void DestroySwapchain(Swapchain* swapchain, const VkDevice logical_device);

    bool GetSwapchainImages(Swapchain* swapchain, const VkDevice logical_device);

    bool CreateSwapchainImageViews(Swapchain* swapchain, const VkDevice logical_device);
    void DestroySwapchainImageViews(Swapchain* swapchain, const VkDevice logical_device);
} // namespace gfx
