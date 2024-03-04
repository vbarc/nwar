#include "nvkutil.h"

#include "ngllog.h"

static const char* deviceTypeToString(VkPhysicalDeviceType type) {
    switch (type) {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            return "VK_PHYSICAL_DEVICE_TYPE_OTHER";
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return "VK_PHYSICAL_DEVICE_TYPE_CPU";
        default:
            return "UNKNOWN_PHYSICAL_DEVICE_TYPE";
    }
}

void nvkDumpPhysicalDevices(const std::vector<VkPhysicalDevice>& devices) {
    NGL_LOGI("Physical devices:");
    for (VkPhysicalDevice device : devices) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);

        NGL_LOGI("  Device: %p", reinterpret_cast<void*>(device));
        NGL_LOGI("    apiVersion:       %u", properties.apiVersion);
        NGL_LOGI("    driverVersion:    %u", properties.driverVersion);
        NGL_LOGI("    vendorID:         x%x", properties.vendorID);
        NGL_LOGI("    deviceID:         x%x", properties.deviceID);
        NGL_LOGI("    deviceType:       %s", deviceTypeToString(properties.deviceType));
        NGL_LOGI("    deviceName:       %s", properties.deviceName);
        // NGL_LOGI("    limits:           %s", limitsToString(properties.limits));
        // NGL_LOGI("    sparseProperties: %s", sparcePropertiesToString(properties.sparseProperties));
    }
}
