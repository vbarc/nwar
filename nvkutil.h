#pragma once

#include <vulkan/vulkan.h>

#include <vector>

void nvkDumpPhysicalDevices(VkInstance instance);
void nvkDumpQueueFamilies(VkPhysicalDevice device);
void nvkDumpSurfaceCapabilities(const VkSurfaceCapabilitiesKHR capabilities, const char* indent);
void nvkDumpSurfaceFormats(const std::vector<VkSurfaceFormatKHR>& formats, const char* indent);
void nvkDumpPresentModes(const std::vector<VkPresentModeKHR>& modes, const char* indent);
const char* nvkFormatToString(VkFormat format);
