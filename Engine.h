//
// Created by markoffline on 11/2/22.
//

#ifndef VK_SAMPLE_ENGINE_H
#define VK_SAMPLE_ENGINE_H

#include <cinttypes>
#include <string>

#include <GLFW/glfw3.h>

class Engine {
public:
    Engine() = default;

    int run(uint16_t width, uint16_t height, const std::string& title);

private:
    // Members
    bool initialized = false;

    GLFWwindow* m_window;

    // Functions
    void mainLoop();

    int initializeVulkan();
};

#endif //VK_SAMPLE_ENGINE_H
