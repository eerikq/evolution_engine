#pragma once

#include <volk.h>

namespace gfx {
    struct Instance {
        VkInstance instance = nullptr;
        VkDebugUtilsMessengerEXT debug_messenger = nullptr;
    };

    struct InstanceDesc {
        const char* app_name;
        const char* engine_name;
        const uint32_t application_version;
        const uint32_t engine_version;
        const uint32_t vulkan_api_version;

        const bool enable_validation_layers;
        const uint32_t validation_layers_count;
        const char* validation_layers;

        const uint32_t extensions_count;
        const char* const* extensions;
    };

    bool CreateInstance(Instance* instance, const InstanceDesc* desc);
    void DestroyInstance(Instance* instance);

    bool CreateDebugMessenger(Instance* instance);
    void DestroyDebugMessenger(Instance* instance);
} // namespace gfx
