//
// Created by leixing on 2025-07-12.
//

#include "Test06.h"

#include <numbers>
#include <cmath>
#include <thread>
#include <chrono>

#include "util/FileUtil.h"
#include "image/ImageInterface.h"
#include "model/Model.h"
#include "model/ModelLoader.h"
#include "image/StbImage.h"
#include "model/Vertex.h"

namespace test {
    Test06::Test06() {
        LOG_D("Test06::Test06()");
    }

    Test06::~Test06() = default;

    void Test06::init(GLFWwindow* window, int32_t width, int32_t height) {
        std::vector<uint32_t> vertexShaderCode = util::FileUtil::loadSpvFile("shader/06_load_3d_model.vert.spv");
        std::vector<uint32_t> fragmentShaderCode = util::FileUtil::loadSpvFile("shader/06_load_3d_model.frag.spv");

        vklite::ShaderConfigure shaderConfigure = vklite::ShaderConfigure()
                .vertexShaderCode(std::move(vertexShaderCode))
                .fragmentShaderCode(std::move(fragmentShaderCode))
                .addVertexBinding([&](vklite::VertexBindingConfigure& vertexBindingConfigure) {
                    vertexBindingConfigure
                            .binding(0)
                            .stride(sizeof(model::Vertex))
                            .addAttribute(0, ShaderFormat::Vec3)
                            .addAttribute(1, ShaderFormat::Vec2);
                })
                .addDescriptorSetConfigure([&](vklite::DescriptorSetConfigure& descriptorSetConfigure) {
                    descriptorSetConfigure
                            .set(0)
                            .addUniformBuffer(0, vk::ShaderStageFlagBits::eVertex)
                            .addCombinedImageSampler(1, vk::ShaderStageFlagBits::eFragment);
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
                .depthTestEnable(mDepthTestEnable)
                .buildUnique();


        const char* MODEL_PATH = "resource/model/viking_room/viking_room.obj";
        const char* TEXTURE_PATH = "resource/model/viking_room/viking_room.png";

        model::Model model = model::ModelLoader::load(MODEL_PATH);
        std::unique_ptr<image::StbImage> textureImage = image::StbImage::loadImageAsRgba(TEXTURE_PATH);

        mIndexBuffer = vklite::IndexBufferBuilder()
                .device((*mDevice).getVkDevice())
                .physicalDeviceMemoryProperties((*mPhysicalDevice).getMemoryProperties())
                .size(model.getIndicesBytes())
                .buildUnique();
        (*mIndexBuffer).update(*mCommandPool, model.getIndices());
        mIndexVkBuffer = (*mIndexBuffer).getVkBuffer();
        mIndexBufferOffset = 0;
        mIndexCount = model.getIndicesCount();

        uint32_t verticesSize = model.getVerticesBytes();
        mVertexBuffer = vklite::VertexBufferBuilder()
                .device(mDevice->getVkDevice())
                .physicalDeviceMemoryProperties(mPhysicalDevice->getMemoryProperties())
                .size(verticesSize)
                .buildUnique();
        (*mVertexBuffer).update(*mCommandPool, model.getVerticesData(), verticesSize);
        mVertexBuffers.push_back((*mVertexBuffer).getVkBuffer());
        mVertexBufferOffsets.push_back(0);


        mMvpMatrix = math::MvpMatrix{};
        float screenWidth = static_cast<float>(width);
        float screenHeight = static_cast<float>(height);
        float aspectRatio = screenWidth / screenHeight;

        mMvpMatrix.view(glm::lookAt(glm::vec3(0.0f, 5.0f, 5.0f),
                                    glm::vec3(0.0f, 0.0f, 0.0f),
                                    glm::vec3(0.0f, 0.0f, 1.0f)));
        mMvpMatrix.projection(glm::perspective(glm::radians(45.0f),
                                               aspectRatio,
                                               1.0f,
                                               20.0f));
        mMvpMatrix.projectionFlipY();
        mTimer.start();


        mSamplers = vklite::CombinedImageSamplerBuilder().asTexture()
                .device((*mDevice).getVkDevice())
                .physicalDeviceMemoryProperties((*mPhysicalDevice).getMemoryProperties())
                .width(textureImage->getWidth())
                .height(textureImage->getHeight())
                .format(textureImage->getVkFormat())
                .build(mFrameCount);

        for (uint32_t i = 0; i < mFrameCount; i++) {
            mSamplers[i].getImage().changeImageLayout(*mCommandPool);
            mSamplers[i].update(*mCommandPool, textureImage->getPixels(), textureImage->getPixelBytes());
        }

        mUniformBuffers = vklite::UniformBufferBuilder()
                .device(mDevice->getVkDevice())
                .physicalDeviceMemoryProperties(mPhysicalDevice->getMemoryProperties())
                .size(sizeof(math::MvpMatrix))
                .build(mFrameCount);

        for (uint32_t i = 0; i < mFrameCount; i++) {
            mUniformBuffers[i].update(*mCommandPool, &mMvpMatrix, sizeof(math::MvpMatrix));
        }

        vklite::DescriptorSetWriters descriptorSetWriters = vklite::DescriptorSetWritersBuilder()
                .frameCount(mFrameCount)
                .descriptorSetMappingConfigure([&](uint32_t frameIndex, vklite::DescriptorSetMappingConfigure& configure) {
                    configure
                            .descriptorSet(mDescriptorSets[frameIndex][0])
                            .addUniformBuffer([&](vklite::UniformBufferDescriptorMapping& mapping) {
                                mapping
                                        .binding(0)
                                        .addBufferInfo(mUniformBuffers[frameIndex].getBuffer());
                            })
                            .addCombinedImageSampler([&](vklite::CombinedImageSamplerDescriptorMapping& mapping) {
                                mapping
                                        .binding(1)
                                        .addImageInfo(mSamplers[frameIndex].getSampler(), mSamplers[frameIndex].getImageView());
                            });
                })
                .build();

        std::vector<vk::WriteDescriptorSet> writeDescriptorSets = descriptorSetWriters.createWriteDescriptorSets();
        mDevice->getVkDevice().updateDescriptorSets(writeDescriptorSets, nullptr);
    }

    void Test06::drawFrame() {
        float deltaTime = mTimer.getDeltaTimeSecond();

        mMvpMatrix.modelRotateZ(deltaTime * glm::radians(90.0f));

        glm::mat4 mvp = mMvpMatrix.calcMvp();
        mUniformBuffers[mCurrentFrameIndex].update(*mCommandPool, &mvp, sizeof(glm::mat4));

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

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    void Test06::cleanup() {
        vk::Device device = (*mDevice).getVkDevice();
        if (device != nullptr) {
            device.waitIdle();
        }
    }

    void Test06::onWindowResized(int32_t width, int32_t height) {
    }
} // test01
