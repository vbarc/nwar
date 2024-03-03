#include "nvkmain.h"

#include <cstdlib>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "ngllog.h"

constexpr uint32_t kWidth = 1920;
constexpr uint32_t kHeight = 1080;

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
        for (uint32_t i = 0; i < glfwExtensionCount; i++) {
            NGL_LOGI("glfwExtensions[%u]: %s", i, glfwExtensions[i]);
        }

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;

        VkResult result = vkCreateInstance(&createInfo, nullptr, &mInstance);
        if (result != VK_SUCCESS) {
            NGL_LOGE("vkCreateInstance");
            abort();
        }
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
