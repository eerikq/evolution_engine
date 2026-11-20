#include "vma.h"

#include "core/log.h"
#include "gfx/device.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

bool gfx::CreateAllocator(const Device* device, const VkInstance instance, VmaAllocator* allocator) {
    VmaVulkanFunctions vulkan_functions = {
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
    };

    VmaAllocatorCreateInfo create_info = {
        .flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
        .physicalDevice = device->physical,
        .device = device->logical,
        .pVulkanFunctions = &vulkan_functions,
        .instance = instance,
        .vulkanApiVersion = VK_API_VERSION_1_4,
    };

    if (vmaCreateAllocator(&create_info, allocator)) {
        evoLog(PrintSeverity::Fatal, "Failed to create Vulkan Memory Allocator");
        return false;
    }

    evoLog(PrintSeverity::Info, "Created Vulkan Memory Allocator");
    return true;
}

void gfx::DestroyAllocator(VmaAllocator* allocator) {
    vmaDestroyAllocator(*allocator);
    evoLog(PrintSeverity::Debug, "Destroyed Vulkan Memory Allocator");
}
