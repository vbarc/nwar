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
