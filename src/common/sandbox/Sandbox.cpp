#include "Sandbox.h"

namespace test {
    Sandbox::Sandbox(std::unique_ptr<TestBase> &&test)
        : mTest(std::move(test)),
          mWindow(nullptr) {
    }

    Sandbox::~Sandbox() = default;

    void Sandbox::run() {
        init();
        mainLoop();
        cleanUp();
    }

    void Sandbox::init() {
        glfwInit();

        // Because GLFW was originally designed to create an OpenGL context,
        // we need to tell it to not create an OpenGL context with a subsequent call:
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // Because handling resized windows takes special care that we'll look into later, disable it for now
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        // The first three parameters specify the width, height and title of the mWindow.
        // The fourth parameter allows you to optionally specify a monitor to open the mWindow on
        // and the last parameter is only relevant to OpenGL.
        mWindow = glfwCreateWindow(mWidth, mHeight, "vklite-test", nullptr, nullptr);
        // 将 this 指针保存到window对象中， 这样可以在callback中取出， 这里使用 lambda， 可以不需要
        glfwSetWindowUserPointer(mWindow, mTest.get());
        glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow *window, int width, int height) {
            TestBase *test = reinterpret_cast<TestBase *>(glfwGetWindowUserPointer(window));
            printf("glfwSetFramebufferSizeCallback: width:%d, height:%d", width, height);
            test->onWindowResized(width, height);
        });

        mTest->init(mWindow, mWidth, mHeight);
    }

    void Sandbox::mainLoop() {
        while (!glfwWindowShouldClose(mWindow)) {
            glfwPollEvents();
            mTest->drawFrame();
        }
    }

    void Sandbox::cleanUp() {
        mTest->cleanup();
    }
}
