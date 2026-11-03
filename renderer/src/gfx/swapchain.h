#pragma once

#include <SDL3/SDL.h>
#include <volk.h>

namespace gfx {
    struct Window;

    bool CreateSwapchain(
        SDL_Window** sdl_window,
        const VkSurfaceKHR surface,

        const VkPhysicalDevice physical_device,
        const VkDevice logical_device,
        VkSwapchainKHR* swapchain,
        VkExtent2D* swapchain_extent,
        VkSurfaceFormatKHR* swapchain_surface_format
    );
    void DestroySwapchain(
        const VkDevice logical_device,
        VkSwapchainKHR* swapchain,
        VkImageView* swapchain_image_views,
        const uint32_t swapchain_image_views_count
    );
    void GetSwapchainImages(const VkDevice logical_device, const VkSwapchainKHR swapchain, VkImage* swapchain_images, uint32_t* swapchain_images_count);

    void CreateSwapchainImageViews(
        const VkDevice logical_device,
        const VkFormat swapchain_surface_format,
        const VkImage* swapchain_images,
        const uint32_t swapchain_images_count,
        VkImageView* swapchain_image_views
    );
    void DestroySwapchainImageViews(const VkDevice logical_device, VkImageView* swapchain_image_views, const uint32_t swapchain_image_views_count);
} // namespace gfx
