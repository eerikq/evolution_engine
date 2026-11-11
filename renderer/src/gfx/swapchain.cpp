#include "swapchain.h"

#include "core/log.h"
#include "evo/platform.h"
#include "gfx/device.h"

#include <algorithm>
#include <vector>

static uint32_t ChooseSwapMinImageCount(const VkSurfaceCapabilitiesKHR* capabilities, const uint32_t max_frames_in_flight) {
    uint32_t minImageCount = std::max<uint32_t>(max_frames_in_flight, capabilities->minImageCount);

    if ((0 < capabilities->maxImageCount) && (capabilities->maxImageCount < minImageCount)) minImageCount = capabilities->maxImageCount;

    return minImageCount;
}

static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR* capabilities, const uint32_t width, const uint32_t height) {
    if (capabilities->currentExtent.width != UINT32_MAX) return capabilities->currentExtent;

    return {
        std::clamp(width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width), std::clamp(height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height)
    };
}

static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
    const VkSurfaceFormatKHR* available_formats, const uint32_t available_formats_count, const VkFormat desired_format, const VkColorSpaceKHR desired_color_space
) {
    for (uint32_t i = 0; i < available_formats_count; i++) {
        if (available_formats[i].format == desired_format && available_formats[i].colorSpace == desired_color_space) {
            return available_formats[i];
        }
    }

    return available_formats[0];
}

static VkPresentModeKHR ChooseSwapPresentMode(const VkPresentModeKHR* available_present_modes, const uint32_t available_present_modes_count, const VkPresentModeKHR desired_present_mode) {
    for (uint32_t i = 0; i < available_present_modes_count; i++) {
        if (available_present_modes[i] == desired_present_mode) {
            return desired_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR; // fallback to FIFO since its guranteed to be supported on anything that can run Vulkan.
}

bool gfx::CreateSwapchain(Swapchain* swapchain, const platform::Window* window, const Device* device, const VkSurfaceKHR surface) {
    // swapchain min image count & extent
    VkSurfaceCapabilitiesKHR surface_capabilities {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physical, surface, &surface_capabilities);

    uint32_t min_image_count = ChooseSwapMinImageCount(&surface_capabilities, swapchain->desc.image_count);
    swapchain->extent = ChooseSwapExtent(&surface_capabilities, window->width, window->height);

    // swapchain surface format
    uint32_t surface_formats_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical, surface, &surface_formats_count, nullptr);
    std::vector<VkSurfaceFormatKHR> surface_formats(surface_formats_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical, surface, &surface_formats_count, surface_formats.data());

    swapchain->surface_format = ChooseSwapSurfaceFormat(surface_formats.data(), surface_formats.size(), swapchain->desc.desired_format, swapchain->desc.desired_color_space);

    // swapchain present mode
    uint32_t present_modes_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device->physical, surface, &present_modes_count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(present_modes_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device->physical, surface, &present_modes_count, present_modes.data());

    VkPresentModeKHR present_mode = ChooseSwapPresentMode(present_modes.data(), present_modes.size(), swapchain->desc.desired_present_mode);

    VkSwapchainCreateInfoKHR swapchain_create_info {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = min_image_count,
        .imageFormat = swapchain->surface_format.format,
        .imageColorSpace = swapchain->surface_format.colorSpace,
        .imageExtent = swapchain->extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE
    };

    if (vkCreateSwapchainKHR(device->logical, &swapchain_create_info, nullptr, &swapchain->swapchain)) {
        evoLog(PrintSeverity::Fatal, "Failed creating swapchain");
        return false;
    }

    evoLog(PrintSeverity::Info, "{} swapchain", window->was_resized ? "Recreated" : "Created");
    return true;
}

void gfx::DestroySwapchain(Swapchain* swapchain, const VkDevice logical_device) {
    gfx::DestroySwapchainImageViews(swapchain, logical_device);

    // swapchain images are held within the swapchain object, meaning they dont need to be manually deleted.
    vkDestroySwapchainKHR(logical_device, swapchain->swapchain, nullptr);
    swapchain->swapchain = nullptr;
}

bool gfx::GetSwapchainImages(Swapchain* swapchain, const VkDevice logical_device) {
    if (vkGetSwapchainImagesKHR(logical_device, swapchain->swapchain, &swapchain->desc.image_count, swapchain->images)) {
        evoLog(PrintSeverity::Fatal, "Failed to retrieve images from the swapchain");
        return false;
    }

    evoLog(PrintSeverity::Debug, "Retrieved images from the swapchain");
    return true;
}

bool gfx::CreateSwapchainImageViews(Swapchain* swapchain, const VkDevice logical_device) {
    VkImageViewCreateInfo image_view_create_info {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapchain->surface_format.format,
        .components =
            {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    for (uint32_t i = 0; i < swapchain->desc.image_count; i++) {
        image_view_create_info.image = swapchain->images[i];
        if (vkCreateImageView(logical_device, &image_view_create_info, nullptr, &(swapchain->image_views[i]))) {
            evoLog(PrintSeverity::Fatal, "Failed to create image view {}/{}", i, swapchain->desc.image_count);
            return false;
        }
    }

    evoLog(PrintSeverity::Debug, "Created swapchain image views");
    return true;
}

void gfx::DestroySwapchainImageViews(Swapchain* swapchain, const VkDevice logical_device) {
    for (uint32_t i = 0; i < swapchain->desc.image_count; i++) {
        vkDestroyImageView(logical_device, swapchain->image_views[i], nullptr);
        swapchain->image_views[i] = nullptr;
    }
}

// not the swapchain.cpp's job anymore
// void swapchainRecreate() {
//     device.waitIdle();

//     cleanup_swap_chain();
//     create_swap_chain();
//     create_image_views();
// }
