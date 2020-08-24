# Vulkan Kompute

## Principles

* Single header easy to import library to boost your Vulkan compute experience (WIP)
* Non-vulkan naming convention to disambiguate Vulkan vs Kompute components
* Extends the existing vulkan API with a simpler compute-specific interface
* BYOV: Play nice with existing Vulkan applications with a bring-your-own-Vulkan design
* TODO

## Getting Started

Run your tensors against default or custom equations via the Manager.

```c++
int main() {

    kp::Manager mgr; // Automatically selects Device 0

    std::shared_ptr<kp::Tensor> tensorLHS{ new kp::Tensor({ 0.0, 1.0, 2.0 }) };
    mgr.evalOp<kp::OpCreateTensor>({ tensorLHS });

    std::shared_ptr<kp::Tensor> tensorRHS{ new kp::Tensor( { 2.0, 4.0, 6.0 }) };
    mgr.evalOp<kp::OpCreateTensor>({ tensorRHS });

    // TODO: Add capabilities for just output tensor types
    std::shared_ptr<kp::Tensor> tensorOutput{ new kp::Tensor({ 0.0, 0.0, 0.0 }) };
    mgr.evalOp<kp::OpCreateTensor>({ tensorOutput });

    mgr.evalOp<kp::OpMult>({ tensorLHS, tensorRHS, tensorOutput });

    std::cout << fmt::format("Output: {}", tensorOutput.data()) << std::endl;
}
```

Record commands in a single submit by using a Sequence.

```c++
int main() {
    kp::Manager mgr;

    std::shared_ptr<kp::Tensor> tensorLHS{ new kp::Tensor({ 0.0, 1.0, 2.0 }) };
    std::shared_ptr<kp::Tensor> tensorRHS{ new kp::Tensor( { 2.0, 4.0, 6.0 }) };
    std::shared_ptr<kp::Tensor> tensorOutput{ new kp::Tensor({ 0.0, 0.0, 0.0 }) };

    kp::Sequence sq = mgr.constructSequence();
    // Begin recoding commands
    sq.begin();

    // Record sequence of operations to be sent to GPU in batch
    {
        sq.record<kp::OpCreateTensor>({ tensorLHS });
        sq.record<kp::OpCreateTensor>({ tensorRHS });
        sq.record<kp::OpCreateTensor>({ tensorOutput });

        sq.record<kp::OpMult<>>({ tensorLHS, tensorRHS, tensorOutput });
    }
    // Stop recording
    sq.end();
    // Submit operations to GPU
    sq.eval();

    std::cout << fmt::format("Output: {}", tensorOutput.data()) << std::endl;
}
```

Create your own operation

```c++
class CustomOp : kp::OpBase {
    // ...
    void init(std::shared_ptr<Tensor> tensors) {
        // ... extra steps to initialise tensors
        this->mAlgorithm->init("path/to/your/shader.compute.spv", tensors);
    }
}

int main() {
    kp::Manager kManager(); // Chooses device 0 

    kp::Tensor inputOne({0, 1, 2, 3}); 

    kp::Tensor inputTwo({0, 1, 2, 3});

    kp::Tensor output( {0, 0, 0, 0} );
    kManager.eval<kp::CustomOp>({ inputOne, inputTwo, output });

    std::cout << fmt::format("Output: {}", tensorOutput.data()) << std::endl;
}
```


## Development

* Follows Mozilla C++ Style Guide https://www-archive.mozilla.org/hacking/mozilla-style-guide.html
    + Uses post-commit hook to run the linter, you can set it up so it runs the linter before commit
* Uses vcpkg for finding the dependencies, it's the recommanded set up to retrieve the libraries
