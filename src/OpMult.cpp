

#include "Tensor.hpp"

#include "OpMult.hpp"

namespace kp {

OpMult::OpMult()
{
    SPDLOG_DEBUG("Kompute OpMult constructor base");
}

// TODO: Remove physicalDevice from main initialiser
OpMult::OpMult(std::shared_ptr<vk::PhysicalDevice> physicalDevice,
               std::shared_ptr<vk::Device> device,
               std::shared_ptr<vk::CommandBuffer> commandBuffer)
  : OpBase(physicalDevice, device, commandBuffer)
{
    SPDLOG_DEBUG("Kompute OpMult constructor with params");

    this->mAlgorithm = std::make_shared<Algorithm>(device, commandBuffer);
}

OpMult::~OpMult()
{
    SPDLOG_DEBUG("Kompute OpMult destructor started");
}

void
OpMult::init(std::vector<std::shared_ptr<Tensor>> tensors)
{
    SPDLOG_DEBUG("Kompute OpMult init called");

    if (tensors.size() < 3) {
        throw std::runtime_error(
          "Kompute OpMult called with less than 1 tensor");
    } else if (tensors.size() > 3) {
        spdlog::warn("Kompute OpMult called with more than 2 tensor");
    }

    this->mTensorLHS = tensors[0];
    this->mTensorRHS = tensors[1];
    this->mTensorOutput = tensors[2];

    this->mTensorOutputStaging= std::make_shared<Tensor>(
      this->mTensorOutput->data(), Tensor::TensorTypes::eStaging);

    this->mAlgorithm->init(
        "shaders/glsl/computeheadless.comp.spv", tensors);
}

void
OpMult::record()
{
    SPDLOG_DEBUG("Kompute OpMult record called");

    this->mAlgorithm->recordDispatch(1, 1, 1);

    this->mTensorOutputStaging->recordCopyFrom(this->mTensorOutput);
}

void OpMult::postSubmit()
{
    SPDLOG_DEBUG("Kompute OpCreateTensor postSubmit called");

    this->mTensorOutputStaging->copyDataFromHostBuffer();
    this->mTensorOutput->setData(this->mTensorOutputStaging->data());
}

}
