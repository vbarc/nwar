#include "nvkdbg.h"

#include <vulkan/vulkan.h>

#include <cstdio>

#define __NVK_DEBUG_ENABLED

void nvkAppendDebugExtensionsIfNecessary(std::vector<const char*>& extensions) {
#ifdef __NVK_DEBUG_ENABLED
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif  // __NVK_DEBUG_ENABLED
}

void nvkAppendDebugLayersIfNecessary(std::vector<const char*>& layers) {
#ifdef __NVK_DEBUG_ENABLED
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif  // __NVK_DEBUG_ENABLED
}
