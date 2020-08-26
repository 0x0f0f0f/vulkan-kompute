![GitHub](https://img.shields.io/badge/Release-ALPHA-yellow.svg)
![GitHub](https://img.shields.io/badge/Version-0.1.0-green.svg)
![GitHub](https://img.shields.io/badge/C++-11—20-purple.svg)
![GitHub](https://img.shields.io/badge/Build-cmake-red.svg)
![GitHub](https://img.shields.io/badge/Python-3.5—3.8-blue.svg)
![GitHub](https://img.shields.io/badge/License-Apache-black.svg)

<table>
<tr>

<td width="20%">
<img src="docs/images/kompute.jpg">
</td>

<td>

<h1>Vulkan Kompute</h1>
<h3>The General Purpose Vulkan Compute Framework</h3>

</td>

</tr>
</table>

## Principles & Features

* Single header easy to import static library
* Packaged with vcpkg for easy download and integration with projects
* Non-Vulkan naming convention to disambiguate Vulkan vs Kompute components
* Extends the existing Vulkan API with a compute-specific interface
* BYOV: Play nice with existing Vulkan applications with a bring-your-own-Vulkan design
* Directed acyclic memory management and relationships of ownership
* Explicit memory management responsibilities
* Opinionated approach towards base interface for memory management hierarchy with explicit and extensible design
* Best practices for safe memory GPU / Vulkan memory management (WIP)

## Getting Started

Run your tensors against default operations via the Manager.

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

Record commands in a single submit by using a Sequence to send in batch to GPU.

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

Create your own custom operations to leverage Vulkan Compute for your specialised use-cases.

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

## Motivations

Vulkan Kompute was created after identifying the challenge most GPU processing projects with Vulkan undergo - namely having to build extensive boilerplate for Vulkan and create abstractions and interfaces that expose the core compute capabilities. It is only after a few thousand lines of code that it's possible to start building the application-specific logic. 

We believe Vulkan has an excellent design in its way to interact with the GPU, so by no means we aim to abstract or hide any complexity, but instead we want to provide a baseline of tools and interfaces that allow Vulkan Compute developers to focus on the higher level computational complexities of the application.

It is because of this that we have adopted development principles for the project that ensure the Vulkan API is augmented specifically for computation, whilst speeding development iterations and opening the doors to further use-cases.

## Components & Architecture

The core architecture of Kompute include the following:
* Kompute Manager - Base orchestrator which creates and manages device and child components
* Kompute Sequence - Container of operations that can be sent to GPU as batch
* Kompute Operation - Individual operation which performs actions on top of tensors and (opt) algorithms
* Kompute Tensor - Tensor structured data used in GPU operations
* Kompute Algorithm - Abstraction for (shader) code executed in the GPU
* Kompute ParameterGroup - Container that can group tensors to be fed into an algorithm

To see a full breakdown you can read further in the documentation.

<table>
<th>
Full Vulkan Components
</th>
<th>
Simplified Kompute Components
</th>
<tr>
<td width=30%>


<img width="100%" src="docs/images/kompute-vulkan-architecture.jpg">

<br>
<br>
(very tiny, check the docs to for details)
<br>
<br>

<img width="100%" src="https://www.memesmonkey.com/images/memesmonkey/a2/a29e06384bf8981e7ae66d5150383f6e.jpeg">

</td>
<td>
<img width="100%" src="docs/images/kompute-architecture.jpg">
</td>
</tr>
</table>


## Kompute Development

We appreciate PRs and Issues. If you want to contribute try checking the "Good first issue" tag, but even using Vulkan Kompute and reporting issues is a great contribution!

### Dev Overview

* Follows Mozilla C++ Style Guide https://www-archive.mozilla.org/hacking/mozilla-style-guide.html
    + Uses post-commit hook to run the linter, you can set it up so it runs the linter before commit
* Uses vcpkg for finding the dependencies, it's the recommanded set up to retrieve the libraries
    + All dependencies are defined in vcpkg.json 
* Uses cmake as build system, and provides a top level makefile with recommended command
* Uses xxd (or xxd.exe windows 64bit port) to convert shader spirv to header files


