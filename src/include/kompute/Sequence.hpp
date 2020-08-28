#pragma once

#include "kompute/Core.hpp"

#include "kompute/OpBase.hpp"

namespace kp {

/**
    Container of operations that can be sent to GPU as batch
*/
class Sequence
{
  public:
    /**
        Constructor
    */
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

    // TODO: Explore design without template using just top level class
    template<typename T, typename... TArgs>
    void record(std::vector<std::shared_ptr<Tensor>> tensors)
    {
        static_assert(std::is_base_of<OpBase, T>::value,
                      "Template only valid with OpBase derived classes");

        SPDLOG_DEBUG("Kompute Sequence record function started");

        SPDLOG_DEBUG("Kompute Sequence creating OpBase derived class instance");
        T* op =
          new T(this->mPhysicalDevice, this->mDevice, this->mCommandBuffer);
        OpBase* baseOp = dynamic_cast<OpBase*>(op);

        std::unique_ptr<OpBase> baseOpPtr{ baseOp };

        SPDLOG_DEBUG(
          "Kompute Sequence running init on OpBase derived class instance");
        baseOpPtr->init(tensors);

        SPDLOG_DEBUG(
          "Kompute Sequence running record on OpBase derived class instance");
        baseOpPtr->record();

        mOperations.push_back(std::move(baseOpPtr));
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
    std::vector<std::unique_ptr<OpBase>> mOperations;

    // Record state
    bool mRecording = false;

    // Create functions
    void createCommandPool();
    void createCommandBuffer();
};

} // End namespace kp
