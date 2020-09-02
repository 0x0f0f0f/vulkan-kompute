
#include "kompute/Sequence.hpp"

namespace kp {

Sequence::Sequence()
{
    SPDLOG_DEBUG("Kompute Sequence base constructor");
    this->mIsInit = false;
}

Sequence::Sequence(std::shared_ptr<vk::PhysicalDevice> physicalDevice,
                   std::shared_ptr<vk::Device> device,
                   std::shared_ptr<vk::Queue> computeQueue,
                   uint32_t queueIndex)
{
    SPDLOG_DEBUG("Kompute Sequence Constructor with existing device & queue");

    this->mPhysicalDevice = physicalDevice;
    this->mDevice = device;
    this->mComputeQueue = computeQueue;
    this->mQueueIndex = queueIndex;
    this->mIsInit = true;
}

Sequence::~Sequence()
{
    SPDLOG_DEBUG("Kompute Sequence Destructor started");

    if (!this->mDevice) {
        spdlog::error(
          "Kompute Sequence destructor reached with null Device pointer");
        return;
    }

    if (this->mFreeCommandBuffer) {
        spdlog::info("Freeing CommandBuffer");
        if (!this->mCommandBuffer) {
            spdlog::error("Kompute Sequence destructor reached with null "
                          "CommandPool pointer");
            return;
        }
        this->mDevice->freeCommandBuffers(
          *this->mCommandPool, 1, this->mCommandBuffer.get());
        SPDLOG_DEBUG("Kompute Sequence Freed CommandBuffer");
    }

    if (this->mFreeCommandPool) {
        spdlog::info("Destroying CommandPool");
        if (this->mCommandPool == nullptr) {
            spdlog::error("Kompute Sequence destructor reached with null "
                          "CommandPool pointer");
            return;
        }
        this->mDevice->destroy(*this->mCommandPool);
        SPDLOG_DEBUG("Kompute Sequence Destroyed CommandPool");
    }
}

void
Sequence::init()
{
    this->createCommandPool();
    this->createCommandBuffer();
    this->mIsInit = true;
}

bool
Sequence::begin()
{
    SPDLOG_DEBUG("Kompute sequence called BEGIN");

    if (this->isRecording()) {
        spdlog::warn("Kompute Sequence begin called when  already recording");
        return false;
    }

    if (!this->mCommandPool) {
        throw std::runtime_error("Kompute Sequence command pool is null");
    }

    if (!this->mRecording) {
        spdlog::info("Kompute Sequence command recording BEGIN");
        this->mCommandBuffer->begin(vk::CommandBufferBeginInfo());
        this->mRecording = true;
    } else {
        spdlog::warn("Kompute Sequence attempted to start command recording "
                     "but recording already started");
    }
    return true;
}

bool
Sequence::end()
{
    SPDLOG_DEBUG("Kompute Sequence calling END");

    if (!this->isRecording()) {
        spdlog::warn("Kompute Sequence end called when not recording");
        return false;
    }

    if (!this->mCommandPool) {
        throw std::runtime_error("Kompute Sequence command pool is null");
    }

    if (this->mRecording) {
        spdlog::info("Kompute Sequence command recording END");
        this->mCommandBuffer->end();
        this->mRecording = false;
    } else {
        spdlog::warn("Kompute Sequence attempted to end command recording but "
                     "recording not started");
    }
    return true;
}

bool
Sequence::eval()
{
    SPDLOG_DEBUG("Kompute sequence compute recording EVAL");

    if (this->isRecording()) {
        spdlog::warn("Kompute Sequence eval called when still recording");
        return false;
    }

    const vk::PipelineStageFlags waitStageMask =
      vk::PipelineStageFlagBits::eTransfer;
    vk::SubmitInfo submitInfo(
      0, nullptr, &waitStageMask, 1, this->mCommandBuffer.get());

    vk::Fence fence = this->mDevice->createFence(vk::FenceCreateInfo());

    SPDLOG_DEBUG(
      "Kompute sequence submitting command buffer into compute queue");

    this->mComputeQueue->submit(1, &submitInfo, fence);
    this->mDevice->waitForFences(1, &fence, VK_TRUE, UINT64_MAX);
    this->mDevice->destroy(fence);

    for (size_t i = 0; i < this->mOperations.size(); i++) {
        this->mOperations[i]->postSubmit();
    }

    SPDLOG_DEBUG("Kompute sequence EVAL success");

    return true;
}

bool
Sequence::isRecording()
{
    return this->mRecording;
}

bool
Sequence::isInit()
{
    return this->mIsInit;
}

void
Sequence::createCommandPool()
{
    SPDLOG_DEBUG("Kompute Sequence creating command pool");

    if (!this->mDevice) {
        throw std::runtime_error("Kompute Sequence device is null");
    }
    if (this->mQueueIndex < 0) {
        throw std::runtime_error("Kompute Sequence queue index not provided");
    }

    this->mFreeCommandPool = true;

    vk::CommandPoolCreateInfo commandPoolInfo(vk::CommandPoolCreateFlags(),
                                              this->mQueueIndex);
    this->mCommandPool = std::make_shared<vk::CommandPool>();
    this->mDevice->createCommandPool(
      &commandPoolInfo, nullptr, this->mCommandPool.get());
    SPDLOG_DEBUG("Kompute Sequence Command Pool Created");
}

void
Sequence::createCommandBuffer()
{
    SPDLOG_DEBUG("Kompute Sequence creating command buffer");
    if (!this->mDevice) {
        throw std::runtime_error("Kompute Sequence device is null");
    }
    if (!this->mCommandPool) {
        throw std::runtime_error("Kompute Sequence command pool is null");
    }

    this->mFreeCommandBuffer = true;

    vk::CommandBufferAllocateInfo commandBufferAllocateInfo(
      *this->mCommandPool, vk::CommandBufferLevel::ePrimary, 1);

    this->mCommandBuffer = std::make_shared<vk::CommandBuffer>();
    this->mDevice->allocateCommandBuffers(&commandBufferAllocateInfo,
                                          this->mCommandBuffer.get());
    SPDLOG_DEBUG("Kompute Sequence Command Buffer Created");
}

}
