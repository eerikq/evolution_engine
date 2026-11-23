#pragma once

#include <volk.h>

namespace gfx {
    struct InstanceDesc {
        const char* app_name = "";
        const char* engine_name = "";
        const uint32_t application_version = 0;
        const uint32_t engine_version = 0;
        const uint32_t vulkan_api_version = 0;

        const uint32_t validation_layers_count = 0;
        const char* validation_layers = "";

        const uint32_t extensions_count = 0;
        const char* const* extensions;
    };

    bool CreateInstance(const InstanceDesc* desc, VkInstance* instance);
    void DestroyInstance(VkInstance* instance);

    bool CreateDebugMessenger(const VkInstance instance, VkDebugUtilsMessengerEXT* debug_messenger);
    void DestroyDebugMessenger(const VkInstance instance, VkDebugUtilsMessengerEXT* debug_messenger);
} // namespace gfx
