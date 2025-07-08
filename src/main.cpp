#include <memory>
#include <chrono>

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "Renderer.hpp"
#include "FontAtlas.hpp"

Renderer renderer;
int8_t keys_[1024];
double scrollWheel_;


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
        {
            keys_[key] = 3;
        }
        else if (action == GLFW_RELEASE)
        {
            keys_[key] = 0;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    scrollWheel_ = yoffset;
}

bool GetKeyDown(int key)
{
    return keys_[key] & 2;
}

bool GetKey(int key)
{
    return keys_[key] & 3;
}

void HandleCamera(double deltaTimeMilliSeconds)
{
    glm::vec2 currentCameraPosition = renderer.GetCameraPosition();
    glm::vec2 cameraChange = glm::vec2(0, 0);
    const float speed = 0.1f;

    if (GetKey(GLFW_KEY_LEFT) || GetKey(GLFW_KEY_A))
    {
        cameraChange -= glm::vec2(speed * deltaTimeMilliSeconds, 0);
    }

    if (GetKey(GLFW_KEY_RIGHT) || GetKey(GLFW_KEY_D))
    {
        cameraChange += glm::vec2(speed * deltaTimeMilliSeconds, 0);
    }

    if (GetKey(GLFW_KEY_UP) || GetKey(GLFW_KEY_W))
    {
        cameraChange += glm::vec2(0, speed * deltaTimeMilliSeconds);
    }

    if (GetKey(GLFW_KEY_DOWN) || GetKey(GLFW_KEY_S))
    {
        cameraChange -= glm::vec2(0, speed * deltaTimeMilliSeconds);
    }
    renderer.SetCameraPosition(currentCameraPosition + cameraChange);

    renderer.SetZoom(renderer.GetZoom() + scrollWheel_);
    if (renderer.GetZoom() < 0)
    {
       //renderer.SetZoom(0);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: opengl_msdfatlas <path of font file>\n");
        printf("e.g. opengl_msdfatlas JupiteroidRegular.ttf");
        return 1;
    }

    //glm::vec2 windowSize = glm::vec2(2560, 1440);
    glm::vec2 windowSize = glm::vec2(1280, 720);
    GLFWwindow* window = renderer.CreateWindow("OpenGL - msdf - atlas", windowSize, glm::vec2(160, 90));
    
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);

    FontAtlas arial = FontAtlas(argv[1]);

    auto currentFrame = std::chrono::steady_clock::now();
    auto lastFrame = std::chrono::steady_clock::now();

    auto lastFpsDisplayTime = std::chrono::steady_clock::now();
    int fps = 0;
    int fpsDisplay = 0;
    while (!glfwWindowShouldClose(window))
    {
        // calculate delta time used for fps independent movement
        currentFrame = std::chrono::steady_clock::now();

        if (std::chrono::duration_cast<std::chrono::microseconds>(currentFrame - lastFpsDisplayTime).count() >= 1000000)
        {
            lastFpsDisplayTime = currentFrame;
            //printf("Fps: %d\n", fps);
            fpsDisplay = fps;
            fps = 0;
        }

        auto deltaTimeMilliSeconds = std::chrono::duration_cast<std::chrono::microseconds>(currentFrame - lastFrame).count() / 1000.f;

        // clear last frame
        glClearColor(0.3f, 0.2f, 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // handle user camera movement
        HandleCamera(deltaTimeMilliSeconds);
        
        // setup shader inputs
        renderer.BeginFrame();

        // disable depth testing to prevent issues with slightly  overlapping characters
        glDisable(GL_DEPTH_TEST);

        glm::vec4 red = glm::vec4(1, 0, 0, 1);
        glm::vec4 white = glm::vec4(1, 1, 1, 1);
        glm::vec4 green = glm::vec4(0, 1, 0, 1);

        // draw the text
        renderer.DrawText(arial, "Controls\n\tMove camera:\n\t\twasd/arrowkeys\n\tZoom:\n\t\tscrollwheel", glm::vec3(-80, 30, 0), 2, white, false);
        renderer.DrawText(arial, "LEFT aligned", glm::vec3(0, 10, 0), 10, green, false);
        renderer.DrawText(arial, "I'm a centered text\nwith several\nrows!", glm::vec3(0, -20, 0), 4, white);

        const int count = 200;
        std::string text = "Render this " + std::to_string(count) + " times";
        for (int i = 0; i < count; i++)
        {
            renderer.DrawText(arial, text, glm::vec3(0, -i*0.1, 0), 10, white);
        }

        // enable depth testing
        glEnable(GL_DEPTH_TEST);
        
        renderer.EndFrame(arial);

        // draw frame
        glfwSwapBuffers(window);

        // reset input
        for (int i = 0; i < 1024; i++)
        {
            keys_[i] &= 1;
        }
        scrollWheel_ = 0;

        // poll new input
        glfwPollEvents();

        lastFrame = currentFrame;
        fps++;
    }
}