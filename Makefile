
# This makefile is optimized to be run from WSL and to interact with the 
# Windows host as there are limitations when building GPU programs. This
# makefile contains the commands for interacting with the visual studio
# build via command line for faster iterations, as the intention is to 
# support other editors (optimised for vim). There are also commands that
# support the builds for linux-native compilations and these are the commands
# starting with mk_.

VCPKG_WIN_PATH ?= "C:\\Users\\axsau\\Programming\\lib\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake"
VCPKG_UNIX_PATH ?= "/c/Users/axsau/Programming/lib/vcpkg/scripts/buildsystems/vcpkg.cmake"

ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
	CMAKE_BIN ?= "C:\Program Files\CMake\bin\cmake.exe"
	SCMP_BIN="C:\\VulkanSDK\\1.2.141.2\\Bin32\\glslangValidator.exe"
	MSBUILD_BIN ?= "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe"
else
	CLANG_FORMAT_BIN ?= "/home/alejandro/Programming/lib/clang+llvm-10.0.0-x86_64-linux-gnu-ubuntu-18.04/bin/clang-format"
	CMAKE_BIN ?= "/c/Program Files/CMake/bin/cmake.exe"
	SCMP_BIN ?= "/c/VulkanSDK/1.2.141.2/Bin32/glslangValidator.exe"
	MSBUILD_BIN ?= "/c/Program Files (x86)/Microsoft Visual Studio/2019/Community/MSBuild/Current/Bin/MSBuild.exe"
endif


####### Main Target Rules #######

push_docs_to_ghpages:
	GIT_DEPLOY_DIR="build/docs/sphinx/" \
		GIT_DEPLOY_BRANCH="gh-pages" \
		GIT_DEPLOY_REPO="origin" \
			./scripts/push_folder_to_branch.sh

####### CMAKE quickstart commands #######

clean_cmake:
	rm -rf build/

####### Visual studio build shortcut commands #######

mk_cmake:
	$(CMAKE_BIN) \
		-Bbuild \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_TOOLCHAIN_FILE=$(VCPKG_UNIX_PATH) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
		-G "Unix Makefiles"

mk_build_all:
	make -C build/

mk_build_docs:
	make -C build/ docs

mk_build_kompute:
	make -C build/ kompute

mk_build_tests:
	make -C build/ test_kompute

mk_run_docs: mk_build_docs
	(cd build/docs/sphinx && python2.7 -m SimpleHTTPServer)

mk_run_tests: mk_build_tests
	./build/test/test_kompute


####### Visual studio build shortcut commands #######

vs_cmake:
	$(CMAKE_BIN) \
		-Bbuild \
		-DCMAKE_TOOLCHAIN_FILE=$(VCPKG_WIN_PATH) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-G "Visual Studio 16 2019"

vs_build_all:
	$(MSBUILD_BIN) build/kompute.sln

vs_build_docs:
	$(MSBUILD_BIN) build/docs/gendocsall.vcxproj

vs_build_kompute:
	$(MSBUILD_BIN) build/src/kompute.vcxproj

vs_build_tests:
	$(MSBUILD_BIN) build/test/test_kompute.vcxproj

vs_run_docs: vs_build_docs
	(cd build/docs/sphinx && python2.7 -m SimpleHTTPServer)

vs_run_tests: vs_build_tests
	./build/test/Debug/test_kompute.exe

####### General project commands #######

install_python_reqs:
	python -m pip install -r scripts/requirements.txt

build_shaders:
	python scripts/convert_shaders.py \
		--shader-path shaders/glsl \
		--shader-binary $(SCMP_BIN) \
		--header-path src/include/kompute/shaders/ \
		-v

docker_seldon_run:
	docker run \
		--rm -it \
		-p 5000:5000 \
		axsauze/vulkan-seldon:0.1 \
		seldon-core-microservice Model REST

docker_seldon_run_gpu:
	docker run \
		--rm -it \
		--gpus all \
		-p 5000:5000 \
		axsauze/vulkan-seldon:0.1 \
		seldon-core-microservice Model REST

docker_seldon_push:
	docker push axsauze/vulkan-seldon:0.1

docker_seldon_build: 
	docker build . -f Dockerfile.seldon -t axsauze/vulkan-seldon:0.1

docker_vulkan_build:
	docker build . -t axsauze/vulkan-sum:0.1 

push_vulkan_docker:
	docker push axsauze/vulkan-sum:0.1

build_single_header:
	quom \
		--include_directory \
		"src/include/" \
		"single_include/AggregateHeaders.cpp" \
		"single_include/kompute/Kompute.hpp"

format:
	$(CLANG_FORMAT_BIN) -i -style="{BasedOnStyle: mozilla, IndentWidth: 4}" src/*.cpp src/include/kompute/*.hpp

clean:
	find src -name "*gch" -exec rm {} \; || "No ghc files"
	rm ./bin/main.exe || echo "No main.exe"

run:
	./bin/main.exe;

