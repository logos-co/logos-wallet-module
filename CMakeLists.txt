cmake_minimum_required(VERSION 3.14)
project(LogosWalletModulePlugin LANGUAGES CXX)

# Include the Logos Module CMake helper
if(DEFINED ENV{LOGOS_MODULE_BUILDER_ROOT})
    include($ENV{LOGOS_MODULE_BUILDER_ROOT}/cmake/LogosModule.cmake)
else()
    message(FATAL_ERROR "LogosModule.cmake not found. Set LOGOS_MODULE_BUILDER_ROOT.")
endif()

logos_module(
    NAME wallet_module
    SOURCES
        src/wallet_module_impl.h
        src/wallet_module_impl.cpp
    FIND_PACKAGES
        Threads
    LINK_LIBRARIES
        Threads::Threads
)

# Link Abseil if available
find_package(absl QUIET)
if(absl_FOUND)
    target_link_libraries(wallet_module_module_plugin PRIVATE
        absl::base absl::strings absl::log absl::check)
endif()
