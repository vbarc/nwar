#include "nvkutil.h"

#include <cinttypes>
#include <string>

#include "ngllog.h"
#include "nvkerr.h"

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

static std::string sampleCountFlagsToString(VkSampleCountFlags flags) {
    std::string result = "";
    for (int i = 0; i < 8; i++) {
        int n = 1 << i;
        if (flags & n) {
            if (!result.empty()) {
                result += ',';
            }
            result += std::to_string(n);
        }
    }
    return std::to_string(flags) + " (" + result + ")";
}

static std::string queueFlagsToString(VkQueueFlags flags) {
    std::string result = "";

    auto append = [&result](const char* s) {
        if (!result.empty()) {
            result += ", ";
        }
        result += s;
    };

    if (flags & VK_QUEUE_GRAPHICS_BIT) {
        append("GRAPHICS");
    }
    if (flags & VK_QUEUE_COMPUTE_BIT) {
        append("COMPUTE");
    }
    if (flags & VK_QUEUE_TRANSFER_BIT) {
        append("TRANSFER");
    }
    if (flags & VK_QUEUE_SPARSE_BINDING_BIT) {
        append("SPARSE_BINDING");
    }
    if (flags & VK_QUEUE_PROTECTED_BIT) {
        append("PROTECTED");
    }
    if (flags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
        append("VIDEO_DECODE");
    }
    if (flags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
        append("VIDEO_ENCODE");
    }
    if (flags & VK_QUEUE_OPTICAL_FLOW_BIT_NV) {
        append("OPTICAL_FLOW");
    }

    return result;
}

void nvkDumpPhysicalDevices(VkInstance instance) {
    NGL_LOGI("Physical devices:");

    uint32_t deviceCount;
    NVK_CHECK(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (VkPhysicalDevice device : devices) {
        NGL_LOGI("  Device: %p", reinterpret_cast<void*>(device));

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        NGL_LOGI("    apiVersion:       %u", properties.apiVersion);
        NGL_LOGI("    driverVersion:    %u", properties.driverVersion);
        NGL_LOGI("    vendorID:         x%x", properties.vendorID);
        NGL_LOGI("    deviceID:         x%x", properties.deviceID);
        NGL_LOGI("    deviceType:       %s", deviceTypeToString(properties.deviceType));
        NGL_LOGI("    deviceName:       %s", properties.deviceName);

        NGL_LOGI("    limits:");
        const VkPhysicalDeviceLimits& limits = properties.limits;

        NGL_LOGI("      maxImageDimension1D:                             %u", limits.maxImageDimension1D);
        NGL_LOGI("      maxImageDimension2D:                             %u", limits.maxImageDimension2D);
        NGL_LOGI("      maxImageDimension3D:                             %u", limits.maxImageDimension3D);
        NGL_LOGI("      maxImageDimensionCube:                           %u", limits.maxImageDimensionCube);
        NGL_LOGI("      maxImageArrayLayers:                             %u", limits.maxImageArrayLayers);
        NGL_LOGI("      maxTexelBufferElements:                          %u", limits.maxTexelBufferElements);
        NGL_LOGI("      maxUniformBufferRange:                           %u", limits.maxUniformBufferRange);
        NGL_LOGI("      maxStorageBufferRange:                           %u", limits.maxStorageBufferRange);
        NGL_LOGI("      maxPushConstantsSize:                            %u", limits.maxPushConstantsSize);
        NGL_LOGI("      maxMemoryAllocationCount:                        %u", limits.maxMemoryAllocationCount);
        NGL_LOGI("      maxSamplerAllocationCount:                       %u", limits.maxSamplerAllocationCount);
        NGL_LOGI("      bufferImageGranularity:                          %" PRIu64, limits.bufferImageGranularity);
        NGL_LOGI("      sparseAddressSpaceSize:                          %" PRIu64, limits.sparseAddressSpaceSize);
        NGL_LOGI("      maxBoundDescriptorSets:                          %u", limits.maxBoundDescriptorSets);
        NGL_LOGI("      maxPerStageDescriptorSamplers:                   %u", limits.maxPerStageDescriptorSamplers);
        NGL_LOGI("      maxPerStageDescriptorUniformBuffers:             %u",
                 limits.maxPerStageDescriptorUniformBuffers);
        NGL_LOGI("      maxPerStageDescriptorStorageBuffers:             %u",
                 limits.maxPerStageDescriptorStorageBuffers);
        NGL_LOGI("      maxPerStageDescriptorSampledImages:              %u",
                 limits.maxPerStageDescriptorSampledImages);
        NGL_LOGI("      maxPerStageDescriptorStorageImages:              %u",
                 limits.maxPerStageDescriptorStorageImages);
        NGL_LOGI("      maxPerStageDescriptorInputAttachments:           %u",
                 limits.maxPerStageDescriptorInputAttachments);
        NGL_LOGI("      maxPerStageResources:                            %u", limits.maxPerStageResources);
        NGL_LOGI("      maxDescriptorSetSamplers:                        %u", limits.maxDescriptorSetSamplers);
        NGL_LOGI("      maxDescriptorSetUniformBuffers:                  %u", limits.maxDescriptorSetUniformBuffers);
        NGL_LOGI("      maxDescriptorSetUniformBuffersDynamic:           %u",
                 limits.maxDescriptorSetUniformBuffersDynamic);
        NGL_LOGI("      maxDescriptorSetStorageBuffers:                  %u", limits.maxDescriptorSetStorageBuffers);
        NGL_LOGI("      maxDescriptorSetStorageBuffersDynamic:           %u",
                 limits.maxDescriptorSetStorageBuffersDynamic);
        NGL_LOGI("      maxDescriptorSetSampledImages:                   %u", limits.maxDescriptorSetSampledImages);
        NGL_LOGI("      maxDescriptorSetStorageImages:                   %u", limits.maxDescriptorSetStorageImages);
        NGL_LOGI("      maxDescriptorSetInputAttachments:                %u", limits.maxDescriptorSetInputAttachments);
        NGL_LOGI("      maxVertexInputAttributes:                        %u", limits.maxVertexInputAttributes);
        NGL_LOGI("      maxVertexInputBindings:                          %u", limits.maxVertexInputBindings);
        NGL_LOGI("      maxVertexInputAttributeOffset:                   %u", limits.maxVertexInputAttributeOffset);
        NGL_LOGI("      maxVertexInputBindingStride:                     %u", limits.maxVertexInputBindingStride);
        NGL_LOGI("      maxVertexOutputComponents:                       %u", limits.maxVertexOutputComponents);
        NGL_LOGI("      maxTessellationGenerationLevel:                  %u", limits.maxTessellationGenerationLevel);
        NGL_LOGI("      maxTessellationPatchSize:                        %u", limits.maxTessellationPatchSize);
        NGL_LOGI("      maxTessellationControlPerVertexInputComponents:  %u",
                 limits.maxTessellationControlPerVertexInputComponents);
        NGL_LOGI("      maxTessellationControlPerVertexOutputComponents: %u",
                 limits.maxTessellationControlPerVertexOutputComponents);
        NGL_LOGI("      maxTessellationControlPerPatchOutputComponents:  %u",
                 limits.maxTessellationControlPerPatchOutputComponents);
        NGL_LOGI("      maxTessellationControlTotalOutputComponents:     %u",
                 limits.maxTessellationControlTotalOutputComponents);
        NGL_LOGI("      maxTessellationEvaluationInputComponents:        %u",
                 limits.maxTessellationEvaluationInputComponents);
        NGL_LOGI("      maxTessellationEvaluationOutputComponents:       %u",
                 limits.maxTessellationEvaluationOutputComponents);
        NGL_LOGI("      maxGeometryShaderInvocations:                    %u", limits.maxGeometryShaderInvocations);
        NGL_LOGI("      maxGeometryInputComponents:                      %u", limits.maxGeometryInputComponents);
        NGL_LOGI("      maxGeometryOutputComponents:                     %u", limits.maxGeometryOutputComponents);
        NGL_LOGI("      maxGeometryOutputVertices:                       %u", limits.maxGeometryOutputVertices);
        NGL_LOGI("      maxGeometryTotalOutputComponents:                %u", limits.maxGeometryTotalOutputComponents);
        NGL_LOGI("      maxFragmentInputComponents:                      %u", limits.maxFragmentInputComponents);
        NGL_LOGI("      maxFragmentOutputAttachments:                    %u", limits.maxFragmentOutputAttachments);
        NGL_LOGI("      maxFragmentDualSrcAttachments:                   %u", limits.maxFragmentDualSrcAttachments);
        NGL_LOGI("      maxFragmentCombinedOutputResources:              %u",
                 limits.maxFragmentCombinedOutputResources);
        NGL_LOGI("      maxComputeSharedMemorySize:                      %u", limits.maxComputeSharedMemorySize);
        NGL_LOGI("      maxComputeWorkGroupCount:                        %u, %u, %u",
                 limits.maxComputeWorkGroupCount[0], limits.maxComputeWorkGroupCount[1],
                 limits.maxComputeWorkGroupCount[2]);
        NGL_LOGI("      maxComputeWorkGroupInvocations:                  %u", limits.maxComputeWorkGroupInvocations);
        NGL_LOGI("      maxComputeWorkGroupSize:                         %u, %u, %u", limits.maxComputeWorkGroupSize[0],
                 limits.maxComputeWorkGroupSize[1], limits.maxComputeWorkGroupSize[2]);
        NGL_LOGI("      subPixelPrecisionBits:                           %u", limits.subPixelPrecisionBits);
        NGL_LOGI("      subTexelPrecisionBits:                           %u", limits.subTexelPrecisionBits);
        NGL_LOGI("      mipmapPrecisionBits:                             %u", limits.mipmapPrecisionBits);
        NGL_LOGI("      maxDrawIndexedIndexValue:                        %u", limits.maxDrawIndexedIndexValue);
        NGL_LOGI("      maxDrawIndirectCount:                            %u", limits.maxDrawIndirectCount);
        NGL_LOGI("      maxSamplerLodBias:                               %0.3f", limits.maxSamplerLodBias);
        NGL_LOGI("      maxSamplerAnisotropy:                            %0.3f", limits.maxSamplerAnisotropy);
        NGL_LOGI("      maxViewports:                                    %u", limits.maxViewports);
        NGL_LOGI("      maxViewportDimensions:                           %u, %u", limits.maxViewportDimensions[0],
                 limits.maxViewportDimensions[1]);
        NGL_LOGI("      viewportBoundsRange:                             %0.3f, %0.3f", limits.viewportBoundsRange[0],
                 limits.viewportBoundsRange[1]);
        NGL_LOGI("      viewportSubPixelBits:                            %u", limits.viewportSubPixelBits);
        NGL_LOGI("      minMemoryMapAlignment:                           %zd", limits.minMemoryMapAlignment);
        NGL_LOGI("      minTexelBufferOffsetAlignment:                   %" PRIu64,
                 limits.minTexelBufferOffsetAlignment);
        NGL_LOGI("      minUniformBufferOffsetAlignment:                 %" PRIu64,
                 limits.minUniformBufferOffsetAlignment);
        NGL_LOGI("      minStorageBufferOffsetAlignment:                 %" PRIu64,
                 limits.minStorageBufferOffsetAlignment);
        NGL_LOGI("      minTexelOffset:                                  %d", limits.minTexelOffset);
        NGL_LOGI("      maxTexelOffset:                                  %u", limits.maxTexelOffset);
        NGL_LOGI("      minTexelGatherOffset:                            %d", limits.minTexelGatherOffset);
        NGL_LOGI("      maxTexelGatherOffset:                            %u", limits.maxTexelGatherOffset);
        NGL_LOGI("      minInterpolationOffset:                          %0.3f", limits.minInterpolationOffset);
        NGL_LOGI("      maxInterpolationOffset:                          %0.3f", limits.maxInterpolationOffset);
        NGL_LOGI("      subPixelInterpolationOffsetBits:                 %u", limits.subPixelInterpolationOffsetBits);
        NGL_LOGI("      maxFramebufferWidth:                             %u", limits.maxFramebufferWidth);
        NGL_LOGI("      maxFramebufferHeight:                            %u", limits.maxFramebufferHeight);
        NGL_LOGI("      maxFramebufferLayers:                            %u", limits.maxFramebufferLayers);
        NGL_LOGI("      framebufferColorSampleCounts:                    %s",
                 sampleCountFlagsToString(limits.framebufferColorSampleCounts).c_str());
        NGL_LOGI("      framebufferDepthSampleCounts:                    %s",
                 sampleCountFlagsToString(limits.framebufferDepthSampleCounts).c_str());
        NGL_LOGI("      framebufferStencilSampleCounts:                  %s",
                 sampleCountFlagsToString(limits.framebufferStencilSampleCounts).c_str());
        NGL_LOGI("      framebufferNoAttachmentsSampleCounts:            %s",
                 sampleCountFlagsToString(limits.framebufferNoAttachmentsSampleCounts).c_str());
        NGL_LOGI("      maxColorAttachments:                             %u", limits.maxColorAttachments);
        NGL_LOGI("      sampledImageColorSampleCounts:                   %s",
                 sampleCountFlagsToString(limits.sampledImageColorSampleCounts).c_str());
        NGL_LOGI("      sampledImageIntegerSampleCounts:                 %s",
                 sampleCountFlagsToString(limits.sampledImageIntegerSampleCounts).c_str());
        NGL_LOGI("      sampledImageDepthSampleCounts:                   %s",
                 sampleCountFlagsToString(limits.sampledImageDepthSampleCounts).c_str());
        NGL_LOGI("      sampledImageStencilSampleCounts:                 %s",
                 sampleCountFlagsToString(limits.sampledImageStencilSampleCounts).c_str());
        NGL_LOGI("      storageImageSampleCounts:                        %s",
                 sampleCountFlagsToString(limits.storageImageSampleCounts).c_str());
        NGL_LOGI("      maxSampleMaskWords:                              %u", limits.maxSampleMaskWords);
        NGL_LOGI("      timestampComputeAndGraphics:                     %d", limits.timestampComputeAndGraphics);
        NGL_LOGI("      timestampPeriod:                                 %0.3f", limits.timestampPeriod);
        NGL_LOGI("      maxClipDistances:                                %u", limits.maxClipDistances);
        NGL_LOGI("      maxCullDistances:                                %u", limits.maxCullDistances);
        NGL_LOGI("      maxCombinedClipAndCullDistances:                 %u", limits.maxCombinedClipAndCullDistances);
        NGL_LOGI("      discreteQueuePriorities:                         %u", limits.discreteQueuePriorities);
        NGL_LOGI("      pointSizeRange:                                  %0.3f, %0.3f", limits.pointSizeRange[0],
                 limits.pointSizeRange[1]);
        NGL_LOGI("      lineWidthRange:                                  %0.3f, %0.3f", limits.lineWidthRange[0],
                 limits.lineWidthRange[1]);
        NGL_LOGI("      pointSizeGranularity:                            %0.3f", limits.pointSizeGranularity);
        NGL_LOGI("      lineWidthGranularity:                            %0.3f", limits.lineWidthGranularity);
        NGL_LOGI("      strictLines:                                     %d", limits.strictLines);
        NGL_LOGI("      standardSampleLocations:                         %d", limits.standardSampleLocations);
        NGL_LOGI("      optimalBufferCopyOffsetAlignment:                %" PRIu64,
                 limits.optimalBufferCopyOffsetAlignment);
        NGL_LOGI("      optimalBufferCopyRowPitchAlignment:              %" PRIu64,
                 limits.optimalBufferCopyRowPitchAlignment);
        NGL_LOGI("      nonCoherentAtomSize:                             %" PRIu64, limits.nonCoherentAtomSize);

        const VkPhysicalDeviceSparseProperties& sparseProperties = properties.sparseProperties;
        NGL_LOGI("    sparseProperties:");
        NGL_LOGI("      residencyStandard2DBlockShape:            %d", sparseProperties.residencyStandard2DBlockShape);
        NGL_LOGI("      residencyStandard2DMultisampleBlockShape: %d",
                 sparseProperties.residencyStandard2DMultisampleBlockShape);
        NGL_LOGI("      residencyStandard3DBlockShape:            %d", sparseProperties.residencyStandard3DBlockShape);
        NGL_LOGI("      residencyAlignedMipSize:                  %d", sparseProperties.residencyAlignedMipSize);
        NGL_LOGI("      residencyNonResidentStrict:               %d", sparseProperties.residencyNonResidentStrict);

        NGL_LOGI("    features:");
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);

        NGL_LOGI("      robustBufferAccess:                      %d", features.robustBufferAccess);
        NGL_LOGI("      fullDrawIndexUint32:                     %d", features.fullDrawIndexUint32);
        NGL_LOGI("      imageCubeArray:                          %d", features.imageCubeArray);
        NGL_LOGI("      independentBlend:                        %d", features.independentBlend);
        NGL_LOGI("      geometryShader:                          %d", features.geometryShader);
        NGL_LOGI("      tessellationShader:                      %d", features.tessellationShader);
        NGL_LOGI("      sampleRateShading:                       %d", features.sampleRateShading);
        NGL_LOGI("      dualSrcBlend:                            %d", features.dualSrcBlend);
        NGL_LOGI("      logicOp:                                 %d", features.logicOp);
        NGL_LOGI("      multiDrawIndirect:                       %d", features.multiDrawIndirect);
        NGL_LOGI("      drawIndirectFirstInstance:               %d", features.drawIndirectFirstInstance);
        NGL_LOGI("      depthClamp:                              %d", features.depthClamp);
        NGL_LOGI("      depthBiasClamp:                          %d", features.depthBiasClamp);
        NGL_LOGI("      fillModeNonSolid:                        %d", features.fillModeNonSolid);
        NGL_LOGI("      depthBounds:                             %d", features.depthBounds);
        NGL_LOGI("      wideLines:                               %d", features.wideLines);
        NGL_LOGI("      largePoints:                             %d", features.largePoints);
        NGL_LOGI("      alphaToOne:                              %d", features.alphaToOne);
        NGL_LOGI("      multiViewport:                           %d", features.multiViewport);
        NGL_LOGI("      samplerAnisotropy:                       %d", features.samplerAnisotropy);
        NGL_LOGI("      textureCompressionETC2:                  %d", features.textureCompressionETC2);
        NGL_LOGI("      textureCompressionASTC_LDR:              %d", features.textureCompressionASTC_LDR);
        NGL_LOGI("      textureCompressionBC:                    %d", features.textureCompressionBC);
        NGL_LOGI("      occlusionQueryPrecise:                   %d", features.occlusionQueryPrecise);
        NGL_LOGI("      pipelineStatisticsQuery:                 %d", features.pipelineStatisticsQuery);
        NGL_LOGI("      vertexPipelineStoresAndAtomics:          %d", features.vertexPipelineStoresAndAtomics);
        NGL_LOGI("      fragmentStoresAndAtomics:                %d", features.fragmentStoresAndAtomics);
        NGL_LOGI("      shaderTessellationAndGeometryPointSize:  %d", features.shaderTessellationAndGeometryPointSize);
        NGL_LOGI("      shaderImageGatherExtended:               %d", features.shaderImageGatherExtended);
        NGL_LOGI("      shaderStorageImageExtendedFormats:       %d", features.shaderStorageImageExtendedFormats);
        NGL_LOGI("      shaderStorageImageMultisample:           %d", features.shaderStorageImageMultisample);
        NGL_LOGI("      shaderStorageImageReadWithoutFormat:     %d", features.shaderStorageImageReadWithoutFormat);
        NGL_LOGI("      shaderStorageImageWriteWithoutFormat:    %d", features.shaderStorageImageWriteWithoutFormat);
        NGL_LOGI("      shaderUniformBufferArrayDynamicIndexing: %d", features.shaderUniformBufferArrayDynamicIndexing);
        NGL_LOGI("      shaderSampledImageArrayDynamicIndexing:  %d", features.shaderSampledImageArrayDynamicIndexing);
        NGL_LOGI("      shaderStorageBufferArrayDynamicIndexing: %d", features.shaderStorageBufferArrayDynamicIndexing);
        NGL_LOGI("      shaderStorageImageArrayDynamicIndexing:  %d", features.shaderStorageImageArrayDynamicIndexing);
        NGL_LOGI("      shaderClipDistance:                      %d", features.shaderClipDistance);
        NGL_LOGI("      shaderCullDistance:                      %d", features.shaderCullDistance);
        NGL_LOGI("      shaderFloat64:                           %d", features.shaderFloat64);
        NGL_LOGI("      shaderInt64:                             %d", features.shaderInt64);
        NGL_LOGI("      shaderInt16:                             %d", features.shaderInt16);
        NGL_LOGI("      shaderResourceResidency:                 %d", features.shaderResourceResidency);
        NGL_LOGI("      shaderResourceMinLod:                    %d", features.shaderResourceMinLod);
        NGL_LOGI("      sparseBinding:                           %d", features.sparseBinding);
        NGL_LOGI("      sparseResidencyBuffer:                   %d", features.sparseResidencyBuffer);
        NGL_LOGI("      sparseResidencyImage2D:                  %d", features.sparseResidencyImage2D);
        NGL_LOGI("      sparseResidencyImage3D:                  %d", features.sparseResidencyImage3D);
        NGL_LOGI("      sparseResidency2Samples:                 %d", features.sparseResidency2Samples);
        NGL_LOGI("      sparseResidency4Samples:                 %d", features.sparseResidency4Samples);
        NGL_LOGI("      sparseResidency8Samples:                 %d", features.sparseResidency8Samples);
        NGL_LOGI("      sparseResidency16Samples:                %d", features.sparseResidency16Samples);
        NGL_LOGI("      sparseResidencyAliased:                  %d", features.sparseResidencyAliased);
        NGL_LOGI("      variableMultisampleRate:                 %d", features.variableMultisampleRate);
        NGL_LOGI("      inheritedQueries:                        %d", features.inheritedQueries);

        NGL_LOGI("    extensions:");
        uint32_t extensionCount;
        NVK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));
        std::vector<VkExtensionProperties> extensions(extensionCount);
        NVK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data()));

        for (VkExtensionProperties extension : extensions) {
            NGL_LOGI("      %s %u", extension.extensionName, extension.specVersion);
        }
    }
}

void nvkDumpQueueFamilies(VkPhysicalDevice device) {
    NGL_LOGI("Queue families for device: %p", reinterpret_cast<void*>(device));

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> propertiesVector(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, propertiesVector.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        NGL_LOGI("  Family: %u", i);
        const VkQueueFamilyProperties& props = propertiesVector[i];
        NGL_LOGI("    queueFlags:                  %s", queueFlagsToString(props.queueFlags).c_str());
        NGL_LOGI("    queueCount:                  %u", props.queueCount);
        NGL_LOGI("    timestampValidBits:          %u", props.timestampValidBits);
        NGL_LOGI("    minImageTransferGranularity: %u x %u x %u", props.minImageTransferGranularity.width,
                 props.minImageTransferGranularity.height, props.minImageTransferGranularity.depth);
    }
}

static std::string surfaceTransformFlagsToString(VkSurfaceTransformFlagsKHR flags) {
    std::string result = "";

    auto append = [&result](const char* s) {
        if (!result.empty()) {
            result += ", ";
        }
        result += s;
    };

    if (flags & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        append("IDENTITY");
    }
    if (flags & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR) {
        append("ROTATE_90");
    }
    if (flags & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR) {
        append("ROTATE_180");
    }
    if (flags & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR) {
        append("ROTATE_270");
    }
    if (flags & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR) {
        append("HORIZONTAL_MIRROR");
    }
    if (flags & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR) {
        append("HORIZONTAL_MIRROR_ROTATE_90");
    }
    if (flags & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR) {
        append("HORIZONTAL_MIRROR_ROTATE_180");
    }
    if (flags & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR) {
        append("HORIZONTAL_MIRROR_ROTATE_270");
    }
    if (flags & VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR) {
        append("INHERIT");
    }

    return result;
}

static std::string compositeAlphaFlagsToString(VkCompositeAlphaFlagsKHR flags) {
    std::string result = "";

    auto append = [&result](const char* s) {
        if (!result.empty()) {
            result += ", ";
        }
        result += s;
    };

    if (flags & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
        append("OPAQUE");
    }
    if (flags & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
        append("PRE_MULTIPLIED");
    }
    if (flags & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
        append("POST_MULTIPLIED");
    }
    if (flags & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
        append("INHERIT");
    }

    return result;
}

static std::string imageUsageFlagsToString(VkImageUsageFlags flags) {
    std::string result = "";

    auto append = [&result](const char* s) {
        if (!result.empty()) {
            result += ", ";
        }
        result += s;
    };

    if (flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        append("TRANSFER_SRC");
    }
    if (flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        append("TRANSFER_DST");
    }
    if (flags & VK_IMAGE_USAGE_SAMPLED_BIT) {
        append("SAMPLED");
    }
    if (flags & VK_IMAGE_USAGE_STORAGE_BIT) {
        append("STORAGE");
    }
    if (flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        append("COLOR_ATTACHMENT");
    }
    if (flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        append("DEPTH_STENCIL_ATTACHMENT");
    }
    if (flags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
        append("TRANSIENT_ATTACHMENT");
    }
    if (flags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) {
        append("INPUT_ATTACHMENT");
    }
    if (flags & VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR) {
        append("VIDEO_DECODE_DST");
    }
    if (flags & VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR) {
        append("VIDEO_DECODE_SRC");
    }
    if (flags & VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR) {
        append("VIDEO_DECODE_DPB");
    }
    if (flags & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) {
        append("FRAGMENT_DENSITY_MAP");
    }
    if (flags & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) {
        append("FRAGMENT_SHADING_RATE_ATTACHMENT");
    }
    if (flags & VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT) {
        append("HOST_TRANSFER");
    }
    if (flags & VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR) {
        append("VIDEO_ENCODE_DST");
    }
    if (flags & VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR) {
        append("VIDEO_ENCODE_SRC");
    }
    if (flags & VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR) {
        append("VIDEO_ENCODE_DPB");
    }
    if (flags & VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT) {
        append("ATTACHMENT_FEEDBACK_LOOP");
    }
    if (flags & VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI) {
        append("INVOCATION_MASK_HUAWEI");
    }
    if (flags & VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM) {
        append("SAMPLE_WEIGHT_QCOM");
    }
    if (flags & VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM) {
        append("SAMPLE_BLOCK_MATCH_QCOM");
    }
    if (flags & VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV) {
        append("SHADING_RATE_IMAGE_NV");
    }

    return result;
}

void nvkDumpSurfaceCapabilities(const VkSurfaceCapabilitiesKHR capabilities, const char* indent) {
    NGL_LOGI("%sminImageCount:           %u", indent, capabilities.minImageCount);
    NGL_LOGI("%smaxImageCount:           %u", indent, capabilities.maxImageCount);
    NGL_LOGI("%scurrentExtent:           %u x %u", indent, capabilities.currentExtent.width,
             capabilities.currentExtent.height);
    NGL_LOGI("%sminImageExtent:          %u x %u", indent, capabilities.minImageExtent.width,
             capabilities.minImageExtent.height);
    NGL_LOGI("%smaxImageExtent:          %u x %u", indent, capabilities.maxImageExtent.width,
             capabilities.maxImageExtent.height);
    NGL_LOGI("%smaxImageArrayLayers:     %u", indent, capabilities.maxImageArrayLayers);
    NGL_LOGI("%ssupportedTransforms:     %s", indent,
             surfaceTransformFlagsToString(capabilities.supportedTransforms).c_str());
    NGL_LOGI("%scurrentTransform:        %s", indent,
             surfaceTransformFlagsToString(capabilities.currentTransform).c_str());
    NGL_LOGI("%ssupportedCompositeAlpha: %s", indent,
             compositeAlphaFlagsToString(capabilities.supportedCompositeAlpha).c_str());
    NGL_LOGI("%ssupportedUsageFlags: %s", indent, imageUsageFlagsToString(capabilities.supportedUsageFlags).c_str());
}

static const char* formatToString(VkFormat format) {
    switch (format) {
        case VK_FORMAT_UNDEFINED:
            return "UNDEFINED";
        case VK_FORMAT_R4G4_UNORM_PACK8:
            return "R4G4_UNORM_PACK8";
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
            return "R4G4B4A4_UNORM_PACK16";
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
            return "B4G4R4A4_UNORM_PACK16";
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
            return "R5G6B5_UNORM_PACK16";
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
            return "B5G6R5_UNORM_PACK16";
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
            return "R5G5B5A1_UNORM_PACK16";
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
            return "B5G5R5A1_UNORM_PACK16";
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
            return "A1R5G5B5_UNORM_PACK16";
        case VK_FORMAT_R8_UNORM:
            return "R8_UNORM";
        case VK_FORMAT_R8_SNORM:
            return "R8_SNORM";
        case VK_FORMAT_R8_USCALED:
            return "R8_USCALED";
        case VK_FORMAT_R8_SSCALED:
            return "R8_SSCALED";
        case VK_FORMAT_R8_UINT:
            return "R8_UINT";
        case VK_FORMAT_R8_SINT:
            return "R8_SINT";
        case VK_FORMAT_R8_SRGB:
            return "R8_SRGB";
        case VK_FORMAT_R8G8_UNORM:
            return "R8G8_UNORM";
        case VK_FORMAT_R8G8_SNORM:
            return "R8G8_SNORM";
        case VK_FORMAT_R8G8_USCALED:
            return "R8G8_USCALED";
        case VK_FORMAT_R8G8_SSCALED:
            return "R8G8_SSCALED";
        case VK_FORMAT_R8G8_UINT:
            return "R8G8_UINT";
        case VK_FORMAT_R8G8_SINT:
            return "R8G8_SINT";
        case VK_FORMAT_R8G8_SRGB:
            return "R8G8_SRGB";
        case VK_FORMAT_R8G8B8_UNORM:
            return "R8G8B8_UNORM";
        case VK_FORMAT_R8G8B8_SNORM:
            return "R8G8B8_SNORM";
        case VK_FORMAT_R8G8B8_USCALED:
            return "R8G8B8_USCALED";
        case VK_FORMAT_R8G8B8_SSCALED:
            return "R8G8B8_SSCALED";
        case VK_FORMAT_R8G8B8_UINT:
            return "R8G8B8_UINT";
        case VK_FORMAT_R8G8B8_SINT:
            return "R8G8B8_SINT";
        case VK_FORMAT_R8G8B8_SRGB:
            return "R8G8B8_SRGB";
        case VK_FORMAT_B8G8R8_UNORM:
            return "B8G8R8_UNORM";
        case VK_FORMAT_B8G8R8_SNORM:
            return "B8G8R8_SNORM";
        case VK_FORMAT_B8G8R8_USCALED:
            return "B8G8R8_USCALED";
        case VK_FORMAT_B8G8R8_SSCALED:
            return "B8G8R8_SSCALED";
        case VK_FORMAT_B8G8R8_UINT:
            return "B8G8R8_UINT";
        case VK_FORMAT_B8G8R8_SINT:
            return "B8G8R8_SINT";
        case VK_FORMAT_B8G8R8_SRGB:
            return "B8G8R8_SRGB";
        case VK_FORMAT_R8G8B8A8_UNORM:
            return "R8G8B8A8_UNORM";
        case VK_FORMAT_R8G8B8A8_SNORM:
            return "R8G8B8A8_SNORM";
        case VK_FORMAT_R8G8B8A8_USCALED:
            return "R8G8B8A8_USCALED";
        case VK_FORMAT_R8G8B8A8_SSCALED:
            return "R8G8B8A8_SSCALED";
        case VK_FORMAT_R8G8B8A8_UINT:
            return "R8G8B8A8_UINT";
        case VK_FORMAT_R8G8B8A8_SINT:
            return "R8G8B8A8_SINT";
        case VK_FORMAT_R8G8B8A8_SRGB:
            return "R8G8B8A8_SRGB";
        case VK_FORMAT_B8G8R8A8_UNORM:
            return "B8G8R8A8_UNORM";
        case VK_FORMAT_B8G8R8A8_SNORM:
            return "B8G8R8A8_SNORM";
        case VK_FORMAT_B8G8R8A8_USCALED:
            return "B8G8R8A8_USCALED";
        case VK_FORMAT_B8G8R8A8_SSCALED:
            return "B8G8R8A8_SSCALED";
        case VK_FORMAT_B8G8R8A8_UINT:
            return "B8G8R8A8_UINT";
        case VK_FORMAT_B8G8R8A8_SINT:
            return "B8G8R8A8_SINT";
        case VK_FORMAT_B8G8R8A8_SRGB:
            return "B8G8R8A8_SRGB";
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
            return "A8B8G8R8_UNORM_PACK32";
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
            return "A8B8G8R8_SNORM_PACK32";
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
            return "A8B8G8R8_USCALED_PACK32";
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
            return "A8B8G8R8_SSCALED_PACK32";
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
            return "A8B8G8R8_UINT_PACK32";
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
            return "A8B8G8R8_SINT_PACK32";
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
            return "A8B8G8R8_SRGB_PACK32";
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
            return "A2R10G10B10_UNORM_PACK32";
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
            return "A2R10G10B10_SNORM_PACK32";
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
            return "A2R10G10B10_USCALED_PACK32";
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
            return "A2R10G10B10_SSCALED_PACK32";
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
            return "A2R10G10B10_UINT_PACK32";
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
            return "A2R10G10B10_SINT_PACK32";
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            return "A2B10G10R10_UNORM_PACK32";
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
            return "A2B10G10R10_SNORM_PACK32";
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
            return "A2B10G10R10_USCALED_PACK32";
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
            return "A2B10G10R10_SSCALED_PACK32";
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
            return "A2B10G10R10_UINT_PACK32";
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
            return "A2B10G10R10_SINT_PACK32";
        case VK_FORMAT_R16_UNORM:
            return "R16_UNORM";
        case VK_FORMAT_R16_SNORM:
            return "R16_SNORM";
        case VK_FORMAT_R16_USCALED:
            return "R16_USCALED";
        case VK_FORMAT_R16_SSCALED:
            return "R16_SSCALED";
        case VK_FORMAT_R16_UINT:
            return "R16_UINT";
        case VK_FORMAT_R16_SINT:
            return "R16_SINT";
        case VK_FORMAT_R16_SFLOAT:
            return "R16_SFLOAT";
        case VK_FORMAT_R16G16_UNORM:
            return "R16G16_UNORM";
        case VK_FORMAT_R16G16_SNORM:
            return "R16G16_SNORM";
        case VK_FORMAT_R16G16_USCALED:
            return "R16G16_USCALED";
        case VK_FORMAT_R16G16_SSCALED:
            return "R16G16_SSCALED";
        case VK_FORMAT_R16G16_UINT:
            return "R16G16_UINT";
        case VK_FORMAT_R16G16_SINT:
            return "R16G16_SINT";
        case VK_FORMAT_R16G16_SFLOAT:
            return "R16G16_SFLOAT";
        case VK_FORMAT_R16G16B16_UNORM:
            return "R16G16B16_UNORM";
        case VK_FORMAT_R16G16B16_SNORM:
            return "R16G16B16_SNORM";
        case VK_FORMAT_R16G16B16_USCALED:
            return "R16G16B16_USCALED";
        case VK_FORMAT_R16G16B16_SSCALED:
            return "R16G16B16_SSCALED";
        case VK_FORMAT_R16G16B16_UINT:
            return "R16G16B16_UINT";
        case VK_FORMAT_R16G16B16_SINT:
            return "R16G16B16_SINT";
        case VK_FORMAT_R16G16B16_SFLOAT:
            return "R16G16B16_SFLOAT";
        case VK_FORMAT_R16G16B16A16_UNORM:
            return "R16G16B16A16_UNORM";
        case VK_FORMAT_R16G16B16A16_SNORM:
            return "R16G16B16A16_SNORM";
        case VK_FORMAT_R16G16B16A16_USCALED:
            return "R16G16B16A16_USCALED";
        case VK_FORMAT_R16G16B16A16_SSCALED:
            return "R16G16B16A16_SSCALED";
        case VK_FORMAT_R16G16B16A16_UINT:
            return "R16G16B16A16_UINT";
        case VK_FORMAT_R16G16B16A16_SINT:
            return "R16G16B16A16_SINT";
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return "R16G16B16A16_SFLOAT";
        case VK_FORMAT_R32_UINT:
            return "R32_UINT";
        case VK_FORMAT_R32_SINT:
            return "R32_SINT";
        case VK_FORMAT_R32_SFLOAT:
            return "R32_SFLOAT";
        case VK_FORMAT_R32G32_UINT:
            return "R32G32_UINT";
        case VK_FORMAT_R32G32_SINT:
            return "R32G32_SINT";
        case VK_FORMAT_R32G32_SFLOAT:
            return "R32G32_SFLOAT";
        case VK_FORMAT_R32G32B32_UINT:
            return "R32G32B32_UINT";
        case VK_FORMAT_R32G32B32_SINT:
            return "R32G32B32_SINT";
        case VK_FORMAT_R32G32B32_SFLOAT:
            return "R32G32B32_SFLOAT";
        case VK_FORMAT_R32G32B32A32_UINT:
            return "R32G32B32A32_UINT";
        case VK_FORMAT_R32G32B32A32_SINT:
            return "R32G32B32A32_SINT";
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return "R32G32B32A32_SFLOAT";
        case VK_FORMAT_R64_UINT:
            return "R64_UINT";
        case VK_FORMAT_R64_SINT:
            return "R64_SINT";
        case VK_FORMAT_R64_SFLOAT:
            return "R64_SFLOAT";
        case VK_FORMAT_R64G64_UINT:
            return "R64G64_UINT";
        case VK_FORMAT_R64G64_SINT:
            return "R64G64_SINT";
        case VK_FORMAT_R64G64_SFLOAT:
            return "R64G64_SFLOAT";
        case VK_FORMAT_R64G64B64_UINT:
            return "R64G64B64_UINT";
        case VK_FORMAT_R64G64B64_SINT:
            return "R64G64B64_SINT";
        case VK_FORMAT_R64G64B64_SFLOAT:
            return "R64G64B64_SFLOAT";
        case VK_FORMAT_R64G64B64A64_UINT:
            return "R64G64B64A64_UINT";
        case VK_FORMAT_R64G64B64A64_SINT:
            return "R64G64B64A64_SINT";
        case VK_FORMAT_R64G64B64A64_SFLOAT:
            return "R64G64B64A64_SFLOAT";
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
            return "B10G11R11_UFLOAT_PACK32";
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
            return "E5B9G9R9_UFLOAT_PACK32";
        case VK_FORMAT_D16_UNORM:
            return "D16_UNORM";
        case VK_FORMAT_X8_D24_UNORM_PACK32:
            return "X8_D24_UNORM_PACK32";
        case VK_FORMAT_D32_SFLOAT:
            return "D32_SFLOAT";
        case VK_FORMAT_S8_UINT:
            return "S8_UINT";
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return "D16_UNORM_S8_UINT";
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return "D24_UNORM_S8_UINT";
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return "D32_SFLOAT_S8_UINT";
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
            return "BC1_RGB_UNORM_BLOCK";
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
            return "BC1_RGB_SRGB_BLOCK";
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
            return "BC1_RGBA_UNORM_BLOCK";
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
            return "BC1_RGBA_SRGB_BLOCK";
        case VK_FORMAT_BC2_UNORM_BLOCK:
            return "BC2_UNORM_BLOCK";
        case VK_FORMAT_BC2_SRGB_BLOCK:
            return "BC2_SRGB_BLOCK";
        case VK_FORMAT_BC3_UNORM_BLOCK:
            return "BC3_UNORM_BLOCK";
        case VK_FORMAT_BC3_SRGB_BLOCK:
            return "BC3_SRGB_BLOCK";
        case VK_FORMAT_BC4_UNORM_BLOCK:
            return "BC4_UNORM_BLOCK";
        case VK_FORMAT_BC4_SNORM_BLOCK:
            return "BC4_SNORM_BLOCK";
        case VK_FORMAT_BC5_UNORM_BLOCK:
            return "BC5_UNORM_BLOCK";
        case VK_FORMAT_BC5_SNORM_BLOCK:
            return "BC5_SNORM_BLOCK";
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:
            return "BC6H_UFLOAT_BLOCK";
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:
            return "BC6H_SFLOAT_BLOCK";
        case VK_FORMAT_BC7_UNORM_BLOCK:
            return "BC7_UNORM_BLOCK";
        case VK_FORMAT_BC7_SRGB_BLOCK:
            return "BC7_SRGB_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            return "ETC2_R8G8B8_UNORM_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
            return "ETC2_R8G8B8_SRGB_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
            return "ETC2_R8G8B8A1_UNORM_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
            return "ETC2_R8G8B8A1_SRGB_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
            return "ETC2_R8G8B8A8_UNORM_BLOCK";
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
            return "ETC2_R8G8B8A8_SRGB_BLOCK";
        case VK_FORMAT_EAC_R11_UNORM_BLOCK:
            return "EAC_R11_UNORM_BLOCK";
        case VK_FORMAT_EAC_R11_SNORM_BLOCK:
            return "EAC_R11_SNORM_BLOCK";
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
            return "EAC_R11G11_UNORM_BLOCK";
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
            return "EAC_R11G11_SNORM_BLOCK";
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
            return "ASTC_4x4_UNORM_BLOCK";
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
            return "ASTC_4x4_SRGB_BLOCK";
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
            return "ASTC_5x4_UNORM_BLOCK";
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
            return "ASTC_5x4_SRGB_BLOCK";
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
            return "ASTC_5x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
            return "ASTC_5x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
            return "ASTC_6x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
            return "ASTC_6x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
            return "ASTC_6x6_UNORM_BLOCK";
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
            return "ASTC_6x6_SRGB_BLOCK";
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
            return "ASTC_8x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
            return "ASTC_8x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
            return "ASTC_8x6_UNORM_BLOCK";
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
            return "ASTC_8x6_SRGB_BLOCK";
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
            return "ASTC_8x8_UNORM_BLOCK";
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
            return "ASTC_8x8_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
            return "ASTC_10x5_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
            return "ASTC_10x5_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
            return "ASTC_10x6_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
            return "ASTC_10x6_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
            return "ASTC_10x8_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
            return "ASTC_10x8_SRGB_BLOCK";
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
            return "ASTC_10x10_UNORM_BLOCK";
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
            return "ASTC_10x10_SRGB_BLOCK";
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
            return "ASTC_12x10_UNORM_BLOCK";
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
            return "ASTC_12x10_SRGB_BLOCK";
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
            return "ASTC_12x12_UNORM_BLOCK";
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
            return "ASTC_12x12_SRGB_BLOCK";
        case VK_FORMAT_G8B8G8R8_422_UNORM:
            return "G8B8G8R8_422_UNORM";
        case VK_FORMAT_B8G8R8G8_422_UNORM:
            return "B8G8R8G8_422_UNORM";
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
            return "G8_B8_R8_3PLANE_420_UNORM";
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
            return "G8_B8R8_2PLANE_420_UNORM";
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
            return "G8_B8_R8_3PLANE_422_UNORM";
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
            return "G8_B8R8_2PLANE_422_UNORM";
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
            return "G8_B8_R8_3PLANE_444_UNORM";
        case VK_FORMAT_R10X6_UNORM_PACK16:
            return "R10X6_UNORM_PACK16";
        case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
            return "R10X6G10X6_UNORM_2PACK16";
        case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
            return "R10X6G10X6B10X6A10X6_UNORM_4PACK16";
        case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
            return "G10X6B10X6G10X6R10X6_422_UNORM_4PACK16";
        case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
            return "B10X6G10X6R10X6G10X6_422_UNORM_4PACK16";
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
            return "G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16";
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
            return "G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16";
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
            return "G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16";
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
            return "G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16";
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
            return "G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16";
        case VK_FORMAT_R12X4_UNORM_PACK16:
            return "R12X4_UNORM_PACK16";
        case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
            return "R12X4G12X4_UNORM_2PACK16";
        case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
            return "R12X4G12X4B12X4A12X4_UNORM_4PACK16";
        case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
            return "G12X4B12X4G12X4R12X4_422_UNORM_4PACK16";
        case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
            return "B12X4G12X4R12X4G12X4_422_UNORM_4PACK16";
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
            return "G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
            return "G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
            return "G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
            return "G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
            return "G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16";
        case VK_FORMAT_G16B16G16R16_422_UNORM:
            return "G16B16G16R16_422_UNORM";
        case VK_FORMAT_B16G16R16G16_422_UNORM:
            return "B16G16R16G16_422_UNORM";
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
            return "G16_B16_R16_3PLANE_420_UNORM";
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
            return "G16_B16R16_2PLANE_420_UNORM";
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
            return "G16_B16_R16_3PLANE_422_UNORM";
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
            return "G16_B16R16_2PLANE_422_UNORM";
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
            return "G16_B16_R16_3PLANE_444_UNORM";
        case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
            return "G8_B8R8_2PLANE_444_UNORM";
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
            return "G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16";
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
            return "G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16";
        case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM:
            return "G16_B16R16_2PLANE_444_UNORM";
        case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
            return "A4R4G4B4_UNORM_PACK16";
        case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
            return "A4B4G4R4_UNORM_PACK16";
        case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
            return "ASTC_4x4_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
            return "ASTC_5x4_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
            return "ASTC_5x5_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
            return "ASTC_6x5_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
            return "ASTC_6x6_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
            return "ASTC_8x5_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
            return "ASTC_8x6_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
            return "ASTC_8x8_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
            return "ASTC_10x5_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
            return "ASTC_10x6_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
            return "ASTC_10x8_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
            return "ASTC_10x10_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
            return "ASTC_12x10_SFLOAT_BLOCK";
        case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:
            return "ASTC_12x12_SFLOAT_BLOCK";
        case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
            return "PVRTC1_2BPP_UNORM_BLOCK_IMG";
        case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
            return "PVRTC1_4BPP_UNORM_BLOCK_IMG";
        case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
            return "PVRTC2_2BPP_UNORM_BLOCK_IMG";
        case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
            return "PVRTC2_4BPP_UNORM_BLOCK_IMG";
        case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
            return "PVRTC1_2BPP_SRGB_BLOCK_IMG";
        case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
            return "PVRTC1_4BPP_SRGB_BLOCK_IMG";
        case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
            return "PVRTC2_2BPP_SRGB_BLOCK_IMG";
        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
            return "PVRTC2_4BPP_SRGB_BLOCK_IMG";
        case VK_FORMAT_R16G16_S10_5_NV:
            return "R16G16_S10_5_NV";
        case VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR:
            return "A1B5G5R5_UNORM_PACK16_KHR";
        case VK_FORMAT_A8_UNORM_KHR:
            return "A8_UNORM_KHR";
        default:
            return "UNKNOWN";
    }
}

static const char* colorSpaceToString(VkColorSpaceKHR colorSpace) {
    switch (colorSpace) {
        case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:
            return "SRGB_NONLINEAR_KHR";
        case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:
            return "DISPLAY_P3_NONLINEAR_EXT";
        case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
            return "EXTENDED_SRGB_LINEAR_EXT";
        case VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT:
            return "DISPLAY_P3_LINEAR_EXT";
        case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT:
            return "DCI_P3_NONLINEAR_EXT";
        case VK_COLOR_SPACE_BT709_LINEAR_EXT:
            return "BT709_LINEAR_EXT";
        case VK_COLOR_SPACE_BT709_NONLINEAR_EXT:
            return "BT709_NONLINEAR_EXT";
        case VK_COLOR_SPACE_BT2020_LINEAR_EXT:
            return "BT2020_LINEAR_EXT";
        case VK_COLOR_SPACE_HDR10_ST2084_EXT:
            return "HDR10_ST2084_EXT";
        case VK_COLOR_SPACE_DOLBYVISION_EXT:
            return "DOLBYVISION_EXT";
        case VK_COLOR_SPACE_HDR10_HLG_EXT:
            return "HDR10_HLG_EXT";
        case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT:
            return "ADOBERGB_LINEAR_EXT";
        case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT:
            return "ADOBERGB_NONLINEAR_EXT";
        case VK_COLOR_SPACE_PASS_THROUGH_EXT:
            return "PASS_THROUGH_EXT";
        case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT:
            return "EXTENDED_SRGB_NONLINEAR_EXT";
        case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD:
            return "DISPLAY_NATIVE_AMD";
        default:
            return "UNKNOWN";
    }
}

void nvkDumpSurfaceFormats(const std::vector<VkSurfaceFormatKHR>& formats, const char* indent) {
    for (VkSurfaceFormatKHR format : formats) {
        NGL_LOGI("%s%s/%s", indent, formatToString(format.format), colorSpaceToString(format.colorSpace));
    }
}

static const char* presentModeToString(VkPresentModeKHR mode) {
    switch (mode) {
        case VK_PRESENT_MODE_IMMEDIATE_KHR:
            return "VK_PRESENT_MODE_IMMEDIATE_KHR";
        case VK_PRESENT_MODE_MAILBOX_KHR:
            return "VK_PRESENT_MODE_MAILBOX_KHR";
        case VK_PRESENT_MODE_FIFO_KHR:
            return "VK_PRESENT_MODE_FIFO_KHR";
        case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
            return "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
        case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
            return "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR";
        case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
            return "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR";
        default:
            return "UNKNOWN";
    }
}

void nvkDumpPresentModes(const std::vector<VkPresentModeKHR>& modes, const char* indent) {
    for (VkPresentModeKHR mode : modes) {
        NGL_LOGI("%s%s", indent, presentModeToString(mode));
    }
}
