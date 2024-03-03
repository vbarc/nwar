#include "nvkmain.h"

#include <cstdlib>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "ngllog.h"
#include "nvkerr.h"

#define __NVK_DEBUG_ENABLED

constexpr uint32_t kWidth = 1920;
constexpr uint32_t kHeight = 1080;

const std::vector<const char*> kLayers = {
#ifdef __NVK_DEBUG_ENABLED
        "VK_LAYER_KHRONOS_validation"
#endif  // __NVK_DEBUG_ENABLED
};

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
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

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        createInfo.enabledLayerCount = static_cast<uint32_t>(kLayers.size());
        createInfo.ppEnabledLayerNames = kLayers.data();

        NVK_CHECK(vkCreateInstance(&createInfo, nullptr, &mInstance));
        NGL_LOGI("mInstance: %p", reinterpret_cast<void*>(mInstance));
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(mWindow)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        vkDestroyInstance(mInstance, nullptr);
        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }

    GLFWwindow* mWindow;
    VkInstance mInstance;
};

int nvkMain() {
    HelloTriangleApplication app;
    app.run();
    return 0;
}
