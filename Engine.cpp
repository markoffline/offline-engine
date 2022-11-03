#include "Engine.h"

#include <iostream>
#include <stdexcept>

int Engine::run(uint16_t width, uint16_t height, const std::string& title) {
    // Start with creating a vulkan instance
    // On slower devices the window will show up way before any rendering happens
    createInstance();

    // Prepare GLFW to be used by this application.
    glfwInit();

    // Tell GLFW to create a window without OpenGL
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Create the window and surface
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface);


    initializeVulkan();

    initialized = true;
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
    selectPhysicalDevice();
    getQueueFamilyIndices();

    createDevice();

    return 0;
}

void Engine::cleanUpVulkan() {
    vkDestroyDevice(m_device, nullptr);

    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

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

void Engine::createDevice() {
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo graphicsQueueInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = m_graphicsFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
    };
    VkDeviceQueueCreateInfo presentQueueInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = m_presentFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
    };
    std::array<VkDeviceQueueCreateInfo, 2> queueInfos = {graphicsQueueInfo, presentQueueInfo};

    VkPhysicalDeviceFeatures features = {};

    VkDeviceCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = queueInfos.size(),
            .pQueueCreateInfos = queueInfos.data(),
            .pEnabledFeatures = &features
    };

    if (vkCreateDevice(m_physicalDevice, &info, nullptr, &m_device) != VK_SUCCESS)
        throw std::runtime_error("failed to create device");
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

void Engine::getQueueFamilyIndices() {
    // Get count of queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
    // Fill vector with queue family properties
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

    // Get queue indices
    int i = 0;
    for (VkQueueFamilyProperties queueFamily : queueFamilies) {
        // Get graphics queue index
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            m_graphicsFamilyIndex = i;

        // Check if supports presenting
        VkBool32 supportsPresenting = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &supportsPresenting);
        if (supportsPresenting)
            m_presentFamilyIndex = i;

        i++;
    }
}


