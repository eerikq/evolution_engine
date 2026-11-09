#include "instance.h"

#include "core/log.h"

#include <cstring>
#include <vector>

static bool ValidateInstanceLayers(const uint32_t layers_count, const char* const* layers_names) {
    uint32_t layer_property_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_property_count, nullptr);

    std::vector<VkLayerProperties> layer_properties(layer_property_count);
    vkEnumerateInstanceLayerProperties(&layer_property_count, layer_properties.data());

    for (uint32_t i = 0; i < layers_count; i++) {
        bool found_layer = false;

        for (uint32_t j = 0; j < layer_property_count; j++) {
            found_layer = strcmp(layers_names[i], layer_properties[j].layerName) == 0;
            if (found_layer) break;
        }

        if (!found_layer) {
            evoLog(PrintSeverity::Fatal, "Required layer not supported: {}", layers_names[i]);
            return false;
        }
    }

    return true;
}

static bool ValidateInstanceExtensions(const uint32_t extensions_count, const char* const* extensions_names) {
    uint32_t extension_property_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_property_count, nullptr);

    std::vector<VkExtensionProperties> extension_properties(extension_property_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_property_count, extension_properties.data());

    for (uint32_t i = 0; i < extensions_count; i++) {
        bool found_extension = false;

        for (uint32_t j = 0; j < extension_property_count; j++) {
            found_extension = strcmp(extensions_names[i], extension_properties[j].extensionName) == 0;
            if (found_extension) break;
        }

        if (!found_extension) {
            evoLog(PrintSeverity::Fatal, "Required extension not supported: {}", extensions_names[i]);
            return false;
        }
    }

    return true;
}

bool gfx::CreateInstance(Instance* instance, const InstanceDesc* desc) {
    VkApplicationInfo app_info {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = desc->app_name,
        .applicationVersion = desc->application_version,
        .pEngineName = desc->engine_name,
        .engineVersion = desc->engine_version,
        .apiVersion = desc->vulkan_api_version,
    };

    // instance (validation) layers
    uint32_t validation_layers_count = desc->enable_validation_layers ? desc->validation_layers_count : 0;
    const char* const* validation_layers_names = desc->enable_validation_layers ? &desc->validation_layers : nullptr;

    if (!ValidateInstanceLayers(validation_layers_count, validation_layers_names)) return false;

    std::vector extensions(desc->extensions, desc->extensions + desc->extensions_count);
    if (desc->enable_validation_layers) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    // append "VK_EXT_DEBUG_UTILS_EXTENSION_NAME" to the end of extensions to enable validation layers

    if (!ValidateInstanceExtensions((uint32_t)extensions.size(), extensions.data())) return false;

    VkInstanceCreateInfo create_info {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = validation_layers_count,
        .ppEnabledLayerNames = validation_layers_names,
        .enabledExtensionCount = (uint32_t)extensions.size(),
        .ppEnabledExtensionNames = extensions.data(),
    };

    if (vkCreateInstance(&create_info, nullptr, &instance->instance)) {
        evoLog(PrintSeverity::Fatal, "Failed to create Vulkan instance");
        return false;
    }

    evoLog(PrintSeverity::Info, "Created Vulkan instance. Validation layers {}", desc->enable_validation_layers ? "Enabled" : "Disabled");
    return true;
}

void gfx::DestroyInstance(Instance* instance) {
    vkDestroyInstance(instance->instance, nullptr);
    evoLog(PrintSeverity::Debug, "Destroyed Vulkan instance");
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data) {
    evoLog(PrintSeverity::Warn, "[{}] {}", type, p_callback_data->pMessage);
    return VK_FALSE;
}

bool gfx::CreateDebugMessenger(Instance* instance) {
    VkDebugUtilsMessageSeverityFlagsEXT severity_flags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    VkDebugUtilsMessageTypeFlagsEXT message_type_flags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    VkDebugUtilsMessengerCreateInfoEXT create_info {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = severity_flags,
        .messageType = message_type_flags,
        .pfnUserCallback = &debugCallback,
    };

    if (vkCreateDebugUtilsMessengerEXT(instance->instance, &create_info, nullptr, &instance->debug_messenger)) {
        evoLog(PrintSeverity::Error, "Failed to create Vulkan debug messenger");
        return false;
    }

    evoLog(PrintSeverity::Debug, "Created Vulkan debug messenger");
    return true;
}

void gfx::DestroyDebugMessenger(Instance* instance) {
    vkDestroyDebugUtilsMessengerEXT(instance->instance, instance->debug_messenger, nullptr);
    evoLog(PrintSeverity::Debug, "Destroyed Vulkan debug messenger");
}
