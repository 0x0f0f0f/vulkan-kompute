#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

// SPDLOG_ACTIVE_LEVEL must be defined before spdlog.h import
#if DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#endif

#include <spdlog/spdlog.h>

#include "OpBase.hpp"

namespace kp {

class Sequence
{
  public:
    Sequence();
    Sequence(std::shared_ptr<vk::PhysicalDevice> physicalDevice,
             std::shared_ptr<vk::Device> device,
             std::shared_ptr<vk::Queue> computeQueue,
             uint32_t queueIndex);
    ~Sequence();

    // Record command functions
    void begin();
    void end();
    void eval();

    template<typename T, typename... TArgs>
    void record(TArgs&&... args)
    {
        static_assert(std::is_base_of<OpBase, T>::value,
                      "Template only valid with OpBase derived classes");

        SPDLOG_DEBUG("Kompute Sequence record");

        T* op =
          new T(this->mPhysicalDevice, this->mDevice, this->mCommandBuffer);
        OpBase* baseOp = dynamic_cast<OpBase*>(op);

        std::unique_ptr<OpBase> baseOpPtr{ baseOp };

        baseOpPtr->init(std::forward<TArgs>(args)...);
        baseOpPtr->record();

        operations.push_back(std::move(baseOpPtr));
    }

  private:
    std::shared_ptr<vk::PhysicalDevice> mPhysicalDevice = nullptr;
    std::shared_ptr<vk::Device> mDevice = nullptr;
    std::shared_ptr<vk::Queue> mComputeQueue = nullptr;
    uint32_t mQueueIndex = -1;
    std::shared_ptr<vk::CommandPool> mCommandPool = nullptr;
    bool mFreeCommandPool = false;
    std::shared_ptr<vk::CommandBuffer> mCommandBuffer = nullptr;
    bool mFreeCommandBuffer = false;

    // Base op objects
    std::vector<std::unique_ptr<OpBase>> operations;

    // Record state
    bool mRecording = false;

    // Create functions
    void createCommandPool();
    void createCommandBuffer();
};

} // End namespace kp
