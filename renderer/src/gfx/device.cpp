#include "device.h"

#include "core/log.h"

#include <cstring>
#include <vector>

static bool IsDeviceSuitable(const VkPhysicalDevice device, const gfx::DeviceDesc* desc) {
    VkPhysicalDeviceProperties2 device_properties {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
    };
    vkGetPhysicalDeviceProperties2(device, &device_properties);

    evoLog(PrintSeverity::Debug, "Evaluating {} for suitability", device_properties.properties.deviceName);

    // check if device supports vulkan 1.4
    if (device_properties.properties.apiVersion < VK_API_VERSION_1_4) {
        evoLog(PrintSeverity::Debug, "{} doesn't support Vulkan 1.4", device_properties.properties.deviceName);
        return false;
    }

    // check if any of the queue families support graphics operations
    uint32_t queue_property_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(device, &queue_property_count, nullptr);

    std::vector<VkQueueFamilyProperties2> queue_properties(queue_property_count);
    for (uint32_t i = 0; i < queue_property_count; i++) {
        queue_properties[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
    }
    vkGetPhysicalDeviceQueueFamilyProperties2(device, &queue_property_count, queue_properties.data());

    bool supports_graphics = false;
    for (uint32_t i = 0; i < queue_property_count; i++) {
        if (queue_properties[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            supports_graphics = true;
            break;
        }
    }
    if (!supports_graphics) {
        evoLog(PrintSeverity::Debug, "{} doesn't support graphics operations", device_properties.properties.deviceName);
        return false;
    }

    // check if all required device extensions are available
    uint32_t extension_property_count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_property_count, nullptr);

    std::vector<VkExtensionProperties> extension_properties(extension_property_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_property_count, extension_properties.data());

    for (uint32_t i = 0; i < desc->device_extensions_count; i++) {
        bool found_extension = false;

        for (uint32_t j = 0; j < extension_property_count; j++) {
            found_extension = strcmp(&desc->device_extensions[i], extension_properties[j].extensionName) == 0;
            if (found_extension) break;
        }

        if (!found_extension) {
            evoLog(PrintSeverity::Debug, "{} doesn't support the required extension: {}", device_properties.properties.deviceName, desc->device_extensions[i]);
            return false;
        }
    }

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT features_dynamic_state {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
    };
    VkPhysicalDeviceVulkan13Features features_vulkan_13 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &features_dynamic_state,
    };
    VkPhysicalDeviceVulkan11Features features_vulkan_11 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = &features_vulkan_13,
    };
    VkPhysicalDeviceFeatures2 device_features {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &features_vulkan_11,
    };

    vkGetPhysicalDeviceFeatures2(device, &device_features);
    bool supports_required_features = features_vulkan_11.shaderDrawParameters && features_vulkan_13.dynamicRendering && features_vulkan_13.synchronization2 &&
                                      features_dynamic_state.extendedDynamicState && device_features.features.samplerAnisotropy;

    if (!supports_required_features) {
        evoLog(PrintSeverity::Debug, "{} doesn't support required features", device_properties.properties.deviceName);
        return false;
    }

    evoLog(PrintSeverity::Info, "Picked physical device: {}", device_properties.properties.deviceName);
    return true;
}

bool gfx::PickPhysicalDevice(gfx::Device* device, const gfx::DeviceDesc* desc, const VkInstance instance) {
    uint32_t physical_device_count = 0;
    vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);

    evoLog(PrintSeverity::Debug, "Found {} physical devices", physical_device_count);

    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

    for (uint32_t i = 0; i < physical_device_count; i++) {
        if (IsDeviceSuitable(physical_devices[i], desc)) {
            device->physical = physical_devices[i];

            vkGetPhysicalDeviceProperties2(device->physical, &device->device_properties);
            vkGetPhysicalDeviceFeatures2(device->physical, &device->device_features);

            return true;
        }
    }

    evoLog(PrintSeverity::Fatal, "Failed to find a suitable GPU");
    return false;
}

bool gfx::CreateLogicalDevice(gfx::Device* device, const gfx::DeviceDesc* desc, const VkSurfaceKHR surface) {
    uint32_t queue_property_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(device->physical, &queue_property_count, nullptr);

    std::vector<VkQueueFamilyProperties2> queue_properties(queue_property_count);
    for (uint32_t i = 0; i < queue_property_count; i++) {
        queue_properties[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
    }
    vkGetPhysicalDeviceQueueFamilyProperties2(device->physical, &queue_property_count, queue_properties.data());

    // get the first index into queueFamilyProperties which supports both graphics and present
    for (uint32_t i = 0; i < queue_property_count; i++) {
        VkBool32 supports_surface = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device->physical, i, surface, &supports_surface);
        bool supports_graphics = queue_properties[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT;

        if (supports_graphics && supports_surface) {
            device->queue_family_index = i;
            break;
        }
    }

    if (device->queue_family_index == VK_QUEUE_FAMILY_IGNORED) {
        evoLog(PrintSeverity::Fatal, "Could not find a queue for graphics and present");
        return false;
    }

    float queuePriority = 0.5f;

    VkDeviceQueueCreateInfo device_queue_create_info {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = device->queue_family_index,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT features_dynamic_state {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
        .extendedDynamicState = VK_TRUE,
    };
    VkPhysicalDeviceVulkan13Features features_vulkan_13 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &features_dynamic_state,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE,
    };
    VkPhysicalDeviceVulkan11Features features_vulkan_11 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = &features_vulkan_13,
        .shaderDrawParameters = VK_TRUE,
    };
    VkPhysicalDeviceFeatures2 device_features {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &features_vulkan_11,
        .features = {.samplerAnisotropy = VK_TRUE},
    };

    VkDeviceCreateInfo device_create_info {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &device_features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &device_queue_create_info,
        .enabledExtensionCount = desc->device_extensions_count,
        .ppEnabledExtensionNames = &desc->device_extensions
    };

    if (vkCreateDevice(device->physical, &device_create_info, nullptr, &device->logical)) {
        evoLog(PrintSeverity::Fatal, "Failed to create logical device");
        return false;
    }

    evoLog(PrintSeverity::Info, "Created logical device");
    return true;
}

void gfx::DestroyLogicalDevice(Device* device) {
    vkDestroyDevice(device->logical, nullptr);
    device->logical = nullptr;
    device->graphics_queue = nullptr;
    device->queue_family_index = 0;

    evoLog(PrintSeverity::Debug, "Destroyed logical device");
}

// void GetPhysicalProperties(const VkPhysicalDevice physical_device, VkPhysicalDeviceProperties2* properties) {
//     vkGetPhysicalDeviceProperties2(physical_device, properties);
// }

// void GetPhysicalFeatures(const VkPhysicalDevice physical_device, VkPhysicalDeviceFeatures2* features) {
//     vkGetPhysicalDeviceFeatures2(physical_device, features);
// }

void gfx::GetLogicalQueue(Device* device, const uint32_t queue_index) {
    vkGetDeviceQueue(device->logical, device->queue_family_index, queue_index, &device->graphics_queue);
}
