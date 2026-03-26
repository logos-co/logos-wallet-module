cmake_minimum_required(VERSION 3.14)
project(LogosWalletModulePlugin LANGUAGES CXX)

# Include the Logos Module CMake helper
if(DEFINED ENV{LOGOS_MODULE_BUILDER_ROOT})
    include($ENV{LOGOS_MODULE_BUILDER_ROOT}/cmake/LogosModule.cmake)
else()
    message(FATAL_ERROR "LogosModule.cmake not found. Set LOGOS_MODULE_BUILDER_ROOT.")
endif()

# Define the module
logos_module(
    NAME wallet_module
    SOURCES
        src/wallet_module_interface.h
        src/wallet_module_plugin.h
        src/wallet_module_plugin.cpp
    FIND_PACKAGES
        Threads
    LINK_LIBRARIES
        Threads::Threads
)

# go-wallet-sdk static library (requires whole-archive for Go runtime initialization)
find_library(LIBGOWALLETSDK
    NAMES libgowalletsdk.a libgowalletsdk.lib
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib
    NO_DEFAULT_PATH
)

if(LIBGOWALLETSDK)
    target_link_libraries(wallet_module_module_plugin PRIVATE ${LIBGOWALLETSDK})
    if(APPLE)
        target_link_options(wallet_module_module_plugin PRIVATE -Wl,-force_load ${LIBGOWALLETSDK})
        target_link_libraries(wallet_module_module_plugin PUBLIC "-framework CoreFoundation")
        target_link_libraries(wallet_module_module_plugin PUBLIC "-framework Security")
    else()
        target_link_options(wallet_module_module_plugin PRIVATE
            -Wl,--whole-archive ${LIBGOWALLETSDK} -Wl,--no-whole-archive)
    endif()
endif()

# Link Abseil if available
find_package(absl QUIET)
if(absl_FOUND)
    target_link_libraries(wallet_module_module_plugin PRIVATE
        absl::base absl::strings absl::log absl::check)
endif()
