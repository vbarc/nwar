#pragma once

#include <vector>

void nvkAppendDebugExtensionsIfNecessary(std::vector<const char*>& extensions);
void nvkAppendDebugLayersIfNecessary(std::vector<const char*>& layers);
