//
// Created by markoffline on 11/2/22.
//

#ifndef VK_SAMPLE_ENGINE_H
#define VK_SAMPLE_ENGINE_H

#include <cinttypes>
#include <string>
#include <vector>
#include <array>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Engine {
public:
    Engine() = default;

    int run(uint16_t width, uint16_t height, const std::string& title);

private:
    // Members
    bool initialized = false;

    GLFWwindow* m_window;

    // Vulkan Members
    VkInstance m_instance;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    VkRenderPass m_renderPass;
    VkCommandPool m_commandPool;

    VkSwapchainKHR m_swapchain;
    VkSurfaceFormatKHR m_swapchainFormat;
    VkExtent2D m_swapchainExtent;
    VkPresentModeKHR m_presentMode;

    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;

    std::vector<VkFramebuffer> m_framebuffers;

    std::vector<VkCommandBuffer> m_commandBuffers;

    uint32_t m_graphicsFamilyIndex = -1;
    uint32_t m_presentFamilyIndex = -1;

    // Functions
    void mainLoop();

    // Vulkan
    int initializeVulkan();
    void cleanUpVulkan();

    void createInstance();
    void createDevice();
    void createSwapchain();
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();

    void selectPhysicalDevice();
    void getQueueFamilyIndices();
    void selectSwapchainFormat();
    void getSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void getSwapchainPresentMode();

    void getSwapchainImages();

    void allocCommandBuffers();
};

#endif //VK_SAMPLE_ENGINE_H
