#include "nvkmain.h"

#include <stb_image.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <limits>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

//
#include "nvkvk.h"
//

#include "NvkCamera.h"
#include "nfile.h"
#include "nglassert.h"
#include "ngllog.h"
#include "nvkdbg.h"
#include "nvkerr.h"
#include "nvkutil.h"

constexpr uint32_t kWidth = 1920;
constexpr uint32_t kHeight = 1080;
const std::vector<const char*> kDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
constexpr int kMaxFramesInFlight = 2;
const char* const kModelPath = "models/viking_room.obj";
const char* const kTexturePath = "terrain-texture.png";

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

namespace std {

template <>
struct hash<Vertex> {
    size_t operator()(Vertex const& vertex) const {
        return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
               (hash<glm::vec2>()(vertex.texCoord) << 1);
    }
};

}  // namespace std

struct UniformBufferObject {
    glm::mat4 model_view;
    glm::mat4 proj;
};

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

        if (true) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();

            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

            mWindow = glfwCreateWindow(mode->width, mode->height, "N War (Vulkan)", monitor, nullptr);
        } else {
            mWindow = glfwCreateWindow(kWidth, kHeight, "N War (Vulkan)", nullptr, nullptr);
        }

        if (!mWindow) {
            NGL_LOGE("glfwCreateWindow() failed");
            abort();
        }

        glfwSetWindowUserPointer(mWindow, this);

        glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow* window, int /*width*/, int /*height*/) {
            thiz(window)->mFramebufferResized = true;
        });

        glfwSetKeyCallback(mWindow, [](auto window, int key, int scancode, int action, int mods) {
            if (key == GLFW_KEY_ESCAPE && action != GLFW_RELEASE) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                return;
            }
            if (key == GLFW_KEY_SPACE && action != GLFW_RELEASE) {
                // gIsWireFrameEnabled = !gIsWireFrameEnabled;
                return;
            }
            if (thiz(window)->mCamera.onKeyEvent(key, scancode, action, mods)) {
                return;
            }
        });

        glfwSetMouseButtonCallback(mWindow, [](auto window, int button, int action, int mods) {
            if (thiz(window)->mCamera.onMouseButtonEvent(window, button, action, mods)) {
                return;
            }
        });

        glfwSetCursorPosCallback(mWindow, [](auto window, double x, double y) {
            if (thiz(window)->mCamera.onMouseMotionEvent(window, x, y)) {
                return;
            }
        });
    }

    static HelloTriangleApplication* thiz(GLFWwindow* window) {
        return reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
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
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createCommandPool();
        nvkDumpPhysicalDeviceMemoryProperties(mPhysicalDevice);
        createDepthResources();
        createFramebuffers();
        createTextureImage();
        createTextureImageView();
        createTextureSampler();
        loadModel();
        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
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

        VkPhysicalDeviceFeatures physicalDeviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &physicalDeviceFeatures);
        if (!physicalDeviceFeatures.samplerAnisotropy) {
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
        deviceFeatures.samplerAnisotropy = VK_TRUE;

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
            imageCount = std::min(imageCount, swapchainSupport.capabilities.maxImageCount);
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

    void recreateSwapchain() {
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(mWindow, &width, &height);
        while (width == 0 || height == 0) {
            NGL_LOGI("width: %d, height: %d", width, height);
            glfwGetFramebufferSize(mWindow, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(mDevice);

        cleanupSwapchain();

        createSwapchain();
        createSwapchainImageViews();
        createDepthResources();
        createFramebuffers();
    }

    void createSwapchainImageViews() {
        NGL_LOGI("mSwapchainImageViews:");
        mSwapchainImageViews.resize(mSwapchainImages.size());
        for (size_t i = 0; i < mSwapchainImages.size(); i++) {
            mSwapchainImageViews[i] = createImageView(mSwapchainImages[i], mSwapchainFormat, VK_IMAGE_ASPECT_COLOR_BIT);
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

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask =
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask =
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &dependency;

        NVK_CHECK(vkCreateRenderPass(mDevice, &renderPassCreateInfo, nullptr, &mRenderPass));
        NGL_LOGI("mRenderPass: %p", reinterpret_cast<void*>(mRenderPass));
    }

    void createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;  // Optional

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();
        NVK_CHECK(vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout));
        NGL_LOGI("mDescriptorSetLayout: %p", reinterpret_cast<void*>(mDescriptorSetLayout));
    }

    void createGraphicsPipeline() {
        auto vertShaderCode = nReadFile("out/vertex.spv");
        auto fragShaderCode = nReadFile("out/fragment.spv");
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

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount =
                static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

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
        rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE; // VK_CULL_MODE_BACK_BIT;
        rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
        depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
        depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilStateCreateInfo.minDepthBounds = 0.0f;  // Optional
        depthStencilStateCreateInfo.maxDepthBounds = 1.0f;  // Optional
        depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
        depthStencilStateCreateInfo.front = {};  // Optional
        depthStencilStateCreateInfo.back = {};   // Optional

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &mDescriptorSetLayout;
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
        pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
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
            std::array<VkImageView, 2> attachments = {mSwapchainImageViews[i], mDepthImageView};

            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass = mRenderPass;
            framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferCreateInfo.pAttachments = attachments.data();
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

    VkCommandBuffer beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = mCommandPool;
        allocInfo.commandBufferCount = 1;
        VkCommandBuffer commandBuffer;
        NVK_CHECK(vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer));

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        NVK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        NVK_CHECK(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        NVK_CHECK(vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

        NVK_CHECK(vkQueueWaitIdle(mGraphicsQueue));

        vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
    }

    void createDepthResources() {
        VkFormat depthFormat = findDepthFormat();
        NGL_LOGI("depthFormat: %s", nvkFormatToString(depthFormat));

        createImage(mSwapchainExtent.width, mSwapchainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepthImage,
                    mDepthImageMemory);
        mDepthImageView = createImageView(mDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

        transitionImageLayout(mDepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

    VkFormat findDepthFormat() {
        return findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                   VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        NGL_ABORT("Error finding format");
    }

    bool hasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void createTextureImage() {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(kTexturePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        NGL_VERIFY(pixels);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        NGL_LOGI("Creating staging buffer for texture image...");
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                     stagingBufferMemory);

        void* data;
        vkMapMemory(mDevice, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(mDevice, stagingBufferMemory);

        stbi_image_free(pixels);

        NGL_LOGI("Creating texture image...");
        createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    mTextureImage, mTextureImageMemory);

        transitionImageLayout(mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        copyBufferToImage(stagingBuffer, mTextureImage, texWidth, texHeight);

        transitionImageLayout(mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
        vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
    }

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags memoryPropertyFlags, VkImage& image, VkDeviceMemory& imageMemory) {
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.format = format;
        imageCreateInfo.tiling = tiling;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.usage = usage;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.flags = 0;  // Optional
        NVK_CHECK(vkCreateImage(mDevice, &imageCreateInfo, nullptr, &image));
        NGL_LOGI("Created image: %p", reinterpret_cast<void*>(image));

        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(mDevice, image, &memoryRequirements);
        NGL_LOGI("Memory requirements for image %p:", reinterpret_cast<void*>(image));
        nvkDumpMemoryRequirements(memoryRequirements, "  ");

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memoryRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlags);

        NVK_CHECK(vkAllocateMemory(mDevice, &allocInfo, nullptr, &imageMemory));
        NGL_LOGI("Created image memory: %p", reinterpret_cast<void*>(imageMemory));

        NVK_CHECK(vkBindImageMemory(mDevice, image, imageMemory, 0));
    }

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (hasStencilComponent(format)) {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                   newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask =
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            NGL_ABORT("Unsupported image layout transition");
        }

        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        endSingleTimeCommands(commandBuffer);
    }

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};
        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        endSingleTimeCommands(commandBuffer);
    }

    void createTextureImageView() {
        mTextureImageView = createImageView(mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo viewCreateInfo{};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.image = image;
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.format = format;
        viewCreateInfo.subresourceRange.aspectMask = aspectFlags;
        viewCreateInfo.subresourceRange.baseMipLevel = 0;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;
        viewCreateInfo.subresourceRange.layerCount = 1;
        VkImageView result;
        NVK_CHECK(vkCreateImageView(mDevice, &viewCreateInfo, nullptr, &result));
        return result;
    }

    void createTextureSampler() {
        VkPhysicalDeviceProperties physicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(mPhysicalDevice, &physicalDeviceProperties);

        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.anisotropyEnable = VK_TRUE;
        samplerCreateInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
        samplerCreateInfo.compareEnable = VK_FALSE;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 0.0f;
        NVK_CHECK(vkCreateSampler(mDevice, &samplerCreateInfo, nullptr, &mTextureSampler));
    }

    void loadModel() {
        /*
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, kModelPath)) {
            NGL_ABORT("tinyobj::LoadObj failed, warn: %s, err: %s", warn.c_str(), err.c_str());
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                vertex.pos = {
                        attrib.vertices[3 * index.vertex_index + 0],  //
                        attrib.vertices[3 * index.vertex_index + 1],  //
                        attrib.vertices[3 * index.vertex_index + 2]   //
                };

                vertex.texCoord = {
                        attrib.texcoords[2 * index.texcoord_index + 0],       //
                        1.0 - attrib.texcoords[2 * index.texcoord_index + 1]  //
                };

                vertex.color = {1.0f, 1.0f, 1.0f};

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(mVertices.size());
                    mVertices.push_back(vertex);
                }
                mIndices.push_back(uniqueVertices[vertex]);
            }
        }
        */
        mVertices.push_back(Vertex{{-0.3, 0.35, 0}, {1, 1, 1}, {0, 0}});
        mVertices.push_back(Vertex{{0.3, 0.95, 0}, {1, 1, 1}, {1, 1}});
        mVertices.push_back(Vertex{{-0.3, 0.95, 0}, {1, 1, 1}, {0, 0}});
        mVertices.push_back(Vertex{{-0.3, 0.3, 0}, {1, 1, 1}, {0, 0}});
        mVertices.push_back(Vertex{{0.3, 0.3, 0}, {1, 1, 1}, {0, 0}});
        mVertices.push_back(Vertex{{0.3, 0.9, 0}, {1, 1, 1}, {0, 0}});
        mIndices.push_back(0);
        mIndices.push_back(1);
        mIndices.push_back(2);
        mIndices.push_back(3);
        mIndices.push_back(4);
        mIndices.push_back(5);
    }

    void createVertexBuffer() {
        VkDeviceSize bufferSize = sizeof(Vertex) * mVertices.size();

        NGL_LOGI("Creating staging buffer for vertex buffer...");
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                     stagingBufferMemory);

        void* data;
        vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, mVertices.data(), bufferSize);
        vkUnmapMemory(mDevice, stagingBufferMemory);

        NGL_LOGI("Creating vertex buffer...");
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mVertexBuffer, mVertexBufferMemory);

        copyBuffer(stagingBuffer, mVertexBuffer, bufferSize);

        vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
        vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
    }

    void createIndexBuffer() {
        VkDeviceSize bufferSize = sizeof(uint32_t) * mIndices.size();

        NGL_LOGI("Creating staging buffer for index buffer...");
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                     stagingBufferMemory);

        void* data;
        vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, mIndices.data(), bufferSize);
        vkUnmapMemory(mDevice, stagingBufferMemory);

        NGL_LOGI("Creating index buffer...");
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mIndexBuffer, mIndexBufferMemory);

        copyBuffer(stagingBuffer, mIndexBuffer, bufferSize);

        vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
        vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
    }

    void createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        mUniformBuffers.resize(kMaxFramesInFlight);
        mUniformBufferMemories.resize(kMaxFramesInFlight);
        mUniformBufferMappedAddresses.resize(kMaxFramesInFlight);

        for (size_t i = 0; i < kMaxFramesInFlight; i++) {
            NGL_LOGI("Creating uniform buffer %zu...", i);
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mUniformBuffers[i],
                         mUniformBufferMemories[i]);
            vkMapMemory(mDevice, mUniformBufferMemories[i], 0, bufferSize, 0, &mUniformBufferMappedAddresses[i]);
        }
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags,
                      VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = usage;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        NVK_CHECK(vkCreateBuffer(mDevice, &bufferCreateInfo, nullptr, &buffer));
        NGL_LOGI("Created buffer: %p", reinterpret_cast<void*>(buffer));

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(mDevice, buffer, &memoryRequirements);
        NGL_LOGI("Memory requirements for buffer %p:", reinterpret_cast<void*>(buffer));
        nvkDumpMemoryRequirements(memoryRequirements, "  ");

        uint32_t memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlags);
        NGL_LOGI("Required memory type: %u", memoryTypeIndex);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memoryRequirements.size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;
        NVK_CHECK(vkAllocateMemory(mDevice, &allocInfo, nullptr, &bufferMemory));
        NGL_LOGI("Allocated memory: %p", reinterpret_cast<void*>(bufferMemory));

        NVK_CHECK(vkBindBufferMemory(mDevice, buffer, bufferMemory, 0));
    }

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;  // Optional
        copyRegion.dstOffset = 0;  // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertyFlags) {
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memoryProperties);

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags) {
                return i;
            }
        }
        NGL_ABORT("No memory type for typeFilter: %xu", typeFilter);
    }

    void createDescriptorPool() {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = kMaxFramesInFlight;
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = kMaxFramesInFlight;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = kMaxFramesInFlight;
        NVK_CHECK(vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPool));
        NGL_LOGI("mDescriptorPool: %p", reinterpret_cast<void*>(mDescriptorPool));
    }

    void createDescriptorSets() {
        mDescriptorSets.resize(kMaxFramesInFlight);

        std::vector<VkDescriptorSetLayout> layouts(kMaxFramesInFlight, mDescriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = mDescriptorPool;
        allocInfo.descriptorSetCount = kMaxFramesInFlight;
        allocInfo.pSetLayouts = layouts.data();

        NVK_CHECK(vkAllocateDescriptorSets(mDevice, &allocInfo, mDescriptorSets.data()));

        for (size_t i = 0; i < kMaxFramesInFlight; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = mUniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = mTextureImageView;
            imageInfo.sampler = mTextureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = mDescriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = mDescriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(mDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                                   nullptr);
        }
    }

    void createCommandBuffers() {
        mCommandBuffers.resize(kMaxFramesInFlight);

        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = mCommandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());

        NVK_CHECK(vkAllocateCommandBuffers(mDevice, &allocateInfo, mCommandBuffers.data()));
        for (size_t i = 0; i < mCommandBuffers.size(); i++) {
            NGL_LOGI("mCommandBuffers[%zd]: %p", i, reinterpret_cast<void*>(mCommandBuffers[i]));
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo bufferBeginInfo{};
        bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        bufferBeginInfo.flags = 0;                   // Optional
        bufferBeginInfo.pInheritanceInfo = nullptr;  // Optional
        NVK_CHECK(vkBeginCommandBuffer(commandBuffer, &bufferBeginInfo));

        std::array<VkClearValue, 2> clearValues{};  // The order must match the order of attachments.
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = mRenderPass;
        renderPassBeginInfo.framebuffer = mSwapchainFramebuffers[imageIndex];
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = mSwapchainExtent;
        renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassBeginInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);

        VkBuffer vertexBuffers[] = {mVertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

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

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1,
                                &mDescriptorSets[mCurrentFrame], 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mIndices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        NVK_CHECK(vkEndCommandBuffer(commandBuffer));
    }

    void createSyncObjects() {
        mImageAvailableSemaphores.resize(kMaxFramesInFlight);
        mRenderFinishedSemaphores.resize(kMaxFramesInFlight);
        mInFlightFences.resize(kMaxFramesInFlight);

        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < kMaxFramesInFlight; i++) {
            NVK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mImageAvailableSemaphores[i]));
            NGL_LOGI("mImageAvailableSemaphores[%zd]: %p", i, reinterpret_cast<void*>(mImageAvailableSemaphores[i]));

            NVK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mRenderFinishedSemaphores[i]));
            NGL_LOGI("mRenderFinishedSemaphores[%zd]: %p", i, reinterpret_cast<void*>(mRenderFinishedSemaphores[i]));

            NVK_CHECK(vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &mInFlightFences[i]));
            NGL_LOGI("mInFlightFences[%zd]:           %p", i, reinterpret_cast<void*>(mInFlightFences[i]));
        }
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(mWindow)) {
            double time = glfwGetTime();
            mCamera.onNextFrame(time);

            glfwPollEvents();
            drawFrame();
        }
        NVK_CHECK(vkDeviceWaitIdle(mDevice));
    }

    void drawFrame() {
        NVK_CHECK(vkWaitForFences(mDevice, 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX));

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(mDevice, mSwapchain, UINT64_MAX,
                                                mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapchain();
            return;
        } else {
            NGL_VERIFY(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);
        }

        NVK_CHECK(vkResetFences(mDevice, 1, &mInFlightFences[mCurrentFrame]));

        NVK_CHECK(vkResetCommandBuffer(mCommandBuffers[mCurrentFrame], 0));

        recordCommandBuffer(mCommandBuffers[mCurrentFrame], imageIndex);

        updateUniformBuffer(mCurrentFrame);

        VkSemaphore waitSemaphores[] = {mImageAvailableSemaphores[mCurrentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphores[mCurrentFrame]};

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &mCommandBuffers[mCurrentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        NVK_CHECK(vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFences[mCurrentFrame]));

        VkSwapchainKHR swapChains[] = {mSwapchain};

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;  // Optional
        result = vkQueuePresentKHR(mPresentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFramebufferResized) {
            NGL_LOGI("Resize or swapchain incompatibility detected, recreating swapchain");
            mFramebufferResized = false;
            recreateSwapchain();
        } else {
            NGL_VERIFY(result == VK_SUCCESS);
        }

        mCurrentFrame = (mCurrentFrame + 1) % kMaxFramesInFlight;
    }

    void updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        // float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model_view = mCamera.getModelViewMatrix();
        ubo.proj = glm::perspective(45.0f, mSwapchainExtent.width / static_cast<float>(mSwapchainExtent.height), 0.1f,
                                    1000.0f);
        ubo.proj[1][1] *= -1;

        memcpy(mUniformBufferMappedAddresses[currentImage], &ubo, sizeof(ubo));
    }

    void terminate() {
        cleanupSwapchain();
        for (size_t i = 0; i < kMaxFramesInFlight; i++) {
            vkDestroyFence(mDevice, mInFlightFences[i], nullptr);
            vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
        }
        vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
        for (size_t i = 0; i < kMaxFramesInFlight; i++) {
            vkDestroyBuffer(mDevice, mUniformBuffers[i], nullptr);
            vkFreeMemory(mDevice, mUniformBufferMemories[i], nullptr);
        }
        vkDestroyBuffer(mDevice, mIndexBuffer, nullptr);
        vkFreeMemory(mDevice, mIndexBufferMemory, nullptr);
        vkDestroyBuffer(mDevice, mVertexBuffer, nullptr);
        vkFreeMemory(mDevice, mVertexBufferMemory, nullptr);
        vkDestroySampler(mDevice, mTextureSampler, nullptr);
        vkDestroyImageView(mDevice, mTextureImageView, nullptr);
        vkDestroyImage(mDevice, mTextureImage, nullptr);
        vkFreeMemory(mDevice, mTextureImageMemory, nullptr);
        vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
        vkDestroyPipeline(mDevice, mPipeline, nullptr);
        vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
        vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
        vkDestroyDevice(mDevice, nullptr);
        vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
        nvkTerminateDebugIfNecessary(mInstance);
        vkDestroyInstance(mInstance, nullptr);
        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }

    void cleanupSwapchain() {
        vkDestroyImageView(mDevice, mDepthImageView, nullptr);
        vkDestroyImage(mDevice, mDepthImage, nullptr);
        vkFreeMemory(mDevice, mDepthImageMemory, nullptr);

        for (auto framebuffer : mSwapchainFramebuffers) {
            vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
        }
        for (auto imageView : mSwapchainImageViews) {
            vkDestroyImageView(mDevice, imageView, nullptr);
        }
        vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
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
    VkDescriptorSetLayout mDescriptorSetLayout;
    VkPipelineLayout mPipelineLayout;
    VkPipeline mPipeline;
    std::vector<VkFramebuffer> mSwapchainFramebuffers;
    VkCommandPool mCommandPool;
    VkImage mDepthImage;
    VkDeviceMemory mDepthImageMemory;
    VkImageView mDepthImageView;
    VkImage mTextureImage;
    VkDeviceMemory mTextureImageMemory;
    VkImageView mTextureImageView;
    VkSampler mTextureSampler;
    std::vector<Vertex> mVertices;
    std::vector<uint32_t> mIndices;
    VkBuffer mVertexBuffer;
    VkDeviceMemory mVertexBufferMemory;
    VkBuffer mIndexBuffer;
    VkDeviceMemory mIndexBufferMemory;
    std::vector<VkBuffer> mUniformBuffers;
    std::vector<VkDeviceMemory> mUniformBufferMemories;
    std::vector<void*> mUniformBufferMappedAddresses;
    VkDescriptorPool mDescriptorPool;
    std::vector<VkDescriptorSet> mDescriptorSets;
    std::vector<VkCommandBuffer> mCommandBuffers;
    std::vector<VkSemaphore> mImageAvailableSemaphores;
    std::vector<VkSemaphore> mRenderFinishedSemaphores;
    std::vector<VkFence> mInFlightFences;
    uint32_t mCurrentFrame = 0;

    bool mFramebufferResized = false;

    NvkCamera mCamera{glm::vec3(0.0f, 1.6f, 1.6f), glm::vec3(0.0f, 0.6f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)};
};

int nvkMain() {
    HelloTriangleApplication app;
    app.run();
    return 0;
}
