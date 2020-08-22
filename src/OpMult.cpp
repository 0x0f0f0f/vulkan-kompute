
#include <chrono>
#include <thread>

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
        spdlog::warn("Kompute OpMult called with more than 3 tensors");
    }

    this->mTensorLHS = tensors[0];
    this->mTensorRHS = tensors[1];
    this->mTensorOutput = tensors[2];

    // The dispatch size is set up based on either explicitly provided template parameters or by default it would take the shape and size of the tensors
    if (tX > 0) {
        // If at least the x value is provided we use mainly the parameters provided
        this->mX = tX;
        this->mY = tY > 0 ? tY : 1;
        this->mZ = tZ > 0 ? tZ : 1;
    }
    else {
        // TODO: Fully support the full size dispatch using size for the shape
        this->mX = this->mTensorLHS->size();
        this->mY = 1;
        this->mZ = 1;
    }
    spdlog::info("Kompute OpMult dispatch size X: {}, Y: {}, Z: {}", this->mX, this->mY, this->mZ);

    // TODO: Explore adding a validate function
    if (!(this->mTensorLHS->isInit() && this->mTensorRHS->isInit() &&
          this->mTensorOutput->isInit())) {
        throw std::runtime_error(
          "Kompute OpMult all tensor parameters must be initialised. LHS: " +
          std::to_string(this->mTensorLHS->isInit()) +
          " RHS: " + std::to_string(this->mTensorRHS->isInit()) +
          " Output: " + std::to_string(this->mTensorOutput->isInit()));
    }

    // TODO: Explore use-cases where tensors shouldn't be the same size, and how
    // to deal with those situations
    if (!(this->mTensorLHS->size() == this->mTensorRHS->size() &&
          this->mTensorRHS->size() == this->mTensorOutput->size())) {
        throw std::runtime_error(
          "Kompute OpMult all tensor parameters must be the same size LHS: " +
          std::to_string(this->mTensorLHS->size()) +
          " RHS: " + std::to_string(this->mTensorRHS->size()) +
          " Output: " + std::to_string(this->mTensorOutput->size()));
    }

    this->mTensorOutputStaging = std::make_shared<Tensor>(
      this->mTensorOutput->data(), Tensor::TensorTypes::eStaging);

    this->mTensorOutputStaging->init(this->mPhysicalDevice,
                                     this->mDevice,
                                     this->mCommandBuffer,
                                     this->mTensorOutput->data());

    // TODO: Make this path configurable
    this->mAlgorithm->init("shaders/glsl/opmult.comp.spv", tensors);
}

void
OpMult::record()
{
    SPDLOG_DEBUG("Kompute OpMult record called");

    this->mTensorLHS->recordBufferMemoryBarrier(
        vk::AccessFlagBits::eHostWrite,
        vk::AccessFlagBits::eShaderRead,
        vk::PipelineStageFlagBits::eHost,
        vk::PipelineStageFlagBits::eComputeShader);
    this->mTensorRHS->recordBufferMemoryBarrier(
        vk::AccessFlagBits::eHostWrite,
        vk::AccessFlagBits::eShaderRead,
        vk::PipelineStageFlagBits::eHost,
        vk::PipelineStageFlagBits::eComputeShader);

    this->mAlgorithm->recordDispatch(this->mX, this->mY, this->mZ);

    this->mTensorOutput->recordBufferMemoryBarrier(
        vk::AccessFlagBits::eShaderWrite,
        vk::AccessFlagBits::eTransferRead,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eTransfer);

    this->mTensorOutputStaging->recordCopyFrom(this->mTensorOutput);

    this->mTensorOutput->recordBufferMemoryBarrier(
        vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eHostRead,
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eHost);
}

void
OpMult::postSubmit()
{
    SPDLOG_DEBUG("Kompute OpCreateTensor postSubmit called");

    this->mTensorOutputStaging->copyDataFromHostBuffer();

    this->mTensorOutput->setData(this->mTensorOutputStaging->data());
}

}
