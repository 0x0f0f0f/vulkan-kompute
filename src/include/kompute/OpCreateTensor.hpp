#pragma once

#include "kompute/Core.hpp"

#include "kompute/Tensor.hpp"

#include "kompute/OpBase.hpp"

namespace kp {

class OpCreateTensor : public OpBase
{
  public:
    OpCreateTensor();

    OpCreateTensor(std::shared_ptr<vk::PhysicalDevice> physicalDevice,
                   std::shared_ptr<vk::Device> device,
                   std::shared_ptr<vk::CommandBuffer> commandBuffer);

    ~OpCreateTensor();

    void init(std::vector<std::shared_ptr<Tensor>> tensors) override;

    void record() override;

    void postSubmit() override;

  private:
    std::shared_ptr<Tensor> mPrimaryTensor;
    bool mFreePrimaryTensorResources = false;
    std::shared_ptr<Tensor> mStagingTensor;
    bool mFreeStagingTensorResources = false;
};

} // End namespace kp
