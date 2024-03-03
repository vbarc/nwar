#include "nvkdbg.h"

#include <vulkan/vulkan.h>

#include <cstdio>

#include "nglassert.h"
#include "ngllog.h"
#include "nvkerr.h"

#define __NVK_DEBUG

void nvkAppendDebugExtensionsIfNecessary(std::vector<const char*>& extensions) {
#ifndef __NVK_DEBUG
    if (true) {
        return;
    }
#endif  // __NVK_DEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
}

void nvkAppendDebugLayersIfNecessary(std::vector<const char*>& layers) {
#ifndef __NVK_DEBUG
    if (true) {
        return;
    }
#endif  // __NVK_DEBUG
    layers.push_back("VK_LAYER_KHRONOS_validation");
}

static const char* messageSeverityToString(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity) {
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT";
        default:
            return "UNKNOWN_MESSAGE_SEVERITY";
    }
}

static const char* messageTypeToString(VkDebugUtilsMessageTypeFlagsEXT messageType) {
    switch (messageType) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT";
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT";
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT";
        case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
            return "VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT";
        default:
            return "UNKNOWN_MESSAGE_TYPE";
    }
}

static VkResult createDebugUtilsMessenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator,
                                          VkDebugUtilsMessengerEXT* pMessenger) {
    auto fptr = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (fptr != nullptr) {
        return fptr(instance, pCreateInfo, pAllocator, pMessenger);
    } else {
        return VK_ERROR_FEATURE_NOT_PRESENT;
    }
}

static void destroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                       const VkAllocationCallbacks* pAllocator) {
    auto fptr = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (fptr != nullptr) {
        fptr(instance, messenger, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                    void* /*pUserData*/) {
    const char* severity = messageSeverityToString(messageSeverity);
    const char* type = messageTypeToString(messageType);
    NGL_LOG(severity, "%s: %s", type, pCallbackData->pMessage);
    return VK_FALSE;
}

static VkDebugUtilsMessengerEXT sMessenger = VK_NULL_HANDLE;

void nvkInitDebugIfNecessary(VkInstance instance) {
#ifndef __NVK_DEBUG
    if (true) {
        return;
    }
#endif  // __NVK_DEBUG
    NGL_ASSERT(sMessenger == VK_NULL_HANDLE);
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    NGL_VERIFY(nvkPopulateDebugMessengerCreateInfoIfNecessary(&createInfo));
    NVK_CHECK(createDebugUtilsMessenger(instance, &createInfo, nullptr, &sMessenger));
}

void nvkTerminateDebugIfNecessary(VkInstance instance) {
#ifndef __NVK_DEBUG
    if (true) {
        return;
    }
#endif  // __NVK_DEBUG
    NGL_ASSERT(sMessenger != VK_NULL_HANDLE);
    destroyDebugUtilsMessenger(instance, sMessenger, nullptr);
    sMessenger = VK_NULL_HANDLE;
}

VkDebugUtilsMessengerCreateInfoEXT* nvkPopulateDebugMessengerCreateInfoIfNecessary(
        VkDebugUtilsMessengerCreateInfoEXT* createInfo) {
#ifndef __NVK_DEBUG
    if (true) {
        return nullptr;
    }
#endif  // __NVK_DEBUG
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity =
            /*/ VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |*/
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
    createInfo->pfnUserCallback = debugCallback;
    return createInfo;
}
