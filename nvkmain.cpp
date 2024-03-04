#include "nvkmain.h"

#include <cstdlib>
#include <optional>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "ngllog.h"
#include "nvkdbg.h"
#include "nvkerr.h"
#include "nvkutil.h"

constexpr uint32_t kWidth = 1920;
constexpr uint32_t kHeight = 1080;

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
        nvkDumpPhysicalDevices(mInstance);
        selectPhysicalDevice();
        nvkDumpQueueFamilies(mPhysicalDevice);
    }

    void createInstance() {
        NGL_LOGI("Available extensions:");
        uint32_t extensionCount;
        NVK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        for (VkExtensionProperties extension : extensions) {
            NGL_LOGI("  %s %u", extension.extensionName, extension.specVersion);
        }

        NGL_LOGI("Available layers:");
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
        NGL_LOGI("Extensions required by GLFW:");
        for (uint32_t i = 0; i < glfwExtensionCount; i++) {
            NGL_LOGI("  %s", glfwExtensions[i]);
        }

        std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        nvkAppendDebugExtensionsIfNecessary(requiredExtensions);
        NGL_LOGI("Required extensions:");
        for (const char* requiredExtension : requiredExtensions) {
            NGL_LOGI("  %s", requiredExtension);
        }

        std::vector<const char*> requiredLayers;
        nvkAppendDebugLayersIfNecessary(requiredLayers);
        NGL_LOGI("Required layers:");
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

    void selectPhysicalDevice() {
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

        NGL_LOGI("Selected physical device: %p", reinterpret_cast<void*>(mPhysicalDevice));
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(device);
        return queueFamilyIndices.isComplete();
    }

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;

        bool isComplete() {
            return graphicsFamily.has_value();
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
            if (result.isComplete()) {
                break;
            }
        }

        return result;
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(mWindow)) {
            glfwPollEvents();
        }
    }

    void terminate() {
        nvkTerminateDebugIfNecessary(mInstance);
        vkDestroyInstance(mInstance, nullptr);
        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }

    GLFWwindow* mWindow = nullptr;
    VkInstance mInstance = VK_NULL_HANDLE;
    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
};

int nvkMain() {
    HelloTriangleApplication app;
    app.run();
    return 0;
}
