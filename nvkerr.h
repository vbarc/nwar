#pragma once

#include <vulkan/vulkan.h>

#define NVK_CHECK(cmd) nvkCheck(cmd, #cmd, __FILE__, __LINE__)

void nvkCheck(VkResult result, const char* command, const char* file, int line);
