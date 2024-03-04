#include "nvkutil.h"

#include "ngllog.h"

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
        // NGL_LOGI("    deviceType:       %s", deviceTypeToString(properties.deviceType));
        NGL_LOGI("    deviceName:       %s", properties.deviceName);
        // NGL_LOGI("    limits:           %s", limitsToString(properties.limits));
        // NGL_LOGI("    sparseProperties: %s", sparcePropertiesToString(properties.sparseProperties));
    }
}
