#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

// SPDLOG_ACTIVE_LEVEL must be defined before spdlog.h import
#if DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#endif

#include <spdlog/spdlog.h>

#include "Sequence.hpp"

namespace kp {

class Manager
{
private:

public:
    Manager();

    ~Manager();

    // Evaluate actions
    template <typename T, typename... TArgs>
    void eval(TArgs&&... args) {
        SPDLOG_DEBUG("Kompute Manager eval triggered");
        Sequence sq(this->mDevice, this->mComputeQueue, this->mComputeQueueFamilyIndex);
        SPDLOG_DEBUG("Kompute Manager created sequence");
        sq.begin();
        SPDLOG_DEBUG("Kompute Manager sequence begin");
        sq.record<T>(std::forward<TArgs>(args)...);
        SPDLOG_DEBUG("Kompute Manager sequence end");
        sq.end();
        SPDLOG_DEBUG("Kompute Manager sequence eval");
        sq.eval();
        SPDLOG_DEBUG("Kompute Manager sequence done");
    }


private:
    std::shared_ptr<vk::Instance> mInstance = nullptr;
    bool mFreeInstance = false;
    uint32_t mPhysicalDeviceIndex = -1;
    std::shared_ptr<vk::Device> mDevice = nullptr;
    bool mFreeDevice = false;
    uint32_t mComputeQueueFamilyIndex = -1;
    std::shared_ptr<vk::Queue> mComputeQueue = nullptr;

#if DEBUG
    vk::DebugReportCallbackEXT mDebugReportCallback;
    vk::DispatchLoaderDynamic mDebugDispatcher;
#endif

    // Create functions
    void createInstance();
    void createDevice();
};

} // End namespace kp

