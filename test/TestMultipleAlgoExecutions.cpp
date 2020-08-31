
#include "catch2/catch.hpp"

#include "kompute/Kompute.hpp"

#include <fmt/ranges.h>

TEST_CASE("test_multiple_algo_exec_single_cmd_buf_record") {

    kp::Manager mgr;

    std::shared_ptr<kp::Tensor> tensorA{ new kp::Tensor({ 0, 0, 0 })};

    std::string shader(
        "#version 450\n"
        "layout (local_size_x = 1) in;\n"
        "layout(set = 0, binding = 0) buffer a { uint pa[]; };\n"
        "void main() {\n"
        "    uint index = gl_GlobalInvocationID.x;\n"
        "    pa[index] = pa[index] + 1;\n"
        "}\n"
    );

    std::weak_ptr<kp::Sequence> sqWeakPtr = mgr.getOrCreateManagedSequence("newSequence");
    if (std::shared_ptr<kp::Sequence> sq = sqWeakPtr.lock()) {
        sq->begin();

        sq->record<kp::OpCreateTensor>({ tensorA });

        sq->record<kp::OpAlgoBase<3, 1, 1>>(
                { tensorA }, 
                false, // Whether to copy output from device
                std::vector<char>(shader.begin(), shader.end()));
        sq->record<kp::OpAlgoBase<3, 1, 1>>(
                { tensorA }, 
                false, // Whether to copy output from device
                std::vector<char>(shader.begin(), shader.end()));
        sq->record<kp::OpAlgoBase<3, 1, 1>>(
                { tensorA }, 
                true, // Whether to copy output from device
                std::vector<char>(shader.begin(), shader.end()));

        sq->end();
        sq->eval();
    }
    sqWeakPtr.reset();

    REQUIRE(tensorA->data() == std::vector<uint32_t>{3, 3, 3});
}

TEST_CASE("test_multiple_algo_exec_multiple_record") {

    kp::Manager mgr;

    std::shared_ptr<kp::Tensor> tensorA{ new kp::Tensor({ 0, 0, 0 })};

    std::string shader(
        "#version 450\n"
        "layout (local_size_x = 1) in;\n"
        "layout(set = 0, binding = 0) buffer a { uint pa[]; };\n"
        "void main() {\n"
        "    uint index = gl_GlobalInvocationID.x;\n"
        "    pa[index] = pa[index] + 1;\n"
        "}\n"
    );

    std::weak_ptr<kp::Sequence> sqWeakPtr = mgr.getOrCreateManagedSequence("newSequence");
    if (std::shared_ptr<kp::Sequence> sq = sqWeakPtr.lock()) {
        sq->begin();

        sq->record<kp::OpCreateTensor>({ tensorA });

        sq->record<kp::OpAlgoBase<3, 1, 1>>(
                { tensorA }, 
                false, // Whether to copy output from device
                std::vector<char>(shader.begin(), shader.end()));

        sq->end();
        sq->eval();

        sq->begin();

        sq->record<kp::OpAlgoBase<3, 1, 1>>(
                { tensorA }, 
                false, // Whether to copy output from device
                std::vector<char>(shader.begin(), shader.end()));

        sq->end();
        sq->eval();

        sq->begin();

        sq->record<kp::OpAlgoBase<3, 1, 1>>(
                { tensorA }, 
                true, // Whether to copy output from device
                std::vector<char>(shader.begin(), shader.end()));

        sq->end();
        sq->eval();
    }
    sqWeakPtr.reset();

    REQUIRE(tensorA->data() == std::vector<uint32_t>{3, 3, 3});

}

TEST_CASE("test_multiple_algo_exec_multiple_sequence") {

    kp::Manager mgr;

    std::shared_ptr<kp::Tensor> tensorA{ new kp::Tensor({ 0, 0, 0 })};

    std::string shader(
        "#version 450\n"
        "layout (local_size_x = 1) in;\n"
        "layout(set = 0, binding = 0) buffer a { uint pa[]; };\n"
        "void main() {\n"
        "    uint index = gl_GlobalInvocationID.x;\n"
        "    pa[index] = pa[index] + 1;\n"
        "}\n"
    );

    std::weak_ptr<kp::Sequence> sqWeakPtr = mgr.getOrCreateManagedSequence("newSequence");
    if (std::shared_ptr<kp::Sequence> sq = sqWeakPtr.lock()) {
        sq->begin();

        sq->record<kp::OpCreateTensor>({ tensorA });

        sq->record<kp::OpAlgoBase<3, 1, 1>>(
                { tensorA }, 
                true, // Whether to copy output from device
                std::vector<char>(shader.begin(), shader.end()));

        sq->end();
        sq->eval();
    }

    std::weak_ptr<kp::Sequence> sqWeakPtr2 = mgr.getOrCreateManagedSequence("newSequence2");
    if (std::shared_ptr<kp::Sequence> sq = sqWeakPtr.lock()) {
        sq->begin();

        sq->record<kp::OpAlgoBase<3, 1, 1>>(
                { tensorA }, 
                true, // Whether to copy output from device
                std::vector<char>(shader.begin(), shader.end()));

        sq->end();
        sq->eval();
    }


    std::weak_ptr<kp::Sequence> sqWeakPtr3 = mgr.getOrCreateManagedSequence("newSequence3");
    if (std::shared_ptr<kp::Sequence> sq = sqWeakPtr.lock()) {
        sq->begin();

        sq->record<kp::OpAlgoBase<3, 1, 1>>(
                { tensorA }, 
                true, // Whether to copy output from device
                std::vector<char>(shader.begin(), shader.end()));

        sq->end();
        sq->eval();
    }

    REQUIRE(tensorA->data() == std::vector<uint32_t>{3, 3, 3});
}

