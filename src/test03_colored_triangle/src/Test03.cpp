//
// Created by leixing on 2025-07-12.
//

#include "Test03.h"
#include "util/FileUtil.h"

#include <numbers>
#include <cmath>

namespace test {
    Test03::Test03() {
        LOG_D("Test03::Test03()");
    }

    Test03::~Test03() = default;

    void Test03::init(GLFWwindow* window, int32_t width, int32_t height) {
        std::vector<uint32_t> vertexShaderCode = util::FileUtil::loadSpvFile("shader/03_colored_triangle.vert.spv");
        std::vector<uint32_t> fragmentShaderCode = util::FileUtil::loadSpvFile("shader/03_colored_triangle.frag.spv");

        vklite::ShaderConfigure shaderConfigure = vklite::ShaderConfigure()
                .vertexShaderCode(std::move(vertexShaderCode))
                .fragmentShaderCode(std::move(fragmentShaderCode))
                .addVertexBinding([&](vklite::VertexBindingConfigure& vertexBindingConfigure) {
                    vertexBindingConfigure
                            .binding(0)
                            .stride(sizeof(Vertex))
                            .addAttribute(0, ShaderFormat::Vec3)
                            .addAttribute(1, ShaderFormat::Vec3);
                });


        mInstance = vklite::InstanceBuilder()
                .addPlugin(vklite::SurfacePlugin::buildUnique())
                .addPlugin(vklite::WindowsGlfwSurfacePlugin::buildUnique())
                .addPlugin(vklite::ValidationPlugin::buildUnique())
                .buildUnique();

        mSurface = vklite::WindowsGLFWSurfaceBuilder()
                .instance((*mInstance).getVkInstance())
                .window(window)
                .buildUnique();

        mPhysicalDevice = vklite::PhysicalDeviceSelector::makeDefault(*mSurface, vk::QueueFlagBits::eGraphics)
                .selectUnique((*mInstance).enumeratePhysicalDevices());

        uint32_t presentQueueFamilyIndex = (*mPhysicalDevice).queryQueueFamilyIndicesBySurface((*mSurface).getVkSurface())[0];
        uint32_t graphicQueueFamilyIndex = (*mPhysicalDevice).queryQueueFamilyIndicesByFlags(vk::QueueFlagBits::eGraphics)[0];

        mDevice = vklite::DeviceBuilder()
                .physicalDevice((*mPhysicalDevice).getVkPhysicalDevice())
                .addQueueFamily(graphicQueueFamilyIndex)
                .addQueueFamily(presentQueueFamilyIndex)
                .addPlugin(vklite::SurfacePlugin::buildUnique())
                .addPlugin(vklite::WindowsGlfwSurfacePlugin::buildUnique())
                .addPlugin(vklite::ValidationPlugin::buildUnique())
                .buildUnique();
        LOG_D("device => %p", (void *) (*mDevice).getVkDevice());

        mGraphicQueue = std::make_unique<vklite::Queue>((*mDevice).getQueue(graphicQueueFamilyIndex));
        mPresentQueue = std::make_unique<vklite::Queue>((*mDevice).getQueue(presentQueueFamilyIndex));

        mSwapchain = vklite::SwapchainBuilder()
                .device((*mDevice).getVkDevice())
                .surface((*mSurface).getVkSurface())
                .queueFamilyIndices({presentQueueFamilyIndex})
                .config((*mPhysicalDevice).getVkPhysicalDevice(), (*mSurface).getVkSurface())
                .buildUnique();

        mViewports = (*mSwapchain).fullScreenViewports();
        mScissors = (*mSwapchain).fullScreenScissors();
        // mViewports = (*mSwapchain).centerSquareViewports();
        // mScissors = (*mSwapchain).centerSquareScissors();

        mCommandPool = vklite::CommandPoolBuilder()
                .device((*mDevice).getVkDevice())
                .queueFamilyIndex(graphicQueueFamilyIndex)
                .buildUnique();
        mCommandBuffers = (*mCommandPool).allocateUnique(mFrameCount);

        // 创建附件
        mDisplayImageViews = (*mSwapchain).createDisplayImageViews();

        vklite::Subpass externalSubpass = vklite::Subpass::externalSubpass();
        mRenderPass = vklite::RenderPassBuilder()
                .device((*mDevice).getVkDevice())
                .renderAreaExtend((*mSwapchain).getDisplaySize())
                .addSubpass([&](vklite::Subpass& subpass, [[maybe_unused]] const std::vector<vklite::Subpass>& subpasses) {
                    subpass
                            .pipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                            .addDependency(externalSubpass,
                                           vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
                                           vk::AccessFlagBits::eNone,
                                           vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
                                           vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
                })
                .addAttachment([&](vklite::Attachment& attachment, std::vector<vklite::Subpass>& subpasses) {
                    attachment.presentColorAttachment()
                            .format((*mSwapchain).getVkFormat())
                            .clearColorValue(mClearColor)
                            .asColorAttachmentUsedIn(subpasses[0], vk::ImageLayout::eColorAttachmentOptimal);
                })
                .buildUnique();

        mFramebuffers = vklite::FramebuffersBuilder()
                .count(static_cast<uint32_t>(mDisplayImageViews.size()))
                .framebufferBuilder([&](uint32_t index) {
                    return vklite::FramebufferBuilder()
                            .device((*mDevice).getVkDevice())
                            .renderPass(mRenderPass->getVkRenderPass())
                            .width((*mSwapchain).getDisplaySize().width)
                            .height((*mSwapchain).getDisplaySize().height)
                            .addAttachment(mDisplayImageViews[index].getVkImageView())
                            .build();
                })
                .build();

        mImageAvailableSemaphores = vklite::SemaphoreBuilder().device((*mDevice).getVkDevice()).build(mFrameCount);
        mRenderFinishedSemaphores = vklite::SemaphoreBuilder().device((*mDevice).getVkDevice()).build(mFrameCount);
        mFences = vklite::FenceBuilder()
                .device((*mDevice).getVkDevice())
                // 已发出信号的状态下创建栅栏，以便第一次调用 vkWaitForFences()立即返回
                .fenceCreateFlags(vk::FenceCreateFlagBits::eSignaled)
                .build(mFrameCount);

        std::vector<vk::PushConstantRange> pushConstantRanges = shaderConfigure.getPushConstantRanges();
        mPushConstants.reserve(pushConstantRanges.size());
        for (const vk::PushConstantRange& pushConstantRange: pushConstantRanges) {
            mPushConstants.emplace_back(pushConstantRange.size, pushConstantRange.offset, pushConstantRange.stageFlags);
        }

        mDescriptorPool = vklite::DescriptorPoolBuilder()
                .device((*mDevice).getVkDevice())
                .frameCount(mFrameCount)
                .descriptorPoolSizes(shaderConfigure.calcDescriptorPoolSizes())
                .descriptorSetCount(shaderConfigure.getDescriptorSetCount())
                .buildUnique();

        mDescriptorSetLayouts = vklite::DescriptorSetLayoutsBuilder()
                .device((*mDevice).getVkDevice())
                .bindings(shaderConfigure.createDescriptorSetLayoutBindings())
                .buildUnique();

        mDescriptorSets.reserve(mFrameCount);
        for (uint32_t i = 0; i < mFrameCount; i++) {
            std::vector<vk::DescriptorSet> sets = (*mDescriptorPool).allocateDescriptorSets((*mDescriptorSetLayouts).getDescriptorSetLayouts());
            LOG_D("descriptorPool->allocateDescriptorSets:");
            for (const vk::DescriptorSet& set: sets) {
                LOG_D("\tset:%p", (void *) set);
            }
            mDescriptorSets.push_back(std::move(sets));
        }

        mPipelineLayout = vklite::PipelineLayoutBuilder()
                .device((*mDevice).getVkDevice())
                .descriptorSetLayouts((*mDescriptorSetLayouts).getDescriptorSetLayouts())
                .pushConstantRanges(std::move(pushConstantRanges))
                .buildUnique();

        std::unique_ptr<vklite::ShaderModule> vertexShader = vklite::ShaderModuleBuilder()
                .device((*mDevice).getVkDevice())
                .code(std::move(shaderConfigure.getVertexShaderCode()))
                .buildUnique();

        std::unique_ptr<vklite::ShaderModule> fragmentShader = vklite::ShaderModuleBuilder()
                .device((*mDevice).getVkDevice())
                .code(std::move(shaderConfigure.getFragmentShaderCode()))
                .buildUnique();

        mPipeline = vklite::GraphicsPipelineBuilder()
                .device((*mDevice).getVkDevice())
                .renderPass((*mRenderPass).getVkRenderPass())
                .pipelineLayout((*mPipelineLayout).getVkPipelineLayout())
                .viewports(mViewports)
                .scissors(mScissors)
                .vertexShader(std::move(vertexShader))
                .vertexBindingDescriptions(shaderConfigure.createVertexBindingDescriptions())
                .vertexAttributeDescriptions(shaderConfigure.createVertexAttributeDescriptions())
                .fragmentShader(std::move(fragmentShader))
                .sampleCount(mSampleCount)
                .depthTestEnable(false)
                .buildUnique();



        // 六边形
        double r = 0.8;
        const int numVertices = 6;
        std::vector<Vertex> vertices;

        // 中心点
        vertices.push_back({
            {0.0f, 0.0f, 0.0f}, // 坐标
            {1.0f, 1.0f, 1.0f} // 颜色
        });
        std::vector<glm::vec3> colors = {
            {1.0f, 0.0f, 0.0f},
            {1.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 1.0f, 1.0f},
            {0.0f, 0.0f, 1.0f},
            {1.0f, 0.0f, 1.0f},
        };
        for (int i = 0; i < numVertices; ++i) {
            // 1. 计算角度（度 → 弧度）
            double angle_deg = i * 60.0; // 逆时针旋转60° × i
            double angle_rad = angle_deg * std::numbers::pi / 180.0; // 转为弧度

            // 2. 计算坐标
            double x = r * std::cos(angle_rad);
            double y = r * std::sin(angle_rad);

            // 4. 添加到顶点列表
            vertices.push_back({
                {static_cast<float>(x), static_cast<float>(y), 0.0f},
                colors[i]
            });
        }

        std::vector<uint32_t> indices = {
            0, 2, 1,
            0, 3, 2,
            0, 4, 3,
            0, 5, 4,
            0, 6, 5,
            0, 1, 6,
        };
        uint32_t indicesSize = indices.size() * sizeof(uint32_t);

        // triangle
        // std::vector<Vertex> vertices = {
        //     {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        //     {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        //     {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        // };
        //
        // std::vector<uint32_t> indices = {0, 1, 2};


        mIndexBuffer = vklite::IndexBufferBuilder()
                .device((*mDevice).getVkDevice())
                .physicalDeviceMemoryProperties((*mPhysicalDevice).getMemoryProperties())
                .size(indicesSize)
                .buildUnique();
        (*mIndexBuffer).update(*mCommandPool, indices);
        mIndexVkBuffer = (*mIndexBuffer).getVkBuffer();
        mIndexBufferOffset = 0;
        mIndexCount = indices.size();

        uint32_t verticesSize = vertices.size() * sizeof(Vertex);
        mVertexBuffer = vklite::VertexBufferBuilder()
                .device(mDevice->getVkDevice())
                .physicalDeviceMemoryProperties(mPhysicalDevice->getMemoryProperties())
                .size(verticesSize)
                .buildUnique();
        (*mVertexBuffer).update(*mCommandPool, vertices.data(), verticesSize);
        mVertexBuffers.push_back((*mVertexBuffer).getVkBuffer());
        mVertexBufferOffsets.push_back(0);

    }

    void Test03::drawFrame() {
        vklite::Semaphore& imageAvailableSemaphore = mImageAvailableSemaphores[mCurrentFrameIndex];
        vklite::Semaphore& renderFinishedSemaphore = mRenderFinishedSemaphores[mCurrentFrameIndex];
        vklite::Fence& fence = mFences[mCurrentFrameIndex];
        const vklite::PooledCommandBuffer& commandBuffer = (*mCommandBuffers)[mCurrentFrameIndex];

        vk::Result result = mFences[mCurrentFrameIndex].wait();
        if (result != vk::Result::eSuccess) {
            LOG_E("waitForFences failed");
            throw std::runtime_error("waitForFences failed");
        }

        // 当 acquireNextImageKHR 成功返回时，imageAvailableSemaphore 会被触发，表示图像已经准备好，可以用于渲染。
        auto [acquireResult, imageIndex] = mSwapchain->acquireNextImage(imageAvailableSemaphore.getVkSemaphore());
        if (acquireResult != vk::Result::eSuccess) {
            if (acquireResult == vk::Result::eErrorOutOfDateKHR) {
                // 交换链已与表面不兼容，不能再用于渲染。通常在窗口大小调整后发生。
                LOG_E("acquireNextImageKHR: eErrorOutOfDateKHR, recreateSwapChain");
                //                recreateSwapChain();
                return;
            } else if (acquireResult == vk::Result::eSuboptimalKHR) {
                //vk::Result::eSuboptimalKHR 交换链仍然可以成功显示到表面，但表面属性不再完全匹配。
                LOG_D("acquireNextImageKHR: eSuboptimalKHR");
            } else {
                LOG_E("acquireNextImageKHR: failed: %d", acquireResult);
                throw std::runtime_error("acquireNextImageKHR failed");
            }
        }


        commandBuffer.record([&](const vk::CommandBuffer& vkCommandBuffer_) {
            mRenderPass->execute(vkCommandBuffer_, mFramebuffers[imageIndex].getVkFramebuffer(), [&](const vk::CommandBuffer& vkCommandBuffer) {
                vkCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline->getVkPipeline());
                vkCommandBuffer.setViewport(0, mViewports);
                vkCommandBuffer.setScissor(0, mScissors);

                if (!mDescriptorSets[mCurrentFrameIndex].empty()) {
                    vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipelineLayout->getVkPipelineLayout(), 0, mDescriptorSets[mCurrentFrameIndex], nullptr);
                }

                for (const vklite::PushConstant& pushConstant: mPushConstants) {
                    vkCommandBuffer.pushConstants(mPipelineLayout->getVkPipelineLayout(),
                                                  pushConstant.getStageFlags(),
                                                  pushConstant.getOffset(),
                                                  pushConstant.getSize(),
                                                  pushConstant.getData());
                }

                vkCommandBuffer.bindVertexBuffers(0, mVertexBuffers, mVertexBufferOffsets);
                vkCommandBuffer.bindIndexBuffer(mIndexVkBuffer, mIndexBufferOffset, vk::IndexType::eUint32);
                vkCommandBuffer.drawIndexed(mIndexCount, 1, 0, 0, 0);
            });
        });

        result = mFences[mCurrentFrameIndex].reset();
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("resetFences failed");
        }

        mGraphicQueue->submit(commandBuffer.getVkCommandBuffer(),
                              vk::PipelineStageFlagBits::eColorAttachmentOutput,
                              imageAvailableSemaphore.getVkSemaphore(),
                              renderFinishedSemaphore.getVkSemaphore(),
                              fence.getVkFence());

        result = mPresentQueue->present(mSwapchain->getVkSwapChain(), imageIndex, renderFinishedSemaphore.getVkSemaphore());
        if (result != vk::Result::eSuccess) {
            if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || mFramebufferResized) {
                mFramebufferResized = false;
                LOG_E("presentKHR: eErrorOutOfDateKHR or eSuboptimalKHR or mFramebufferResized, recreateSwapChain");
                // todo: recreateSwapChain
                //                recreateSwapChain();
                return;
            } else {
                throw std::runtime_error("presentKHR failed");
            }
        }

        mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mFrameCount;
    }

    void Test03::cleanup() {
        vk::Device device = (*mDevice).getVkDevice();
        if (device != nullptr) {
            device.waitIdle();
        }
    }

    void Test03::onWindowResized(int32_t width, int32_t height) {
    }
} // test01
