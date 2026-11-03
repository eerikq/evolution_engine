#pragma once

#include <volk.h>
VK_DEFINE_HANDLE(VmaAllocator)

namespace gfx {
    struct Device {
        VkPhysicalDevice physical = nullptr;
        VkDevice logical = nullptr;
        VkQueue graphics_queue = nullptr;
        uint32_t queue_family_index = 0;
        VmaAllocator allocator = nullptr;

        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT features_dynamic {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
        };
        VkPhysicalDeviceVulkan13Features features_vulkan_13 {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &features_dynamic,
        };
        VkPhysicalDeviceVulkan11Features features_vulkan_11 {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = &features_vulkan_13,
        };
        VkPhysicalDeviceFeatures2 device_features {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &features_vulkan_11,
        };

        VkPhysicalDeviceProperties2 device_properties {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        };
    };

    struct DeviceDesc {
        uint32_t device_extensions_count;
        const char* device_extensions;
    };

    bool PickPhysicalDevice(Device* device, const DeviceDesc* desc, const VkInstance instance);
    bool CreateLogicalDevice(Device* device, const DeviceDesc* desc, const VkSurfaceKHR surface);
    void DestroyLogicalDevice(Device* device);

    void GetLogicalQueue(Device* device, const uint32_t queue_index = 0);

    // void GetPhysicalProperties(const VkPhysicalDevice physical_device, VkPhysicalDeviceProperties2* properties);
    // void GetPhysicalFeatures(const VkPhysicalDevice physical_device, VkPhysicalDeviceFeatures2* properties);
} // namespace gfx
