#pragma once

#include <vulkan/vulkan.h>

#include <vector>

void nvkAppendDebugExtensionsIfNecessary(std::vector<const char*>& extensions);
void nvkAppendDebugLayersIfNecessary(std::vector<const char*>& layers);

void nvkInitDebugIfNecessary(VkInstance instance);
void nvkTerminateDebugIfNecessary(VkInstance instance);

VkDebugUtilsMessengerCreateInfoEXT* nvkPopulateDebugMessengerCreateInfoIfNecessary(
        VkDebugUtilsMessengerCreateInfoEXT* createInfo);
