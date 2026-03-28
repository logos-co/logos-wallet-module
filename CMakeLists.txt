cmake_minimum_required(VERSION 3.14)
project(LogosWalletModuleGrpc LANGUAGES CXX)

# Include the Logos gRPC Module CMake helper
if(DEFINED ENV{LOGOS_MODULE_BUILDER_ROOT})
    include($ENV{LOGOS_MODULE_BUILDER_ROOT}/cmake/LogosGrpcModule.cmake)
elseif(DEFINED ENV{LOGOS_GRPC_PLUGIN_ROOT})
    include($ENV{LOGOS_GRPC_PLUGIN_ROOT}/share/logos-plugin-grpc/cmake/LogosGrpcModule.cmake)
else()
    message(FATAL_ERROR "LogosGrpcModule.cmake not found. Set LOGOS_MODULE_BUILDER_ROOT or LOGOS_GRPC_PLUGIN_ROOT.")
endif()

# Define the gRPC module
logos_grpc_module(
    NAME wallet_module
    SOURCES
        src/wallet_service.h
        src/wallet_service.cpp
        src/main.cpp
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
    target_link_libraries(wallet_module_grpc PRIVATE ${LIBGOWALLETSDK})
    if(APPLE)
        target_link_options(wallet_module_grpc PRIVATE -Wl,-force_load ${LIBGOWALLETSDK})
        target_link_libraries(wallet_module_grpc PUBLIC "-framework CoreFoundation")
        target_link_libraries(wallet_module_grpc PUBLIC "-framework Security")
    else()
        target_link_options(wallet_module_grpc PRIVATE
            -Wl,--whole-archive ${LIBGOWALLETSDK} -Wl,--no-whole-archive)
    endif()
endif()

# Link Abseil if available
find_package(absl QUIET)
if(absl_FOUND)
    target_link_libraries(wallet_module_grpc PRIVATE
        absl::base absl::strings absl::log absl::check)
endif()
