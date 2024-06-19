//
// Created by amemiya7 on 2024/6/18.
//

#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Render.h"
#include "Block.h"
#include "Grid.h"

class Game {
public:
    Game(unsigned int windowWidth, unsigned int windowHeight,int gameWidth, int gameHeight) {
        exists.resize((Width + 2) * Height);
        colors.resize((Width + 2)* Height);
        render.resize(windowWidth, windowHeight);
        grid.resize(gameWidth, gameHeight);
    }
    const unsigned int Height = 10, Width = 20;
    const unsigned int NextWidth = 3, NextHeight = 6, NextCount = 1;
    Render& getRender() {
        return render;
    }
private:
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
    }
    Render render{key_callback};
    Block block;
    Grid grid;
    //map two dimenstions to one dimension
    int index(int x, int y) {
        return x + y * Width;
    }
    std::vector<int> exists;
    std::vector<glm::vec3> colors;

};
