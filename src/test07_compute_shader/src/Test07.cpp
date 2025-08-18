//
// Created by leixing on 2025-07-12.
//

#include "Test07.h"

#include <numbers>
#include <cmath>
#include <thread>
#include <chrono>
#include <random>

#include "util/FileUtil.h"

namespace test {
    Test07::Test07() {
        LOG_D("Test07::Test07()");
    }

    Test07::~Test07() = default;

    void Test07::init(GLFWwindow* window, int32_t width, int32_t height) {
        std::vector<uint32_t> computeShaderCode = util::FileUtil::loadSpvFile("shader/07_compute.comp.spv");
        std::vector<uint32_t> vertexShaderCode = util::FileUtil::loadSpvFile("shader/07_compute.vert.spv");
        std::vector<uint32_t> fragmentShaderCode = util::FileUtil::loadSpvFile("shader/07_compute.frag.spv");

        // Initialize particles
        std::default_random_engine rndEngine((unsigned) time(nullptr));
        std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);

        // Initial particle positions on a circle
        std::vector<Particle> particles(mParticleCount);
        for (auto& particle: particles) {
            float r = 0.25f * sqrt(rndDist(rndEngine));
            float theta = rndDist(rndEngine) * 2.0f * 3.14159265358979323846f;
            float x = r * cos(theta) * height / width;
            float y = r * sin(theta);
            particle.position = glm::vec2(x, y);
            particle.velocity = glm::normalize(glm::vec2(x, y)) * 0.00025f;
            particle.color = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), 1.0f);
        }

        vk::DeviceSize shaderStorageBufferSize = sizeof(Particle) * particles.size();


        vklite::ShaderConfigure graphicShaderConfigure = vklite::ShaderConfigure()
                .vertexShaderCode(std::move(vertexShaderCode))
                .fragmentShaderCode(std::move(fragmentShaderCode))
                .addVertexBinding([&](vklite::VertexBindingConfigure& vertexBindingConfigure) {
                    vertexBindingConfigure
                            .binding(0)
                            .stride(sizeof(Particle))
                            .addAttribute(0, ShaderFormat::Vec2, offsetof(Particle, position))
                            .addAttribute(1, ShaderFormat::Vec4, offsetof(Particle, color));
                });

        vklite::ShaderConfigure computeShaderConfigure = vklite::ShaderConfigure()
                .computeShaderCode(std::move(computeShaderCode))
                .addDescriptorSetConfigure([&](vklite::DescriptorSetConfigure& descriptorSetConfigure) {
                    descriptorSetConfigure
                            .set(0)
                            .addUniformBuffer(0, 1, vk::ShaderStageFlagBits::eCompute)
                            .addStorageBuffer(1, 1, vk::ShaderStageFlagBits::eCompute)
                            .addStorageBuffer(2, 1, vk::ShaderStageFlagBits::eCompute);
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
        uint32_t computeAndGraphicQueueFamilyIndex = (*mPhysicalDevice).queryQueueFamilyIndicesByFlags(vk::QueueFlagBits::eGraphics)[0];

        mDevice = vklite::DeviceBuilder()
                .physicalDevice((*mPhysicalDevice).getVkPhysicalDevice())
                .addQueueFamily(computeAndGraphicQueueFamilyIndex)
                .addQueueFamily(presentQueueFamilyIndex)
                .addPlugin(vklite::SurfacePlugin::buildUnique())
                .addPlugin(vklite::WindowsGlfwSurfacePlugin::buildUnique())
                .addPlugin(vklite::ValidationPlugin::buildUnique())
                .buildUnique();
        LOG_D("device => %p", (void *) (*mDevice).getVkDevice());

        mGraphicQueue = std::make_unique<vklite::Queue>((*mDevice).getQueue(computeAndGraphicQueueFamilyIndex));
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
                .queueFamilyIndex(computeAndGraphicQueueFamilyIndex)
                .buildUnique();
        mGraphicCommandBuffers = (*mCommandPool).allocateUnique(mFrameCount);

        // 创建附件
        mDisplayImageViews = (*mSwapchain).createDisplayImageViews();

        if (mMsaaEnable) {
            mColorImageView = vklite::CombinedImageViewBuilder().asColorAttachment()
                    .device((*mDevice).getVkDevice())
                    .format((*mSwapchain).getVkFormat())
                    .size((*mSwapchain).getDisplaySize())
                    .sampleCount(mSampleCount)
                    .physicalDeviceMemoryProperties((*mPhysicalDevice).getMemoryProperties())
                    .buildUnique();
        }

        if (mDepthTestEnable) {
            mDepthImageView = vklite::CombinedImageViewBuilder().asDepthAttachment()
                    .device((*mDevice).getVkDevice())
                    .format((*mPhysicalDevice).findDepthFormat())
                    .size((*mSwapchain).getDisplaySize())
                    .sampleCount(mSampleCount)
                    .physicalDeviceMemoryProperties((*mPhysicalDevice).getMemoryProperties())
                    .buildUnique();
        }

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
                .addAttachmentIf(mMsaaEnable, [&](vklite::Attachment& attachment, std::vector<vklite::Subpass>& subpasses) {
                    attachment.msaaColorAttachment()
                            .sampleCount(mSampleCount)
                            .format((*mSwapchain).getVkFormat())
                            .clearColorValue(mClearColor)
                            .asColorAttachmentUsedIn(subpasses[0], vk::ImageLayout::eColorAttachmentOptimal);
                })
                .addAttachment([&](vklite::Attachment& attachment, std::vector<vklite::Subpass>& subpasses) {
                    attachment.presentColorAttachment()
                            .format((*mSwapchain).getVkFormat())
                            .clearColorValue(mClearColor)
                            .applyIf(mMsaaEnable, [&](vklite::Attachment& thiz) {
                                thiz
                                        .loadOp(vk::AttachmentLoadOp::eDontCare)
                                        .asResolveAttachmentUsedIn(subpasses[0], vk::ImageLayout::eColorAttachmentOptimal);
                            })
                            .applyIf(!mMsaaEnable, [&](vklite::Attachment& thiz) {
                                thiz.asColorAttachmentUsedIn(subpasses[0], vk::ImageLayout::eColorAttachmentOptimal);
                            });
                })
                .addAttachmentIf(mDepthTestEnable, [&](vklite::Attachment& attachment, std::vector<vklite::Subpass>& subpasses) {
                    attachment.depthStencilAttachment()
                            .sampleCount(mSampleCount)
                            .clearDepthValue(mClearDepth)
                            .format((*mPhysicalDevice).findDepthFormat())
                            .asDepthStencilAttachmentUsedIn(subpasses[0], vk::ImageLayout::eDepthStencilAttachmentOptimal);
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
                            // 下面添加附件的顺序不能乱, 附件的顺序由 RenderPass 的附件定义顺序决定，必须严格一致。
                            .addAttachmentIf(mMsaaEnable, [&]() { return mColorImageView->getVkImageView(); })
                            .addAttachment(mDisplayImageViews[index].getVkImageView())
                            .addAttachmentIf(mDepthTestEnable, [&]() { return mDepthImageView->getVkImageView(); })
                            .build();
                })
                .build();

        mGraphicImageAvailableSemaphores = vklite::SemaphoreBuilder().device((*mDevice).getVkDevice()).build(mFrameCount);
        mGraphicRenderFinishedSemaphores = vklite::SemaphoreBuilder().device((*mDevice).getVkDevice()).build(mFrameCount);
        mGraphicFences = vklite::FenceBuilder()
                .device((*mDevice).getVkDevice())
                // 已发出信号的状态下创建栅栏，以便第一次调用 vkWaitForFences()立即返回
                .fenceCreateFlags(vk::FenceCreateFlagBits::eSignaled)
                .build(mFrameCount);

        mDescriptorPool = vklite::DescriptorPoolBuilder()
                .device((*mDevice).getVkDevice())
                .frameCount(mFrameCount)
                .addDescriptorPoolSizes(graphicShaderConfigure.calcDescriptorPoolSizes())
                .addDescriptorSetCount(graphicShaderConfigure.getDescriptorSetCount())
                .addDescriptorPoolSizes(computeShaderConfigure.calcDescriptorPoolSizes())
                .addDescriptorSetCount(computeShaderConfigure.getDescriptorSetCount())
                .buildUnique();

        mGraphicPipeline = vklite::CombinedGraphicPipelineBuilder()
                .device((*mDevice).getVkDevice())
                .frameCount(mFrameCount)
                .renderPass((*mRenderPass).getVkRenderPass())
                .shaderConfigure(graphicShaderConfigure)
                .topology(vk::PrimitiveTopology::ePointList)
                .viewports(mViewports)
                .scissors(mScissors)
                .sampleCount(mSampleCount)
                .depthTestEnable(mDepthTestEnable)
                .buildUnique();


        mComputeQueue = std::make_unique<vklite::Queue>((*mDevice).getQueue(computeAndGraphicQueueFamilyIndex));
        mComputeCommandBuffers = mCommandPool->allocateUnique(mFrameCount);

        mComputePipeline = vklite::CombinedComputePipelineBuilder()
                .device((*mDevice).getVkDevice())
                .frameCount(mFrameCount)
                .descriptorPool((*mDescriptorPool).getVkDescriptorPool())
                .shaderConfigure(computeShaderConfigure)
                .buildUnique();

        mComputeFences = vklite::FenceBuilder()
                .device((*mDevice).getVkDevice())
                // 已发出信号的状态下创建栅栏，以便第一次调用 vkWaitForFences()立即返回
                .fenceCreateFlags(vk::FenceCreateFlagBits::eSignaled)
                .build(mFrameCount);

        mComputeFinishSemaphores = vklite::SemaphoreBuilder()
                .device((*mDevice).getVkDevice())
                .build(mFrameCount);


        for (uint32_t i = 0; i < mFrameCount; i++) {
            vklite::StorageBuffer storageBuffer = vklite::StorageBufferBuilder()
                    .device((*mDevice).getVkDevice())
                    .physicalDeviceMemoryProperties((*mPhysicalDevice).getMemoryProperties())
                    .size(shaderStorageBufferSize)
                    .addUsage(vk::BufferUsageFlagBits::eVertexBuffer)
                    .build();
            storageBuffer.update(*mCommandPool, particles.data(), shaderStorageBufferSize);
            mShaderStorageBuffers.push_back(std::move(storageBuffer));

            vklite::UniformBuffer uniformBuffer = vklite::UniformBufferBuilder()
                    .device((*mDevice).getVkDevice())
                    .physicalDeviceMemoryProperties((*mPhysicalDevice).getMemoryProperties())
                    .size(sizeof(UniformBufferObject))
                    .build();

            mUniformBuffers.push_back(std::move(uniformBuffer));
        }

        for (uint32_t frameIndex = 0; frameIndex < mFrameCount; frameIndex++) {
            mVertexVkBuffers.push_back(mShaderStorageBuffers[frameIndex].getVkBuffer());
            mVertexBufferOffsets.push_back(0);
        }

        vklite::DescriptorSetWriters descriptorSetWriters = vklite::DescriptorSetWritersBuilder()
                .frameCount(mFrameCount)
                .descriptorSetMappingConfigure([&](uint32_t frameIndex, vklite::DescriptorSetMappingConfigure& configure) {
                    configure
                            .descriptorSet(mComputePipeline->getDescriptorSet(frameIndex, 0))
                            .addMapping([&](vklite::DescriptorMapping& mapping) {
                                mapping
                                        .binding(0)
                                        .descriptorType(vk::DescriptorType::eUniformBuffer)
                                        .addBufferInfo(mUniformBuffers[frameIndex].getCombinedMemoryBuffer().getBuffer());
                            })
                            .addMapping([&](vklite::DescriptorMapping& mapping) {
                                mapping
                                        .binding(1)
                                        .descriptorType(vk::DescriptorType::eStorageBuffer)
                                        .addBufferInfo(mShaderStorageBuffers[(frameIndex - 1) % mFrameCount].getVkBuffer(), 0, (vk::DeviceSize) shaderStorageBufferSize);
                            })
                            .addMapping([&](vklite::DescriptorMapping& mapping) {
                                mapping
                                        .binding(2)
                                        .descriptorType(vk::DescriptorType::eStorageBuffer)
                                        .addBufferInfo(mShaderStorageBuffers[frameIndex].getVkBuffer(), 0, (vk::DeviceSize) shaderStorageBufferSize);
                            });
                })
                .build();

        std::vector<vk::WriteDescriptorSet> writeDescriptorSets = descriptorSetWriters.createWriteDescriptorSets();
        mDevice->getVkDevice().updateDescriptorSets(writeDescriptorSets, nullptr);

        mTimer.start();
    }

    void Test07::drawFrame() {
        UniformBufferObject ubo{};
        ubo.deltaTime = mTimer.getDeltaTimeMs() * 2.0f;
        // LOG_D("ubo.deltaTime: %f", ubo.deltaTime);
        mUniformBuffers[mCurrentFrameIndex].update(*mCommandPool, &ubo, sizeof(UniformBufferObject));


        // compute pipeline
        vk::Result result = mComputeFences[mCurrentFrameIndex].wait();
        if (result != vk::Result::eSuccess) {
            LOG_E("waitForFences failed");
            throw std::runtime_error("waitForFences failed");
        }

        result = mComputeFences[mCurrentFrameIndex].reset();
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("mComputeFences[mCurrentFrameIndex] resetFences failed");
        }

        const vklite::PooledCommandBuffer& computeCommandBuffer = (*mComputeCommandBuffers)[mCurrentFrameIndex];
        computeCommandBuffer.record([&](const vk::CommandBuffer& commandBuffer) {
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, mComputePipeline->getVkPipeline());
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mComputePipeline->getVkPipelineLayout(), 0, mComputePipeline->getDescriptorSets(mCurrentFrameIndex), nullptr);

            commandBuffer.dispatch(mParticleCount / 256, 1, 1);
        });

        mComputeQueue->submit(computeCommandBuffer.getVkCommandBuffer(),
                              mComputeFinishSemaphores[mCurrentFrameIndex].getVkSemaphore(),
                              mComputeFences[mCurrentFrameIndex].getVkFence());

        result = mComputeFences[mCurrentFrameIndex].wait();
        if (result != vk::Result::eSuccess) {
            LOG_E("waitForFences failed");
            throw std::runtime_error("waitForFences failed");
        }


        vklite::Semaphore& imageAvailableSemaphore = mGraphicImageAvailableSemaphores[mCurrentFrameIndex];
        vklite::Semaphore& renderFinishedSemaphore = mGraphicRenderFinishedSemaphores[mCurrentFrameIndex];
        vklite::Fence& fence = mGraphicFences[mCurrentFrameIndex];

        result = mGraphicFences[mCurrentFrameIndex].wait();
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

        const vklite::PooledCommandBuffer& commandBuffer = (*mGraphicCommandBuffers)[mCurrentFrameIndex];
        commandBuffer.record([&](const vk::CommandBuffer& vkCommandBuffer_) {
            mRenderPass->execute(vkCommandBuffer_, mFramebuffers[imageIndex].getVkFramebuffer(), [&](const vk::CommandBuffer& vkCommandBuffer) {
                vkCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mGraphicPipeline->getVkPipeline());
                vkCommandBuffer.setViewport(0, mViewports);
                vkCommandBuffer.setScissor(0, mScissors);

                if (!mGraphicPipeline->getDescriptorSets().empty()) {
                    vkCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mGraphicPipeline->getVkPipelineLayout(), 0, mGraphicPipeline->getDescriptorSets(mCurrentFrameIndex), nullptr);
                }

                for (const vklite::PushConstant& pushConstant: mGraphicPipeline->getPushConstants()) {
                    vkCommandBuffer.pushConstants(mGraphicPipeline->getVkPipelineLayout(),
                                                  pushConstant.getStageFlags(),
                                                  pushConstant.getOffset(),
                                                  pushConstant.getSize(),
                                                  pushConstant.getData());
                }

                vkCommandBuffer.bindVertexBuffers(0, mVertexVkBuffers[mCurrentFrameIndex], mVertexBufferOffsets[mCurrentFrameIndex]);
                vkCommandBuffer.draw(mParticleCount, 1, 0, 0);
            });
        });

        result = mGraphicFences[mCurrentFrameIndex].reset();
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


        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    void Test07::cleanup() {
        vk::Device device = (*mDevice).getVkDevice();
        if (device != nullptr) {
            device.waitIdle();
        }
    }

    void Test07::onWindowResized(int32_t width, int32_t height) {
    }
} // test01
