#include "nvkmain.h"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "nvkmain.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "nfile.h"
#include "nglassert.h"
#include "ngllog.h"
#include "nvkdbg.h"
#include "nvkerr.h"
#include "nvkutil.h"

constexpr uint32_t kWidth = 1920;
constexpr uint32_t kHeight = 1080;
const std::vector<const char*> kDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        terminate();
    }

private:
    void initWindow() {
        if (!glfwInit()) {
            NGL_LOGE("glfwInit() failed");
            abort();
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        mWindow = glfwCreateWindow(kWidth, kHeight, "N War (VK)", nullptr, nullptr);
        if (!mWindow) {
            NGL_LOGE("glfwCreateWindow() failed");
            abort();
        }
    }

    void initVulkan() {
        createInstance();
        nvkInitDebugIfNecessary(mInstance);
        createSurface();
        nvkDumpPhysicalDevices(mInstance);
        choosePhysicalDevice();
        nvkDumpQueueFamilies(mPhysicalDevice);
        createDevice();
        createSwapchain();
        createSwapchainImageViews();
        createRenderPass();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandPool();
        createCommandBuffer();
        createSyncObjects();
    }

    void createInstance() {
        NGL_LOGI("Available instance extensions:");
        uint32_t extensionCount;
        NVK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        for (VkExtensionProperties extension : extensions) {
            NGL_LOGI("  %s %u", extension.extensionName, extension.specVersion);
        }

        NGL_LOGI("Available instance layers:");
        uint32_t layerCount;
        NVK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
        std::vector<VkLayerProperties> layers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
        for (VkLayerProperties layer : layers) {
            NGL_LOGI("  %s %u %u %s", layer.layerName, layer.specVersion, layer.implementationVersion,
                     layer.description);
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "N War (VK)";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        if (!mWindow) {
            NGL_LOGE("glfwGetRequiredInstanceExtensions() failed");
            abort();
        }
        NGL_LOGI("Instance extensions required by GLFW:");
        for (uint32_t i = 0; i < glfwExtensionCount; i++) {
            NGL_LOGI("  %s", glfwExtensions[i]);
        }

        std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        nvkAppendDebugExtensionsIfNecessary(requiredExtensions);
        NGL_LOGI("Required instance extensions:");
        for (const char* requiredExtension : requiredExtensions) {
            NGL_LOGI("  %s", requiredExtension);
        }

        std::vector<const char*> requiredLayers;
        nvkAppendDebugLayersIfNecessary(requiredLayers);
        NGL_LOGI("Required instance layers:");
        for (const char* requiredLayer : requiredLayers) {
            NGL_LOGI("  %s", requiredLayer);
        }

        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
        createInfo.ppEnabledLayerNames = requiredLayers.data();
        createInfo.pNext = nvkPopulateDebugMessengerCreateInfoIfNecessary(&debugMessengerCreateInfo);

        NVK_CHECK(vkCreateInstance(&createInfo, nullptr, &mInstance));
        NGL_LOGI("mInstance: %p", reinterpret_cast<void*>(mInstance));
    }

    void createSurface() {
        NVK_CHECK(glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface));
        NGL_LOGI("mSurface: %p", reinterpret_cast<void*>(mSurface));
    }

    void choosePhysicalDevice() {
        uint32_t deviceCount = 0;
        NVK_CHECK(vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr));
        if (deviceCount == 0) {
            NGL_ABORT("Found no Vulkan devices");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                mPhysicalDevice = device;
                break;
            }
        }

        if (mPhysicalDevice == VK_NULL_HANDLE) {
            NGL_ABORT("Found no suitable Vulkan device");
        }

        NGL_LOGI("Chosen physical device: %p", reinterpret_cast<void*>(mPhysicalDevice));
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(device);

        if (!queueFamilyIndices.isComplete()) {
            return false;
        }

        if (!checkDeviceExtensionsSupport(device)) {
            return false;
        }

        SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device);
        if (swapchainSupport.formats.empty() || swapchainSupport.presentModes.empty()) {
            return false;
        }

        return true;
    }

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices result;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                result.graphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            NVK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport));
            if (presentSupport) {
                result.presentFamily = i;
            }
            if (result.isComplete()) {
                break;
            }
        }

        return result;
    }

    bool checkDeviceExtensionsSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        NVK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));
        std::vector<VkExtensionProperties> extensions(extensionCount);
        NVK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data()));

        std::set<std::string> requiredExtensions(kDeviceExtensions.begin(), kDeviceExtensions.end());

        for (const auto& extension : extensions) {
            requiredExtensions.erase(extension.extensionName);
        }
        return requiredExtensions.empty();
    }

    void createDevice() {
        QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);

        NGL_ASSERT(indices.graphicsFamily.has_value());
        NGL_ASSERT(indices.presentFamily.has_value());

        NGL_LOGI("graphicsFamily queue index: %u", indices.graphicsFamily.value());
        NGL_LOGI("presentFamily queue index:  %u", indices.presentFamily.value());

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        std::vector<const char*> requiredLayers;
        nvkAppendDebugLayersIfNecessary(requiredLayers);

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
        createInfo.ppEnabledLayerNames = requiredLayers.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(kDeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = kDeviceExtensions.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        NVK_CHECK(vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice));
        NGL_LOGI("mDevice: %p", reinterpret_cast<void*>(mDevice));

        vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
        vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &mPresentQueue);
        NGL_LOGI("mGraphicsQueue: %p", reinterpret_cast<void*>(mGraphicsQueue));
        NGL_LOGI("mPresentQueue:  %p", reinterpret_cast<void*>(mPresentQueue));
    }

    void createSwapchain() {
        SwapchainSupportDetails swapchainSupport = querySwapchainSupport(mPhysicalDevice);
        VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapchainSupport.formats);
        VkPresentModeKHR presentMode = choosePresentMode(swapchainSupport.presentModes);
        VkExtent2D extent = chooseExtent(swapchainSupport.capabilities);
        uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
        if (swapchainSupport.capabilities.maxImageCount > 0) {
            imageCount = min(imageCount, swapchainSupport.capabilities.maxImageCount);
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = mSurface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        NVK_CHECK(vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapchain));
        NGL_LOGI("mSwapchain: %p", reinterpret_cast<void*>(mSwapchain));

        NVK_CHECK(vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr));
        mSwapchainImages.resize(imageCount);
        NVK_CHECK(vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, mSwapchainImages.data()));
        NGL_LOGI("mSwapchainImages:");
        for (VkImage image : mSwapchainImages) {
            NGL_LOGI("  %p", reinterpret_cast<void*>(image));
        }

        mSwapchainFormat = surfaceFormat.format;
        NGL_LOGI("mSwapchainFormat: %s", nvkFormatToString(mSwapchainFormat));

        mSwapchainExtent = extent;
        NGL_LOGI("mSwapchainExtent: %u x %u", mSwapchainExtent.width, mSwapchainExtent.height);
    }

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device) {
        SwapchainSupportDetails details;

        NVK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.capabilities));

        uint32_t formatCount;
        NVK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr));
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            NVK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, details.formats.data()));
        }

        uint32_t presentModeCount;
        NVK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, nullptr));
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            NVK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount,
                                                                details.presentModes.data()));
        }

        NGL_LOGI("Swapchain support details for device %p surface %p:", reinterpret_cast<void*>(device),
                 reinterpret_cast<void*>(mSurface));
        NGL_LOGI("  Capabilities:");
        nvkDumpSurfaceCapabilities(details.capabilities, "    ");
        NGL_LOGI("  Formats:");
        nvkDumpSurfaceFormats(details.formats, "    ");
        NGL_LOGI("  Present modes:");
        nvkDumpPresentModes(details.presentModes, "    ");

        return details;
    }

    VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& /*availablePresentModes*/) {
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(mWindow, &width, &height);
            VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                            capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                             capabilities.maxImageExtent.height);
            return actualExtent;
        }
    }

    void createSwapchainImageViews() {
        NGL_LOGI("mSwapchainImageViews:");
        mSwapchainImageViews.resize(mSwapchainImages.size());
        for (size_t i = 0; i < mSwapchainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = mSwapchainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = mSwapchainFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;
            NVK_CHECK(vkCreateImageView(mDevice, &createInfo, nullptr, &mSwapchainImageViews[i]));
            NGL_LOGI("  %p", reinterpret_cast<void*>(mSwapchainImageViews[i]));
        }
    }
    void createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = mSwapchainFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &colorAttachment;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &dependency;

        NVK_CHECK(vkCreateRenderPass(mDevice, &renderPassCreateInfo, nullptr, &mRenderPass));
        NGL_LOGI("mRenderPass: %p", reinterpret_cast<void*>(mRenderPass));
    }

    void createGraphicsPipeline() {
        auto vertShaderCode = nReadFile("shaders/vert.spv");
        auto fragShaderCode = nReadFile("shaders/frag.spv");
        NGL_LOGI("vertShaderCode.size: %zu", vertShaderCode.size());
        NGL_LOGI("fragShaderCode.size: %zu", fragShaderCode.size());
        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
        NGL_LOGI("vertShaderModule: %p", reinterpret_cast<void*>(vertShaderModule));
        NGL_LOGI("fragShaderModule: %p", reinterpret_cast<void*>(fragShaderModule));

        VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
        vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageCreateInfo.module = vertShaderModule;
        vertShaderStageCreateInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
        fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageCreateInfo.module = fragShaderModule;
        fragShaderStageCreateInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};

        std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
        viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.scissorCount = 1;

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
        vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;  // Optional
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;  // Optional

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
        inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(mSwapchainExtent.width);
        viewport.height = static_cast<float>(mSwapchainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = mSwapchainExtent;

        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
        rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
        rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationStateCreateInfo.lineWidth = 1.0f;
        rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
        rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;  // Optional
        rasterizationStateCreateInfo.depthBiasClamp = 0.0f;           // Optional
        rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;     // Optional

        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
        multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleStateCreateInfo.minSampleShading = 1.0f;           // Optional
        multisampleStateCreateInfo.pSampleMask = nullptr;             // Optional
        multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
        multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;       // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
        colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                   VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachmentState.blendEnable = VK_FALSE;
        colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
        colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
        colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
        colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
        colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
        colorBlendStateCreateInfo.attachmentCount = 1;
        colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
        colorBlendStateCreateInfo.blendConstants[0] = 0.0f;  // Optional
        colorBlendStateCreateInfo.blendConstants[1] = 0.0f;  // Optional
        colorBlendStateCreateInfo.blendConstants[2] = 0.0f;  // Optional
        colorBlendStateCreateInfo.blendConstants[3] = 0.0f;  // Optional

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 0;             // Optional
        pipelineLayoutCreateInfo.pSetLayouts = nullptr;          // Optional
        pipelineLayoutCreateInfo.pushConstantRangeCount = 0;     // Optional
        pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;  // Optional

        NVK_CHECK(vkCreatePipelineLayout(mDevice, &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout));
        NGL_LOGI("mPipelineLayout: %p", reinterpret_cast<void*>(mPipelineLayout));

        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stageCount = 2;
        pipelineCreateInfo.pStages = shaderStages;
        pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
        pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
        pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
        pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
        pipelineCreateInfo.pDepthStencilState = nullptr;  // Optional
        pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
        pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
        pipelineCreateInfo.layout = mPipelineLayout;
        pipelineCreateInfo.renderPass = mRenderPass;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
        pipelineCreateInfo.basePipelineIndex = -1;               // Optional

        NVK_CHECK(vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mPipeline));
        NGL_LOGI("mPipeline: %p", reinterpret_cast<void*>(mPipeline));

        vkDestroyShaderModule(mDevice, fragShaderModule, nullptr);
        vkDestroyShaderModule(mDevice, vertShaderModule, nullptr);
    }

    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule result;
        NVK_CHECK(vkCreateShaderModule(mDevice, &createInfo, nullptr, &result));
        return result;
    }

    void createFramebuffers() {
        mSwapchainFramebuffers.resize(mSwapchainImageViews.size());
        for (size_t i = 0; i < mSwapchainImageViews.size(); i++) {
            VkImageView attachments[] = {mSwapchainImageViews[i]};

            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass = mRenderPass;
            framebufferCreateInfo.attachmentCount = 1;
            framebufferCreateInfo.pAttachments = attachments;
            framebufferCreateInfo.width = mSwapchainExtent.width;
            framebufferCreateInfo.height = mSwapchainExtent.height;
            framebufferCreateInfo.layers = 1;

            NVK_CHECK(vkCreateFramebuffer(mDevice, &framebufferCreateInfo, nullptr, &mSwapchainFramebuffers[i]));
            NGL_LOGI("mSwapchainFramebuffers[%zd]: %p", i, reinterpret_cast<void*>(mSwapchainFramebuffers[i]));
        }
    }

    void createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(mPhysicalDevice);

        VkCommandPoolCreateInfo poolCreateInfo{};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        NVK_CHECK(vkCreateCommandPool(mDevice, &poolCreateInfo, nullptr, &mCommandPool));
        NGL_LOGI("mCommandPool: %p", reinterpret_cast<void*>(mCommandPool));
    }

    void createCommandBuffer() {
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = mCommandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = 1;

        NVK_CHECK(vkAllocateCommandBuffers(mDevice, &allocateInfo, &mCommandBuffer));
        NGL_LOGI("mCommandBuffer: %p", reinterpret_cast<void*>(mCommandBuffer));
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo bufferbeginInfo{};
        bufferbeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        bufferbeginInfo.flags = 0;                   // Optional
        bufferbeginInfo.pInheritanceInfo = nullptr;  // Optional
        NVK_CHECK(vkBeginCommandBuffer(commandBuffer, &bufferbeginInfo));

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = mRenderPass;
        renderPassBeginInfo.framebuffer = mSwapchainFramebuffers[imageIndex];
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = mSwapchainExtent;
        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(mSwapchainExtent.width);
        viewport.height = static_cast<float>(mSwapchainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = mSwapchainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        NVK_CHECK(vkEndCommandBuffer(commandBuffer));
    }

    void createSyncObjects() {
        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        NVK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mImageAvailableSemaphore));
        NGL_LOGI("mImageAvailableSemaphore: %p", reinterpret_cast<void*>(mImageAvailableSemaphore));
        NVK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mRenderFinishedSemaphore));
        NGL_LOGI("mRenderFinishedSemaphore: %p", reinterpret_cast<void*>(mRenderFinishedSemaphore));

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        NVK_CHECK(vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &mInFlightFence));
        NGL_LOGI("mInFlightFence: %p", reinterpret_cast<void*>(mInFlightFence));
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(mWindow)) {
            glfwPollEvents();
            drawFrame();
        }
        NVK_CHECK(vkDeviceWaitIdle(mDevice));
    }

    void drawFrame() {
        NVK_CHECK(vkWaitForFences(mDevice, 1, &mInFlightFence, VK_TRUE, UINT64_MAX));
        NVK_CHECK(vkResetFences(mDevice, 1, &mInFlightFence));

        uint32_t imageIndex;
        NVK_CHECK(vkAcquireNextImageKHR(mDevice, mSwapchain, UINT64_MAX, mImageAvailableSemaphore, VK_NULL_HANDLE,
                                        &imageIndex));

        NVK_CHECK(vkResetCommandBuffer(mCommandBuffer, 0));

        recordCommandBuffer(mCommandBuffer, imageIndex);

        VkSemaphore waitSemaphores[] = {mImageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphore};

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &mCommandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        NVK_CHECK(vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFence));

        VkSwapchainKHR swapChains[] = {mSwapchain};

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;  // Optional
        NVK_CHECK(vkQueuePresentKHR(mPresentQueue, &presentInfo));
    }

    void terminate() {
        vkDestroyFence(mDevice, mInFlightFence, nullptr);
        vkDestroySemaphore(mDevice, mRenderFinishedSemaphore, nullptr);
        vkDestroySemaphore(mDevice, mImageAvailableSemaphore, nullptr);
        vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
        for (auto framebuffer : mSwapchainFramebuffers) {
            vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
        }
        vkDestroyPipeline(mDevice, mPipeline, nullptr);
        vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
        vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
        for (auto imageView : mSwapchainImageViews) {
            vkDestroyImageView(mDevice, imageView, nullptr);
        }
        vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
        vkDestroyDevice(mDevice, nullptr);
        vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
        nvkTerminateDebugIfNecessary(mInstance);
        vkDestroyInstance(mInstance, nullptr);
        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }

    GLFWwindow* mWindow = nullptr;
    VkInstance mInstance = VK_NULL_HANDLE;
    VkSurfaceKHR mSurface = VK_NULL_HANDLE;
    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;
    VkQueue mGraphicsQueue = VK_NULL_HANDLE;
    VkQueue mPresentQueue = VK_NULL_HANDLE;
    VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
    std::vector<VkImage> mSwapchainImages;
    VkFormat mSwapchainFormat;
    VkExtent2D mSwapchainExtent;
    std::vector<VkImageView> mSwapchainImageViews;
    VkRenderPass mRenderPass;
    VkPipelineLayout mPipelineLayout;
    VkPipeline mPipeline;
    std::vector<VkFramebuffer> mSwapchainFramebuffers;
    VkCommandPool mCommandPool;
    VkCommandBuffer mCommandBuffer;
    VkSemaphore mImageAvailableSemaphore;
    VkSemaphore mRenderFinishedSemaphore;
    VkFence mInFlightFence;
};

int nvkMain() {
    HelloTriangleApplication app;
    app.run();
    return 0;
}
