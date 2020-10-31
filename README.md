
![GitHub](https://img.shields.io/badge/Version-0.4.0-green.svg)
![GitHub](https://img.shields.io/badge/C++-14—20-purple.svg)
![GitHub](https://img.shields.io/badge/Build-cmake-red.svg)
![GitHub](https://img.shields.io/badge/Python-3.5—3.8-blue.svg)
![GitHub](https://img.shields.io/badge/License-Apache-black.svg)

<table>
<tr>

<td width="20%">
<img src="https://raw.githubusercontent.com/EthicalML/vulkan-kompute/master/docs/images/kompute.jpg">
</td>

<td>

<h1>Vulkan Kompute</h1>
<h3>The General Purpose Vulkan Compute Framework.</h3>

</td>

</tr>
</table>

<h4>Blazing fast, mobile-enabled, asynchronous, and optimized for advanced GPU processing usecases.</h4>

🔋 [Documentation](https://kompute.cc) 💻 [Blog Post](https://medium.com/@AxSaucedo/machine-learning-and-data-processing-in-the-gpu-with-vulkan-kompute-c9350e5e5d3a) ⌨ [Examples](#more-examples) 💾


## Principles & Features

* [Single header](#setup) library for simple import to your project
* [Documentation](https://kompute.cc) leveraging doxygen and sphinx 
* [Asynchronous & parallel processing](#asynchronous-and-parallel-operations) capabilities with multi-queue command submission
* [Non-Vulkan naming conventions](#architectural-overview) to disambiguate Vulkan vs Kompute components
* BYOV: [Bring-your-own-Vulkan design](#motivations) to play nice with existing Vulkan applications
* Explicit relationships for GPU and host [memory ownership and memory management](https://kompute.cc/overview/memory-management.html)
* [Short code examples](#simple-examples) showing the core features 
* Longer tutorials for [machine learning 🤖](https://towardsdatascience.com/machine-learning-and-data-processing-in-the-gpu-with-vulkan-kompute-c9350e5e5d3a), [mobile development 📱](https://towardsdatascience.com/gpu-accelerated-machine-learning-in-your-mobile-applications-using-the-android-ndk-vulkan-kompute-1e9da37b7617) and [game development 🎮](https://towardsdatascience.com/supercharging-game-development-with-gpu-accelerated-ml-using-vulkan-kompute-the-godot-game-engine-4e75a84ea9f0).

![](https://raw.githubusercontent.com/ethicalml/vulkan-kompute/master/docs/images/komputer-2.gif)

## Getting Started

### Setup

Kompute is provided as a single header file [`Kompute.hpp`](#setup). See [build-system section](#build-overview) for configurations available.


### Your First Kompute (Simple Version)

This simple example will show the basics of Kompute through the high level API.

1. Create Kompute Manager with default settings (device 0 and first compute compatible queue)
2. Create and initialise Kompute Tensors through manager
3. Run multiplication operation synchronously
4. Map results back from GPU memory to print the results

View the [extended version](#your-first-kompute-extended-version) or [more examples](#simple-examples).

```c++
int main() {

    // 1. Create Kompute Manager with default settings (device 0 and first compute compatible queue)
    kp::Manager mgr; 

    // 2. Create and initialise Kompute Tensors through manager
    auto tensorInA = mgr.buildTensor({ 2., 2., 2. });
    auto tensorInB = mgr.buildTensor({ 1., 2., 3. });
    auto tensorOut = mgr.buildTensor({ 0., 0., 0. });

    // 3. Run multiplication operation synchronously
    mgr.evalOpDefault<kp::OpMult<>>(
        { tensorInA, tensorInB, tensorOut })

    // 4. Map results back from GPU memory to print the results
    mgr.evalOpDefault<kp::OpTensorSyncLocal>({ tensorInA, tensorInB, tensorOut })

    // Prints the output which is Output: { 2, 4, 6 }
    std::cout << fmt::format("Output: {}", 
        tensorOut.data()) << std::endl;
}
```

## Your First Kompute (Extended Version)

We will now show the [same example as above](#your-first-kompute-simple-version) but leveraging more advanced Kompute features:

1. Create Kompute Manager with explicit device 0 and single queue of familyIndex 2
2. Explicitly create Kompute Tensors without initializing in GPU
3. Initialise the Kompute Tensor in GPU memory and map data into GPU
4. Run operation with custom compute shader code asynchronously with explicit dispatch layout
5. Create managed sequence to submit batch operations to the CPU
6. Map data back to host by running the sequence of batch operations

View [more examples](https://kompute.cc/overview/advanced-examples.html#simple-examples).

```c++
int main() {

    // 1. Create Kompute Manager with explicit device 0 and single queue of familyIndex 2
    kp::Manager mgr(0, { 2 });

    // 2. Explicitly create Kompute Tensors without initializing in GPU
    auto tensorInA = std::make_shared<kp::Tensor>(kp::Tensor({ 2., 2., 2. }));
    auto tensorInB = std::make_shared<kp::Tensor>(kp::Tensor({ 1., 2., 3. }));
    auto tensorOut = std::make_shared<kp::Tensor>(kp::Tensor({ 0., 0., 0. }));

    // 3. Initialise the Kompute Tensor in GPU memory and map data into GPU
    mgr.evalOpDefault<kp::OpTensorCreate>({ tensorInA, tensorInB, tensorOut });

    // 4. Run operation with custom compute shader code asynchronously with explicit dispatch layout
    mgr.evalOpAsyncDefault<kp::OpAlgoBase<3, 1, 1>>(
        { tensorInA, tensorInB, tensorOut }, 
        shaderData); // "shaderData" defined is below and can be glsl/spirv string, or path to file

    // 4.1. Before submitting sequence batch we wait for the async operation
    mgr.evalOpAwaitDefault();

    // 5. Create managed sequence to submit batch operations to the CPU
    std::shared_ptr<kp::Sequence> sq = mgr.getOrCreateManagedSequence("seq").lock();

    // 5.1. Explicitly begin recording batch commands
    sq->begin();

    // 5.2. Record batch commands
    sq->record<kp::OpTensorSyncLocal({ tensorInA });
    sq->record<kp::OpTensorSyncLocal({ tensorInB });
    sq->record<kp::OpTensorSyncLocal({ tensorOut });

    // 5.3. Explicitly stop recording batch commands
    sq->end();

    // 6. Map data back to host by running the sequence of batch operations
    sq->eval();

    // Prints the output which is Output: { 2, 4, 6 }
    std::cout << fmt::format("Output: {}", 
        tensorOut.data()) << std::endl;
}
```

Your shader can be provided as raw glsl/hlsl string, SPIR-V bytes array (using our CLI), or string path to file containing either. Below are the examples of the valid ways of providing shader.

#### Passing raw GLSL/HLSL string

```c++
static std::string shaderString = (R"(
    #version 450

    layout (local_size_x = 1) in;

    // The input tensors bind index is relative to index in parameter passed
    layout(set = 0, binding = 0) buffer bina { float tina[]; };
    layout(set = 0, binding = 1) buffer binb { float tinb[]; };
    layout(set = 0, binding = 2) buffer bout { float tout[]; };

    void main() {
        uint index = gl_GlobalInvocationID.x;
        tout[index] = tina[index] * tinb[index];
    }
)");
static std::vector<char> shaderData(shaderString.begin(), shaderString.end());
```

#### Passing SPIR-V Bytes array 

You can use the Kompute [shader-to-cpp-header CLI](https://kompute.cc/overview/shaders-to-headers.html) to convert your GLSL/HLSL or SPIR-V shader into C++ header file (see documentation link for more info). This is useful if you want your binary to be compiled with all relevant artifacts.

```c++
static std::vector<uint8_t> shaderData = { 0x03, //... spirv bytes go here)
```

#### Path to file containing raw glsl/hlsl or SPIRV bytes

```c++
static std::string shaderData = "path/to/shader.glsl";
// Or SPIR-V
static std::string shaderData = "path/to/shader.glsl.spv";
```

## Architectural Overview

The core architecture of Kompute includes the following:
* [Kompute Manager](https://kompute.cc/overview/reference.html#manager) - Base orchestrator which creates and manages device and child components
* [Kompute Sequence](https://kompute.cc/overview/reference.html#sequence) - Container of operations that can be sent to GPU as batch
* [Kompute Operation (Base)](https://kompute.cc/overview/reference.html#algorithm) - Base class from which all operations inherit
* [Kompute Tensor](https://kompute.cc/overview/reference.html#tensor) - Tensor structured data used in GPU operations
* [Kompute Algorithm](https://kompute.cc/overview/reference.html#algorithm) - Abstraction for (shader) code executed in the GPU

To see a full breakdown you can read further in the [C++ Class Reference](https://kompute.cc/overview/reference.html).

<table>
<th>
Full Vulkan Components
</th>
<th>
Simplified Kompute Components
</th>
<tr>
<td width=30%>


<img width="100%" src="https://raw.githubusercontent.com/ethicalml/vulkan-kompute/master/docs/images/kompute-vulkan-architecture.jpg">

<br>
<br>
(very tiny, check the <a href="https://ethicalml.github.io/vulkan-kompute/overview/reference.html">full reference diagram in docs for details</a>)
<br>
<br>

<img width="100%" src="https://raw.githubusercontent.com/ethicalml/vulkan-kompute/master/docs/images/suspicious.jfif">

</td>
<td>
<img width="100%" src="https://raw.githubusercontent.com/ethicalml/vulkan-kompute/master/docs/images/kompute-architecture.jpg">
</td>
</tr>
</table>


## Asynchronous and Parallel Operations

Kompute provides flexibility to run operations in an asynrchonous way through Vulkan Fences. Furthermore, Kompute enables for explicit allocation of queues, which allow for parallel execution of operations across queue families.

The image below provides an intuition on how Kompute Sequences can be allocated to different queues to enable parallel execution based on hardware. You can see the [hands on example](https://kompute.cc/overview/advanced-examples.html#parallel-operations), as well as the [detailed documentation page](https://kompute.cc/overview/async-parallel.html) describing how it would work using an NVIDIA 1650 as an example. 

![](https://raw.githubusercontent.com/ethicalml/vulkan-kompute/master/docs/images/queue-allocation.jpg)

## Mobile Enabled

Kompute has been optimized to work in mobile environments. The [build system](#build-overview) enables for dynamic loading of the Vulkan shared library for Android environments, together with a working [Android NDK Vulkan wrapper](https://github.com/EthicalML/vulkan-kompute/tree/master/vk_ndk_wrapper_include) for the CPP headers.

<table>
<tr>

<td width="70%">
<p>
For a full deep dive you can read the blog post "<a href="https://towardsdatascience.com/gpu-accelerated-machine-learning-in-your-mobile-applications-using-the-android-ndk-vulkan-kompute-1e9da37b7617">Supercharging your Mobile Apps with On-Device GPU Accelerated Machine Learning</a>". 

You can also access the <a href="https://github.com/EthicalML/vulkan-kompute/tree/v0.4.0/examples/android/android-simple">end-to-end example code</a> in the repository, which can be run using android studio.

</p>


<img src="https://raw.githubusercontent.com/EthicalML/vulkan-kompute/android-example/docs/images/android-editor.jpg">

</td>


<td width="30%">
<img src="https://raw.githubusercontent.com/EthicalML/vulkan-kompute/android-example/docs/images/android-kompute.jpg">
</td>

</tr>
</table>

## Motivations

This project started after seeing that a lot of new and renowned ML & DL projects like Pytorch, Tensorflow, Alibaba DNN, Tencent NCNN - among others - have either integrated or are looking to integrate the Vulkan SDK to add mobile (and cross-vendor) GPU support.

The Vulkan SDK offers a great low level interface that enables for highly specialized optimizations - however it comes at a cost of highly verbose code which requires 500-2000 lines of code to even begin writing application code. This has resulted in each of these projects having to implement the same baseline to abstract the non-compute related features of Vulkan. This large amount of non-standardised boiler-plate can result in limited knowledge transfer, higher chance of unique framework implementation bugs being introduced, etc.

We are currently developing Vulkan Kompute not to hide the Vulkan SDK interface (as it's incredibly well designed) but to augment it with a direct focus on Vulkan's GPU computing capabilities. [This article](https://towardsdatascience.com/machine-learning-and-data-processing-in-the-gpu-with-vulkan-kompute-c9350e5e5d3a) provides a high level overview of the motivations of Kompute, together with a set of hands on examples that introduce both GPU computing as well as the core Vulkan Kompute architecture.

## More examples

### Simple examples

* [Pass shader as raw string](https://kompute.cc/overview/advanced-examples.html#simple-shader-example)
* [Record batch commands with a Kompute Sequence](https://kompute.cc/overview/advanced-examples.html#record-batch-commands)
* [Run Asynchronous Operations](https://kompute.cc/overview/advanced-examples.html#asynchronous-operations)
* [Run Parallel Operations Across Multiple GPU Queues](https://kompute.cc/overview/advanced-examples.html#parallel-operations)
* [Create your custom Kompute Operations](https://kompute.cc/overview/advanced-examples.html#your-custom-kompute-operation)
* [Implementing logistic regression from scratch](https://kompute.cc/overview/advanced-examples.html#logistic-regression-example)

### End-to-end examples

* [Machine Learning Logistic Regression Implementation](https://towardsdatascience.com/machine-learning-and-data-processing-in-the-gpu-with-vulkan-kompute-c9350e5e5d3a)
* [Parallelizing GPU-intensive Workloads via Multi-Queue Operations](https://towardsdatascience.com/parallelizing-heavy-gpu-workloads-via-multi-queue-operations-50a38b15a1dc)
* [Android NDK Mobile Kompute ML Application](https://towardsdatascience.com/gpu-accelerated-machine-learning-in-your-mobile-applications-using-the-android-ndk-vulkan-kompute-1e9da37b7617)
* [Game Development Kompute ML in Godot Engine](https://towardsdatascience.com/supercharging-game-development-with-gpu-accelerated-ml-using-vulkan-kompute-the-godot-game-engine-4e75a84ea9f0)

## Build Overview

The build system provided uses `cmake`, which allows for cross platform builds.

### Build parameters (cmake)

The recommended approach to build the project is as out-of-source build in the `build` folder. This project comes with a `Makefile` that provides a set of commands that help with developer workflows. You can see some of the commands if you want to add some of the more advanced commands.

For a base build you just have to run:
```
cmake -Bbuild
```

This by default configures without any of the extra build tasks (such as building shaders) and compiles without the optional dependencies. The table below provides more detail.

| Flag                                                  | Description                                                              |
|-------------------------------------------------------|--------------------------------------------------------------------------|
| -DCMAKE_INSTALL_PREFIX="build/src/CMakefiles/Export/" | Enables local installation (which won't require admin privileges)        |
| -DCMAKE_TOOLCHAIN_FILE="..."                          | This is the path for your package manager if you use it such as vcpkg    |
| -DKOMPUTE_OPT_BUILD_TESTS=1                           | Enable if you wish to build and run the tests (must have deps installed. |
| -DKOMPUTE_OPT_BUILD_DOCS=1                            | Enable if you wish to build the docs (must have docs deps installed)     |
| -DKOMPUTE_OPT_BUILD_SINGLE_HEADER=1                   | Option to build the single header file using "quom" utility              |
| -DKOMPUTE_EXTRA_CXX_FLAGS="..."                       | Allows you to pass extra config flags to compiler                        |
| -DKOMPUTE_OPT_INSTALL=0                               | Disables the install step in the cmake file (useful for android build)   |
| -DKOMPUTE_OPT_ANDROID_BUILD=1                         | Enables android build which includes and excludes relevant libraries     |

#### Compile Flags


| Flag                                 | Description                                                                             |
|--------------------------------------|-----------------------------------------------------------------------------------------|
| KOMPUTE_CREATE_PIPELINE_RESULT_VALUE | Ensure the return value of createPipeline is processed as ResultValue instead of Result |
| -DKOMPUTE_VK_API_VERSION="..."       | Sets the default api version to use for vulkan kompute api                              |
| -DKOMPUTE_VK_API_MAJOR_VERSION=1     | Major version to use for the Vulkan API                                                 |
| -DKOMPUTE_VK_API_MINOR_VERSION=1     | Minor version to use for the Vulkan API                                                 |
| -DKOMPUTE_ENABLE_SPDLOG=1            | Enables the build with SPDLOG and FMT dependencies (must be installed)                  |
| -DKOMPUTE_LOG_VERRIDE=1              | Does not define the SPDLOG_<LEVEL> macros if these are to be overridden                 |
| -DSPDLOG_ACTIVE_LEVEL                | The level for the log level on compile level (whether spdlog is enabled)                |
| -DVVK_USE_PLATFORM_ANDROID_KHR       | Flag to enable android imports in kompute (enabled with -DKOMPUTE_OPT_ANDROID_BUILD)    |
| -DRELEASE=1                          | Enable release build (enabled by cmake release build)                                   |
| -DDEBUG=1                            | Enable debug build including debug flags (enabled by cmake debug build)                 |
| -DKOMPUTE_DISABLE_VK_DEBUG_LAYERS    | Disable the debug Vulkan layers, mainly used for android builds                         |

### Dependencies

Given Kompute is expected to be used across a broad range of architectures and hardware, it will be important to make sure we are able to minimise dependencies. 

#### Required dependencies

The only required dependency in the build is Vulkan. More specifically, the header files vulkan.h and vulkan.hpp, which are both part of the Vulkan SDK. If you haven't installed the Vulkan SDK yet, you can [download it here](https://vulkan.lunarg.com/).

#### Optional dependencies

SPDLOG is the preferred logging library, however by default Vulkan Kompute runs without SPDLOG by overriding the macros. It also provides an easy way to override the macros if you prefer to bring your own logging framework. The macro override is the following:

```c++
#ifndef KOMPUTE_LOG_OVERRIDE // Use this if you want to define custom macro overrides
#if KOMPUTE_SPDLOG_ENABLED // Use this if you want to enable SPDLOG
#include <spdlog/spdlog.h>
#endif //KOMPUTE_SPDLOG_ENABLED
// ... Otherwise it adds macros that use std::cout (and only print first element)
#endif // KOMPUTE_LOG_OVERRIDE
```

You can choose to build with or without SPDLOG by using the cmake flag `KOMPUTE_OPT_ENABLE_SPDLOG`.

Finally, remember that you will still need to set both the compile time log level with `SPDLOG_ACTIVE_LEVEL`, and the runtime log level with `spdlog::set_level(spdlog::level::debug);`.


## Kompute Development

We appreciate PRs and Issues. If you want to contribute try checking the "Good first issue" tag, but even using Vulkan Kompute and reporting issues is a great contribution!

### Contributing

#### Dev Dependencies

* Testing
    + GTest
* Documentation
    + Doxygen (with Dot)
    + Sphynx

#### Development

* Follows Mozilla C++ Style Guide https://www-archive.mozilla.org/hacking/mozilla-style-guide.html
    + Uses post-commit hook to run the linter, you can set it up so it runs the linter before commit
    + All dependencies are defined in vcpkg.json 
* Uses cmake as build system, and provides a top level makefile with recommended command
* Uses xxd (or xxd.exe windows 64bit port) to convert shader spirv to header files
* Uses doxygen and sphinx for documentation and autodocs
* Uses vcpkg for finding the dependencies, it's the recommended set up to retrieve the libraries

##### Updating documentation

To update the documentation you will need to:
* Run the gendoxygen target in the build system
* Run the gensphynx target in the build-system 
* Push to github pages with `make push_docs_to_ghpages`

##### Running tests

To run tests you can use the helper top level Makefile

For visual studio you can run

```
make vs_cmake
make vs_run_tests VS_BUILD_TYPE="Release"
```

For unix you can run

```
make mk_cmake MK_BUILD_TYPE="Release"
make mk_run_tests
```

