//
// Created by leixing on 2025-07-12.
//

#pragma once

#include <memory>
#include <vector>
#include <cstdint>

#include "sandbox/TestBase.h"
#include "math/glm.h"

#include "vklite/vklite.h"
#include "vklite/vklite_windows.h"


namespace test {
    struct Vertex {
        glm::vec3 pos;
    };

    class Test01 : public TestBase {
    private:
        // config
        uint32_t mFrameCount = 2;
        std::array<float, 4> mClearColor = {0.2f, 0.4f, 0.8f, 1.0f};
        vk::SampleCountFlagBits mSampleCount = vk::SampleCountFlagBits::e1;

        //status
        uint32_t mCurrentFrameIndex = 0;
        bool mFramebufferResized = false;


        std::unique_ptr<vklite::Instance> mInstance;
        std::unique_ptr<vklite::Surface> mSurface;
        std::unique_ptr<vklite::PhysicalDevice> mPhysicalDevice;

        std::unique_ptr<vklite::Device> mDevice;

        std::unique_ptr<vklite::Queue> mGraphicQueue;
        std::unique_ptr<vklite::Queue> mPresentQueue;

        std::unique_ptr<vklite::Swapchain> mSwapchain;
        std::vector<vklite::ImageView> mDisplayImageViews;
        std::vector<vk::Viewport> mViewports;
        std::vector<vk::Rect2D> mScissors;

        std::unique_ptr<vklite::CommandPool> mCommandPool;
        std::unique_ptr<vklite::CommandBuffers> mCommandBuffers;

        std::unique_ptr<vklite::RenderPass> mRenderPass;
        vklite::Framebuffers mFramebuffers;

        std::vector<vklite::Semaphore> mImageAvailableSemaphores;
        std::vector<vklite::Semaphore> mRenderFinishedSemaphores;
        std::vector<vklite::Fence> mFences;

        std::unique_ptr<vklite::PipelineLayout> mPipelineLayout;
        std::unique_ptr<vklite::Pipeline> mPipeline;

        // vertex buffer
        std::vector<vk::Buffer> mVertexBuffers;
        std::vector<vk::DeviceSize> mVertexBufferOffsets;

        // index buffer
        vk::Buffer mIndexVkBuffer;
        uint32_t mIndexBufferOffset = 0;
        uint32_t mIndexCount = 0;

        // resource
        std::unique_ptr<vklite::IndexBuffer> mIndexBuffer;
        std::unique_ptr<vklite::VertexBuffer> mVertexBuffer;

    public:
        Test01();

        ~Test01() override;

        void init(GLFWwindow* window, int32_t width, int32_t height) override;

        void drawFrame() override;

        void cleanup() override;

        void onWindowResized(int width, int height) override;
    };
} // test01
