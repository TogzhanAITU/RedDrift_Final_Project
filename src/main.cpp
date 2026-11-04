#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include "Shader.hpp"
#include "Level.hpp"
#include "Player.hpp"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

GLFWwindow* window = nullptr;
bool isFullscreen = false;
int windowedX = 100, windowedY = 100, windowedWidth = SCR_WIDTH, windowedHeight = SCR_HEIGHT;

int currentLevelIndex = 1;
Level currentLevel;
Player player(glm::vec3(0.0f, 2.0f, 0.0f));

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, float dt) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    static bool f11Pressed = false;
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
        if (!f11Pressed) {
            isFullscreen = !isFullscreen;
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            if (isFullscreen) {
                glfwGetWindowPos(window, &windowedX, &windowedY);
                glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
                glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
            } else {
                glfwSetWindowMonitor(window, nullptr, windowedX, windowedY, windowedWidth, windowedHeight, 0);
            }
            f11Pressed = true;
        }
    } else {
        f11Pressed = false;
    }

    static bool gPressed = false;
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        if (!gPressed) {
            player.enableGravity = !player.enableGravity;
            std::cout << "Gravity Enabled: " << (player.enableGravity ? "True" : "False") << std::endl;
            gPressed = true;
        }
    } else {
        gPressed = false;
    }

    static bool fPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!fPressed) {
            player.gravityDirection *= -1;
            std::cout << "Gravity Direction: " << (player.gravityDirection > 0 ? "Normal (Down)" : "Inverted (Up)") << std::endl;
            fPressed = true;
        }
    } else {
        fPressed = false;
    }

    static bool iPressed = false;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
        if (!iPressed) {
            player.enableInertia = !player.enableInertia;
            std::cout << "Inertia: " << (player.enableInertia ? "True" : "False") << std::endl;
            iPressed = true;
        }
    } else {
        iPressed = false;
    }

    float moveSpeed = 6.0f;
    float targetVx = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        targetVx = -moveSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        targetVx = moveSpeed;
    }

    if (player.enableInertia) {
        player.velocity.x += (targetVx - player.velocity.x) * 10.0f * dt;
    } else {
        player.velocity.x = targetVx;
    }

    static bool spacePressed = false;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (!spacePressed) {
            player.jump();
            spacePressed = true;
        }
    } else {
        spacePressed = false;
    }
}

void loadLevel(int levelNum) {
    std::string path = "levels/level" + std::to_string(levelNum) + ".txt";
    if (currentLevel.loadFromFile(path)) {
        std::cout << "Loaded Level " << levelNum << std::endl;
        player.position = glm::vec3(0.0f, 2.0f, 0.0f);
        player.velocity = glm::vec3(0.0f);
    } else {
        std::cerr << "Failed to load level file: " << path << std::endl;
    }
}

int main() {
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Puzzle Platformer", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");

    float cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,  0.8f, 0.2f, 0.2f,
         0.5f, -0.5f, -0.5f,  0.8f, 0.2f, 0.2f,
         0.5f,  0.5f, -0.5f,  0.8f, 0.2f, 0.2f,
         0.5f,  0.5f, -0.5f,  0.8f, 0.2f, 0.2f,
        -0.5f,  0.5f, -0.5f,  0.8f, 0.2f, 0.2f,
        -0.5f, -0.5f, -0.5f,  0.8f, 0.2f, 0.2f,

        -0.5f, -0.5f,  0.5f,  0.2f, 0.8f, 0.2f,
         0.5f, -0.5f,  0.5f,  0.2f, 0.8f, 0.2f,
         0.5f,  0.5f,  0.5f,  0.2f, 0.8f, 0.2f,
         0.5f,  0.5f,  0.5f,  0.2f, 0.8f, 0.2f,
        -0.5f,  0.5f,  0.5f,  0.2f, 0.8f, 0.2f,
        -0.5f, -0.5f,  0.5f,  0.2f, 0.8f, 0.2f,

        -0.5f,  0.5f,  0.5f,  0.2f, 0.2f, 0.8f,
        -0.5f,  0.5f, -0.5f,  0.2f, 0.2f, 0.8f,
        -0.5f, -0.5f, -0.5f,  0.2f, 0.2f, 0.8f,
        -0.5f, -0.5f, -0.5f,  0.2f, 0.2f, 0.8f,
        -0.5f, -0.5f,  0.5f,  0.2f, 0.2f, 0.8f,
        -0.5f,  0.5f,  0.5f,  0.2f, 0.2f, 0.8f,

         0.5f,  0.5f,  0.5f,  0.8f, 0.8f, 0.2f,
         0.5f,  0.5f, -0.5f,  0.8f, 0.8f, 0.2f,
         0.5f, -0.5f, -0.5f,  0.8f, 0.8f, 0.2f,
         0.5f, -0.5f, -0.5f,  0.8f, 0.8f, 0.2f,
         0.5f, -0.5f,  0.5f,  0.8f, 0.8f, 0.2f,
         0.5f,  0.5f,  0.5f,  0.8f, 0.8f, 0.2f,

        -0.5f, -0.5f, -0.5f,  0.2f, 0.8f, 0.8f,
         0.5f, -0.5f, -0.5f,  0.2f, 0.8f, 0.8f,
         0.5f, -0.5f,  0.5f,  0.2f, 0.8f, 0.8f,
         0.5f, -0.5f,  0.5f,  0.2f, 0.8f, 0.8f,
        -0.5f, -0.5f,  0.5f,  0.2f, 0.8f, 0.8f,
        -0.5f, -0.5f, -0.5f,  0.2f, 0.8f, 0.8f,

        -0.5f,  0.5f, -0.5f,  0.8f, 0.2f, 0.8f,
         0.5f,  0.5f, -0.5f,  0.8f, 0.2f, 0.8f,
         0.5f,  0.5f,  0.5f,  0.8f, 0.2f, 0.8f,
         0.5f,  0.5f,  0.5f,  0.8f, 0.2f, 0.8f,
        -0.5f,  0.5f,  0.5f,  0.8f, 0.2f, 0.8f,
        -0.5f,  0.5f, -0.5f,  0.8f, 0.2f, 0.8f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    loadLevel(currentLevelIndex);

    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        float dt = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (dt > 0.1f) dt = 0.1f;

        processInput(window, dt);

        player.update(dt, currentLevel.platforms, currentLevel.height);

        if (player.position.y > currentLevel.height) {
            if (currentLevelIndex < 4) {
                currentLevelIndex++;
                std::cout << "Advancing to Level " << currentLevelIndex << std::endl;
                loadLevel(currentLevelIndex);
            } else {
                player.position.y = currentLevel.height;
                player.velocity.y = 0.0f;
            }
        }

        glClearColor(0.15f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspectRatio = (height > 0) ? (float)width / (float)height : 16.0f / 9.0f;

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
        glm::vec3 cameraPos = glm::vec3(player.position.x, player.position.y, 12.0f);
        glm::vec3 cameraTarget = glm::vec3(player.position.x, player.position.y, 0.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        glBindVertexArray(VAO);

        for (const auto& plat : currentLevel.platforms) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, plat.position);
            model = glm::scale(model, plat.size);
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, player.position);
            model = glm::scale(model, player.size);
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}