#define GLFW_INCLUDE_NONE

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <algorithm>

unsigned int SCR_WIDTH;
unsigned int SCR_HEIGHT;

struct GameObject {
    glm::vec2 position;
    glm::vec2 size;
    glm::vec3 color;
    bool isSpike;
};

GameObject player;
std::vector<GameObject> obstacles;
GameObject ground;

enum GameState {
    MENU,
    PLAYING,
    GAME_OVER
};

GameState currentState = MENU;
int currentLevel = 1;
float levelTime = 0.0f;
float PLAYER_SPEED = 5.0f;
const float JUMP_FORCE = 8.0f;
const float GRAVITY = -18.0f;
const float MAX_JUMP_HEIGHT = 0.8f;
float playerVelocityY = 0.0f;
bool isJumping = false;
float initialJumpY = 0.0f;

float VISIBLE_WIDTH = 16.0f;
float VISIBLE_HEIGHT = 12.0f;

const float OBSTACLE_SPAWN_DISTANCE = 5.0f;
float obstacleSpawnTimer = 0.0f;

bool checkCollision(const GameObject& a, const GameObject& b);

void setupProjection();

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    VISIBLE_WIDTH = 16.0f * (float)width / (float)height;
    VISIBLE_HEIGHT = 12.0f;
    setupProjection();
}

void processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (currentState == PLAYING) {
        player.position.x += PLAYER_SPEED * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !isJumping) {
            playerVelocityY = JUMP_FORCE;
            isJumping = true;
            initialJumpY = player.position.y;
        }

        playerVelocityY += GRAVITY * deltaTime;
        player.position.y += playerVelocityY * deltaTime;

        if (player.position.y > initialJumpY + MAX_JUMP_HEIGHT) {
            player.position.y = initialJumpY + MAX_JUMP_HEIGHT;
            playerVelocityY = 0.0f;
        }

        if (player.position.y <= ground.position.y + ground.size.y) {
            player.position.y = ground.position.y + ground.size.y;
            playerVelocityY = 0.0f;
            isJumping = false;
        }
    }
}

void drawRectangle(const GameObject& obj) {
    glPushMatrix();
    glBegin(GL_QUADS);
    glColor3f(obj.color.r, obj.color.g, obj.color.b);
    glVertex2f(obj.position.x, obj.position.y);
    glVertex2f(obj.position.x + obj.size.x, obj.position.y);
    glVertex2f(obj.position.x + obj.size.x, obj.position.y + obj.size.y);
    glVertex2f(obj.position.x, obj.position.y + obj.size.y);
    glEnd();
    glPopMatrix();
}

void drawSpike(const GameObject& obj) {
    glPushMatrix();
    glBegin(GL_TRIANGLES);
    glColor3f(obj.color.r, obj.color.g, obj.color.b);
    glVertex2f(obj.position.x, obj.position.y);
    glVertex2f(obj.position.x + obj.size.x, obj.position.y);
    glVertex2f(obj.position.x + obj.size.x / 2, obj.position.y + obj.size.y);
    glEnd();
    glPopMatrix();
}

void drawText(const std::string& text, float x, float y, float scale, glm::vec3 color) {
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1);
    glColor3f(color.r, color.g, color.b);
    for (char c : text) {
        glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, c);
    }
    glPopMatrix();
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (currentState == MENU && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        float glX = static_cast<float>((xpos / SCR_WIDTH) * VISIBLE_WIDTH - VISIBLE_WIDTH / 2);
        float glY = static_cast<float>((SCR_HEIGHT - ypos) / SCR_HEIGHT * VISIBLE_HEIGHT - VISIBLE_HEIGHT / 2);

        if (glX >= -2.0f && glX <= 2.0f && glY >= 0.5f && glY <= 2.0f) {
            currentState = PLAYING;
            currentLevel = 1;
            PLAYER_SPEED = 5.0f;
            levelTime = 0.0f;
            obstacleSpawnTimer = 0.0f;
            player.position = { 0.0f, ground.position.y + ground.size.y + 0.15f };
            obstacles.clear();
        }
        else if (glX >= -2.0f && glX <= 2.0f && glY >= -2.0f && glY <= -0.5f) {
            glfwSetWindowShouldClose(window, true);
        }
    }
}

void drawMenu() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    drawText("Square Dash", -2.5f, 3.0f, 0.003f, glm::vec3(1.0f, 1.0f, 1.0f));

    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-2.0f, 0.5f);
    glVertex2f(2.0f, 0.5f);
    glVertex2f(2.0f, 2.0f);
    glVertex2f(-2.0f, 2.0f);
    glEnd();
    drawText("Play", -0.6f, 1.0f, 0.002f, glm::vec3(0.0f, 0.0f, 0.0f));

    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-2.0f, -2.0f);
    glVertex2f(2.0f, -2.0f);
    glVertex2f(2.0f, -0.5f);
    glVertex2f(-2.0f, -0.5f);
    glEnd();
    drawText("Quit", -0.6f, -1.5f, 0.002f, glm::vec3(0.0f, 0.0f, 0.0f));
}

void spawnObstacle() {
    static std::default_random_engine generator(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
    static std::uniform_real_distribution<float> distribution(0.3f, 0.5f);

    float obstacleHeight = distribution(generator);
    float spawnX = player.position.x + VISIBLE_WIDTH / 2 + 1.0f;

    GameObject obstacle;
    obstacle.position = { spawnX, ground.position.y + ground.size.y };
    obstacle.size = { 0.3f, obstacleHeight };

    if (currentLevel == 1) {
        obstacle.color = { 1.0f, 0.0f, 0.0f };
        obstacle.isSpike = false;
    }
    else if (currentLevel == 2) {
        obstacle.color = { 1.0f, 0.5f, 0.0f };
        obstacle.isSpike = true;
    }
    else if (currentLevel == 3) {
        if (distribution(generator) > 0.5f) {
            obstacle.color = { 1.0f, 0.0f, 0.0f };
            obstacle.isSpike = false;
        }
        else {
            obstacle.color = { 1.0f, 0.5f, 0.0f };
            obstacle.isSpike = true;
        }
    }

    obstacles.push_back(obstacle);
}

void cleanupObstacles() {
    obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(),
        [&](const GameObject& obs) { return obs.position.x < player.position.x - VISIBLE_WIDTH / 2 - 1.0f; }),
        obstacles.end());
}

bool checkCollision(const GameObject& a, const GameObject& b) {
    bool xCollision = (a.position.x < b.position.x + b.size.x &&
        a.position.x + a.size.x > b.position.x);
    bool yCollision = (a.position.y < b.position.y + b.size.y &&
        a.position.y + a.size.y > b.position.y);

    return xCollision && yCollision &&
        (a.position.x + a.size.x > b.position.x + 0.01f &&
            a.position.x < b.position.x + b.size.x - 0.01f);
}

void setupProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-VISIBLE_WIDTH / 2, VISIBLE_WIDTH / 2, -VISIBLE_HEIGHT / 2, VISIBLE_HEIGHT / 2, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    SCR_WIDTH = mode->width;
    SCR_HEIGHT = mode->height;

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Square Dash", primaryMonitor, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    player = { {0.0f, 0.1f}, {0.3f, 0.3f}, {0.0f, 0.0f, 1.0f}, false };
    ground = { {-VISIBLE_WIDTH / 2, -VISIBLE_HEIGHT / 2}, {VISIBLE_WIDTH * 100, 0.1f}, {0.5f, 0.5f, 0.5f}, false };

    float lastFrame = 0.0f;

    setupProjection();

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, deltaTime);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (currentState == MENU) {
            drawMenu();
        }
        else if (currentState == PLAYING) {
            levelTime += deltaTime;
            obstacleSpawnTimer += deltaTime;

            if (obstacleSpawnTimer >= 2.0f) {
                spawnObstacle();
                obstacleSpawnTimer = 0.0f;
            }

            cleanupObstacles();

            glLoadIdentity();
            glTranslatef(-player.position.x + VISIBLE_WIDTH / 3, 0, 0.0f);

            drawRectangle(ground);
            drawRectangle(player);

            for (const auto& obstacle : obstacles) {
                if (obstacle.isSpike) {
                    drawSpike(obstacle);
                }
                else {
                    drawRectangle(obstacle);
                }
                if (checkCollision(player, obstacle)) {
                    currentState = GAME_OVER;
                }
            }

            glLoadIdentity();
            char levelText[50];
            sprintf(levelText, "Level: %d  Time: %.1f", currentLevel, levelTime);
            drawText(levelText, -VISIBLE_WIDTH / 2 + 0.2f, VISIBLE_HEIGHT / 2 - 0.5f, 0.001f, glm::vec3(1.0f, 1.0f, 1.0f));

            if (levelTime >= 15.0f) {
                if (currentLevel == 2) {
                    currentState = GAME_OVER;
                }
                else {
                    currentLevel++;
                    levelTime = 0.0f;
                    player.position.x = 0.0f;
                    obstacles.clear();
                    PLAYER_SPEED = 7.0f;
                }
            }
        }
        else if (currentState == GAME_OVER) {
            drawText("Game Over!", -2.0f, 1.0f, 0.003f, glm::vec3(1.0f, 0.0f, 0.0f));
            drawText("Click to return to menu", -3.5f, -1.0f, 0.002f, glm::vec3(1.0f, 1.0f, 1.0f));
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                currentState = MENU;
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
