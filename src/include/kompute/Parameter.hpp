#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

// SPDLOG_ACTIVE_LEVEL must be defined before spdlog.h import
#if DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#endif

#include <spdlog/spdlog.h>

#include "kompute/Tensor.hpp"

namespace kp {

class Algorithm
{
  public:
    Algorithm();

    Algorithm(std::shared_ptr<vk::Device> device);

    // TODO: Add specialisation data
    // TODO: Explore other ways of passing shader (ie raw bytes)
    void init(std::string shaderFilePath,
              std::vector<std::shared_ptr<Tensor>> tensorParams);

    ~Algorithm();

  private:
    // Shared resources
    std::shared_ptr<vk::Device> mDevice;

    // Resources owned by default
    std::shared_ptr<vk::DescriptorSetLayout> mDescriptorSetLayout;
    bool mFreeDescriptorSetLayout = false;
    std::shared_ptr<vk::DescriptorPool> mDescriptorPool;
    bool mFreeDescriptorPool = false;
    std::shared_ptr<vk::DescriptorSet> mDescriptorSet;
    bool mFreeDescriptorSet = false;
    std::shared_ptr<vk::ShaderModule> mShaderModule;
    bool mFreeShaderModule = false;
    std::shared_ptr<vk::PipelineLayout> mPipelineLayout;
    bool mFreePipelineLayout = false;
    std::shared_ptr<vk::PipelineCache> mPipelineCache;
    bool mFreePipelineCache = false;
    std::shared_ptr<vk::Pipeline> mPipeline;
    bool mFreePipeline = false;

    // Create util functions
    void createParameters();
    void createShaderModule(std::string shaderFilePath);
    void createPipeline();
};

} // End namespace kp
