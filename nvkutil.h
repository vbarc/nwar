#pragma once

#include <vulkan/vulkan.h>

#include <vector>

void nvkDumpPhysicalDevices(VkInstance instance);
void nvkDumpQueueFamilies(VkPhysicalDevice device);
