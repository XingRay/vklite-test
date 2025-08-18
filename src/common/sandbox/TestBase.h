//
// Created by leixing on 2025/1/4.
//

#pragma once

#include <GLFW/glfw3.h>

namespace test {
    class TestBase {
    public:
        explicit TestBase();

        virtual ~TestBase();

        virtual void init(GLFWwindow *window, int32_t width, int32_t height) = 0;

        virtual void drawFrame() = 0;

        virtual void cleanup() = 0;

        virtual void onWindowResized(int32_t width, int32_t height) =0;
    };
}
