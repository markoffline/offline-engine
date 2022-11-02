#include "Engine.h"

int Engine::run(uint16_t width, uint16_t height, const std::string& title) {
    // Prepare GLFW to be used by this application.
    glfwInit();

    // Tell GLFW to create a window without OpenGL
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Create the window
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    initialized = true;

    // Run the application as normal
    mainLoop();

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
