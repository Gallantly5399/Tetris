//
// Created by amemiya7 on 2024/6/18.
//

#include "Game.h"

Game::BlockMovement Game::blockMovement = BlockMovement::down;
void Game::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        blockMovement = BlockMovement::left;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        blockMovement = BlockMovement::right;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        blockMovement = BlockMovement::down;
    }
    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        blockMovement = BlockMovement::rotate;
    }
}