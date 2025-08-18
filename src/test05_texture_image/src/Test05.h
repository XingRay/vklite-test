//
// Created by leixing on 2025-07-12.
//

#pragma once

#include <memory>
#include <vector>
#include <cstdint>

#include "sandbox/TestBase.h"
#include "math/glm.h"
#include "util/Timer.h"

#include "vklite/vklite.h"
#include "vklite/vklite_windows.h"
#include "vklite/Log.h"


namespace test {
    struct Vertex {
        glm::vec3 position;
        glm::vec2 uv;
    };

    struct MvpMatrix {
        glm::mat4 model; // 模型矩阵
        glm::mat4 view; // 视图矩阵
        glm::mat4 proj; // 投影矩阵
    };

    class Test05 : public TestBase {
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
        std::unique_ptr<vklite::DescriptorPool> mDescriptorPool;
        std::unique_ptr<vklite::DescriptorSetLayouts> mDescriptorSetLayouts;
        std::vector<std::vector<vk::DescriptorSet> > mDescriptorSets;
        std::vector<vklite::PushConstant> mPushConstants;
        std::unique_ptr<vklite::Pipeline> mPipeline;

        // vertex buffer
        std::vector<vk::Buffer> mVertexBuffers;
        std::vector<vk::DeviceSize> mVertexBufferOffsets;

        // index buffer
        vk::Buffer mIndexVkBuffer;
        uint32_t mIndexBufferOffset = 0;
        uint32_t mIndexCount = 0;

        // resource
        std::unique_ptr<vklite::VertexBuffer> mVertexBuffer;
        std::unique_ptr<vklite::IndexBuffer> mIndexBuffer;

        MvpMatrix mMvpMatrix{};
        util::Timer mTimer;

        std::vector<vklite::CombinedImageSampler> mSamplers;

    public:
        Test05();

        ~Test05() override;

        void init(GLFWwindow* window, int32_t width, int32_t height) override;

        void drawFrame() override;

        void cleanup() override;

        void onWindowResized(int width, int height) override;
    };
} // test01
