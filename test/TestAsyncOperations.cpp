
#include "gtest/gtest.h"

#include <chrono>

#include "kompute/Kompute.hpp"

TEST(TestAsyncOperations, TestManagerAsync)
{
    uint32_t size = 100000;

    uint32_t numParallel = 6;

    std::string shader(R"(
        #version 450

        layout (local_size_x = 1) in;

        layout(set = 0, binding = 0) buffer a { float pa[]; };
        layout(set = 0, binding = 1) buffer b { float pb[]; };

        void main() {
            uint index = gl_GlobalInvocationID.x;

            for (int i = 0; i < 10000; i++)
            {
                pa[index] += 1.0;
            }

            pb[index] = pa[index];
            pa[index] = 0;
        }
    )");

    std::vector<float> data(size, 0.0);
    std::vector<float> resultSync(size, 10000);
    std::vector<float> resultAsync(size, 10000);

    kp::Manager mgr;

    std::vector<std::shared_ptr<kp::Tensor>> inputsSyncA;
    std::vector<std::shared_ptr<kp::Tensor>> inputsSyncB;

    for (uint32_t i = 0; i < numParallel; i++) {
        inputsSyncA.push_back(std::make_shared<kp::Tensor>(kp::Tensor(data)));
        inputsSyncB.push_back(std::make_shared<kp::Tensor>(kp::Tensor(data)));
    }

    mgr.evalOpDefault<kp::OpTensorCreate>(inputsSyncA);
    mgr.evalOpDefault<kp::OpTensorCreate>(inputsSyncB);

    auto startSync = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 0; i < numParallel; i++) {
        mgr.evalOpDefault<kp::OpAlgoBase<>>(
          { inputsSyncA[i],  inputsSyncB[i] }, 
          std::vector<char>(shader.begin(), shader.end()));

    }

    mgr.evalOpDefault<kp::OpTensorSyncLocal>(inputsSyncB);

    auto endSync = std::chrono::high_resolution_clock::now();
    auto durationSync = std::chrono::duration_cast<std::chrono::microseconds>(endSync - startSync).count();

    for (uint32_t i = 0; i < numParallel; i++) {
        EXPECT_EQ(inputsSyncB[i]->data(), resultSync);
    }

    kp::Manager mgrAsync(0, numParallel);

    std::vector<std::shared_ptr<kp::Tensor>> inputsAsyncA;
    std::vector<std::shared_ptr<kp::Tensor>> inputsAsyncB;

    for (uint32_t i = 0; i < numParallel; i++) {
        inputsAsyncA.push_back(std::make_shared<kp::Tensor>(kp::Tensor(data)));
        inputsAsyncB.push_back(std::make_shared<kp::Tensor>(kp::Tensor(data)));
    }

    mgrAsync.evalOpDefault<kp::OpTensorCreate>(inputsAsyncA);
    mgrAsync.evalOpDefault<kp::OpTensorCreate>(inputsAsyncB);

    for (uint32_t i = 0; i < numParallel; i++) {
        mgrAsync.createManagedSequence("async" + std::to_string(i), i);
    }

    auto startAsync = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 0; i < numParallel; i++) {
        mgrAsync.evalOpAsync<kp::OpAlgoBase<>>(
          { inputsAsyncA[i], inputsAsyncB[i] }, 
          "async" + std::to_string(i), 
          std::vector<char>(shader.begin(), shader.end()));
    }

    for (uint32_t i = 0; i < numParallel; i++) {
        mgrAsync.evalOpAwait("async" + std::to_string(i));
    }

    mgrAsync.evalOpDefault<kp::OpTensorSyncLocal>({ inputsAsyncB });

    auto endAsync = std::chrono::high_resolution_clock::now();
    auto durationAsync = std::chrono::duration_cast<std::chrono::microseconds>(endAsync - startAsync).count();

    for (uint32_t i = 0; i < numParallel; i++) {
        EXPECT_EQ(inputsAsyncB[i]->data(), resultAsync);
    }

    SPDLOG_ERROR("Total Sync: {}", durationSync);
    SPDLOG_ERROR("Total Async: {}", durationAsync);
}
