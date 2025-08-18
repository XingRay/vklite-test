//
// Created by leixing on 2025-07-13.
//

#pragma once

#include <cstdint>
#include <memory>

#include <GLFW/glfw3.h>

#include "TestBase.h"


namespace test {
    class Sandbox {
    private:
        const int32_t mWidth = 1024;
        const int32_t mHeight = 1024;
        GLFWwindow *mWindow;

        std::unique_ptr<TestBase> mTest;

    public:
        explicit Sandbox(std::unique_ptr<TestBase>&& test);

        ~Sandbox();

        void run();

    private:
        void init();

        void mainLoop();

        void cleanUp();
    };
} // test
