#pragma once

#include <vulkan/vulkan.h>

#include <vector>

void nvkDumpPhysicalDevices(const std::vector<VkPhysicalDevice>& devices);
