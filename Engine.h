//
// Created by markoffline on 11/2/22.
//

#ifndef VK_SAMPLE_ENGINE_H
#define VK_SAMPLE_ENGINE_H

#include <cinttypes>
#include <string>
#include <vector>

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
    VkPhysicalDevice m_physicalDevice;

    uint32_t m_graphicsFamilyIndex = -1;

    // Functions
    void mainLoop();

    // Vulkan
    int initializeVulkan();
    void cleanUpVulkan();

    void createInstance();

    void selectPhysicalDevice();
    void getQueueFamilyIndices();
};

#endif //VK_SAMPLE_ENGINE_H
