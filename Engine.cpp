#include "Engine.h"

#include <iostream>
#include <stdexcept>

int Engine::run(uint16_t width, uint16_t height, const std::string& title) {
    // Prepare GLFW to be used by this application.
    glfwInit();

    // Tell GLFW to create a window without OpenGL
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Create the window
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    initialized = true;

    initializeVulkan();

    // Run the application as normal
    mainLoop();

    // Cleanup Vulkan
    cleanUpVulkan();

    // Delete Window
    glfwDestroyWindow(m_window);

    // Cleanup GLFW
    glfwTerminate();

    return 0;
}

void Engine::mainLoop() {
    // While the window is running, poll the window and render to the screen.
    while(!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
}

int Engine::initializeVulkan() {
    createInstance();

    selectPhysicalDevice();


    return 0;
}

void Engine::cleanUpVulkan() {
    vkDestroyInstance(m_instance, nullptr);
}

void Engine::createInstance() {
    // Application Info
    VkApplicationInfo appInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Sample by Mark's Offline",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "Sample",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0)
    };

    uint32_t extCount = 0;
    const char** glfwExts =
        glfwGetRequiredInstanceExtensions(&extCount);

    // Info Vulkan needs to create a instance
    VkInstanceCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = extCount,
            .ppEnabledExtensionNames = glfwExts
    };

    // Create the instance
    if (vkCreateInstance(&info, nullptr, &m_instance) != VK_SUCCESS)
        throw std::runtime_error("failed to create Vulkan instance");

    std::cout << "Created Vulkan Instance!" << std::endl;
}

void Engine::selectPhysicalDevice() {
    // Get count of all physical devices so we can make a vector
    uint32_t devCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &devCount, nullptr);
    // Fill in vector of physical devices
    std::vector<VkPhysicalDevice> devices(devCount);
    vkEnumeratePhysicalDevices(m_instance, &devCount, devices.data());

    int i = 0;
    for(VkPhysicalDevice device : devices) {
        // TODO: Use the best GPU available
        m_physicalDevice = device;
        break;
    }
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);
    std::cout << "Selected physical device [" << i << "] - " << properties.deviceName << std::endl;

}

