#include "Engine.h"

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <limits>

#define TARGET_IMAGE_COUNT 2

int Engine::run(uint16_t width, uint16_t height, const std::string& title) {
    // Prepare GLFW to be used by this application.
    glfwInit();

    // Start with creating a vulkan instance
    // On slower devices the window will show up way before any rendering happens
    createInstance();

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

    createSwapchain();

    getSwapchainImages();
    createRenderPass();
    createFramebuffers();
    

    return 0;
}

void Engine::cleanUpVulkan() {
    for (const VkFramebuffer& framebuffer : m_framebuffers) {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }

    vkDestroyRenderPass(m_device, m_renderPass, nullptr);

    for (const VkImageView& view : m_swapchainImageViews) {
        vkDestroyImageView(m_device, view, nullptr);
    }

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

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

    std::array<char*, 1> extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = queueInfos.size(),
            .pQueueCreateInfos = queueInfos.data(),
            .enabledExtensionCount = extensions.size(),
            .ppEnabledExtensionNames = extensions.data(),
            .pEnabledFeatures = &features
    };

    if (vkCreateDevice(m_physicalDevice, &info, nullptr, &m_device) != VK_SUCCESS)
        throw std::runtime_error("failed to create device");
}

void Engine::createSwapchain() {
    VkSurfaceCapabilitiesKHR capabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities);

    // Find and use an supported image count
    int imageCount = TARGET_IMAGE_COUNT;
    if (imageCount < capabilities.minImageCount)
        imageCount = capabilities.minImageCount;
    if (capabilities.maxImageCount != 0)
        if (imageCount > capabilities.maxImageCount)
            imageCount = capabilities.maxImageCount;

    // Find suitable format
    selectSwapchainFormat();

    // Create swapchain extent
    getSwapchainExtent(capabilities);

    // Get the optimal present mode
    getSwapchainPresentMode();

    VkSwapchainCreateInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.pNext = nullptr;
    info.flags = 0;
    info.surface = m_surface;
    info.minImageCount = imageCount;
    info.imageFormat = m_swapchainFormat.format;
    info.imageColorSpace =  m_swapchainFormat.colorSpace;
    info.imageExtent = m_swapchainExtent;
    info.imageArrayLayers = 1;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (m_graphicsFamilyIndex != m_presentFamilyIndex) {
        std::array<uint32_t, 2> families = {m_graphicsFamilyIndex, m_presentFamilyIndex};
        info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        info.queueFamilyIndexCount = families.size();
        info.pQueueFamilyIndices = families.data();
    } else {
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    info.preTransform = capabilities.currentTransform;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.presentMode = m_presentMode;
    info.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(m_device, &info, nullptr, &m_swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create a swapchain!");
    }
}

void Engine::createRenderPass() {
    // Information about the attachments for the render pass
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.flags = 0;
    colorAttachment.format = m_swapchainFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;

    // Information Vulkan needs to create a render pass
    VkRenderPassCreateInfo info {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.attachmentCount = 1;
    info.pAttachments = &colorAttachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 0;
    info.pDependencies = nullptr;


    // Create the render pass
    if (vkCreateRenderPass(m_device, &info, nullptr, &m_renderPass))
        throw std::runtime_error("failed to create a render pass");
}

void Engine::createFramebuffers() {
    VkFramebufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.renderPass = m_renderPass;
    info.attachmentCount = 1;
    info.width = m_swapchainExtent.width;
    info.height = m_swapchainExtent.height;
    info.layers = 1;

    // Prepare vector for values that will be copied in.
    m_framebuffers.resize(m_swapchainImages.size());
    for (int i = 0; i < m_swapchainImages.size(); i++) {
        info.pAttachments = &m_swapchainImageViews[i];
        if(vkCreateFramebuffer(m_device, &info, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create a framebuffer");
    }
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

void Engine::selectSwapchainFormat() {
    // Get count of formats to create a vector
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
    // Fill in vector with formats
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, formats.data());

    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM
                && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            m_swapchainFormat = format;
            return;
        }
    } 
    if (formats.size() > 0)
        m_swapchainFormat = formats[0];
    else
        m_swapchainFormat = {
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        };
}

void Engine::getSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    // If capabilities has the width, use the capabilities
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        m_swapchainExtent = capabilities.currentExtent;
        return;
    } else {
        // Get width and height of the window's framebuffer
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);

        // Clamp those numbers to the limits of the surface
        uint32_t _width = std::clamp(static_cast<uint32_t>(width),
                            capabilities.minImageExtent.width,
                            capabilities.maxImageExtent.width);
        uint32_t _height = std::clamp(static_cast<uint32_t>(height),
                            capabilities.minImageExtent.height,
                            capabilities.maxImageExtent.height);

        // Create the extent with those width and height numbers
        m_swapchainExtent = {
            _width,
            _height
        };
    }
}

void Engine::getSwapchainPresentMode() {
    // Get count so we can make an array
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr);

    // Fill in that array
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, presentModes.data());

    // Cycle through all the available present modes, and choose mailbox if available.
    for (auto& presentMode : presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            m_presentMode = presentMode;
            return;
        }
    }
    // Use FIFO if mailbox is not available
    m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
}

void Engine::getSwapchainImages() {
    // Get image count so we can make a array of images.
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);

    // Fill in that vector with the swapchain images.
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());

    // Prepare for image views
    m_swapchainImageViews.resize(imageCount);

    for (int i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.image = m_swapchainImages[i];
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = m_swapchainFormat.format;
        info.components.r = VK_COMPONENT_SWIZZLE_R;
        info.components.g = VK_COMPONENT_SWIZZLE_G;
        info.components.b = VK_COMPONENT_SWIZZLE_B;
        info.components.a = VK_COMPONENT_SWIZZLE_A;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_device, &info, nullptr, &m_swapchainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create a image view");
        }

    }
}
