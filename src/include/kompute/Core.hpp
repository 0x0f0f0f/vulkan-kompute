#pragma once

//#define VK_NO_PROTOTYPES 1
//#undef VK_NO_PROTOTYPES
//#undef DEBUG
//#ifndef RELEASE
//#define RELEASE 1
//#endif
//#define USE_DEBUG_EXTENTIONS

#ifdef VK_USE_PLATFORM_ANDROID_KHR
//#include <android/log.h>
//#include <android_native_app_glue.h>
#include <kompute_vk_ndk_wrapper.hpp>

// VK_NO_PROTOTYPES required before vulkan import but after wrapper.hpp
#undef VK_NO_PROTOTYPES
#define DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif

#include <vulkan/vulkan.hpp>

// SPDLOG_ACTIVE_LEVEL must be defined before spdlog.h import
#if !defined(SPDLOG_ACTIVE_LEVEL)
#if DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#else
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#endif
#endif

#ifndef KOMPUTE_LOG_OVERRIDE
#if KOMPUTE_ENABLE_SPDLOG
#include <spdlog/spdlog.h>
#else
#include <iostream>
#if SPDLOG_ACTIVE_LEVEL > 1
#define SPDLOG_DEBUG(message, ...)
#else
#define SPDLOG_DEBUG(message, ...)                                             \
    std::cout << "DEBUG: " << message << std::endl
#endif // SPDLOG_ACTIVE_LEVEL > 1
#if SPDLOG_ACTIVE_LEVEL > 2
#define SPDLOG_INFO(message, ...)
#else
#define SPDLOG_INFO(message, ...) std::cout << "INFO: " << message << std::endl
#endif // SPDLOG_ACTIVE_LEVEL > 2
#if SPDLOG_ACTIVE_LEVEL > 3
#define SPDLOG_WARN(message, ...)
#else
#define SPDLOG_WARN(message, ...)                                              \
    std::cout << "WARNING: " << message << std::endl
#endif // SPDLOG_ACTIVE_LEVEL > 3
#if SPDLOG_ACTIVE_LEVEL > 4
#define SPDLOG_ERROR(message, ...)
#else
#define SPDLOG_ERROR(message, ...)                                             \
    std::cout << "ERROR: " << message << std::endl
#endif // SPDLOG_ACTIVE_LEVEL > 4
#endif // KOMPUTE_SPDLOG_ENABLED
#endif // KOMPUTE_LOG_OVERRIDE
