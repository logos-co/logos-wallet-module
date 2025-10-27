cmake_minimum_required(VERSION 3.14)
project(LogosWalletModulePlugin LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)

option(LOGOS_WALLET_MODULE_USE_VENDOR "Force use of vendored Logos dependencies" OFF)

# Allow override from environment or command line
if(NOT DEFINED LOGOS_LIBLOGOS_ROOT)
    set(_parent_liblogos "${CMAKE_SOURCE_DIR}/../logos-liblogos")
    set(_use_vendor ${LOGOS_WALLET_MODULE_USE_VENDOR})
    if(NOT _use_vendor)
        if(NOT EXISTS "${_parent_liblogos}/interface.h")
            set(_use_vendor ON)
        endif()
    endif()
    if(_use_vendor)
        set(LOGOS_LIBLOGOS_ROOT "${CMAKE_SOURCE_DIR}/vendor/logos-liblogos")
    else()
        set(LOGOS_LIBLOGOS_ROOT "${_parent_liblogos}")
    endif()
endif()

if(NOT DEFINED LOGOS_CPP_SDK_ROOT)
    set(_parent_cpp_sdk "${CMAKE_SOURCE_DIR}/../logos-cpp-sdk")
    set(_use_vendor ${LOGOS_WALLET_MODULE_USE_VENDOR})
    if(NOT _use_vendor)
        if(NOT EXISTS "${_parent_cpp_sdk}/cpp/logos_api.h")
            set(_use_vendor ON)
        endif()
    endif()
    if(_use_vendor)
        set(LOGOS_CPP_SDK_ROOT "${CMAKE_SOURCE_DIR}/vendor/logos-cpp-sdk")
    else()
        set(LOGOS_CPP_SDK_ROOT "${_parent_cpp_sdk}")
    endif()
endif()

# Check if dependencies are available (support both source and installed layouts)
set(_liblogos_found FALSE)
if(EXISTS "${LOGOS_LIBLOGOS_ROOT}/interface.h")
    set(_liblogos_found TRUE)
    set(_liblogos_is_source TRUE)
elseif(EXISTS "${LOGOS_LIBLOGOS_ROOT}/include/interface.h")
    set(_liblogos_found TRUE)
    set(_liblogos_is_source FALSE)
endif()

set(_cpp_sdk_found FALSE)
if(EXISTS "${LOGOS_CPP_SDK_ROOT}/cpp/logos_api.h")
    set(_cpp_sdk_found TRUE)
    set(_cpp_sdk_is_source TRUE)
elseif(EXISTS "${LOGOS_CPP_SDK_ROOT}/include/cpp/logos_api.h")
    set(_cpp_sdk_found TRUE)
    set(_cpp_sdk_is_source FALSE)
endif()

if(NOT _liblogos_found)
    message(FATAL_ERROR "logos-liblogos not found at ${LOGOS_LIBLOGOS_ROOT}. "
                        "Set LOGOS_LIBLOGOS_ROOT or run git submodule update --init --recursive.")
endif()

if(NOT _cpp_sdk_found)
    message(FATAL_ERROR "logos-cpp-sdk not found at ${LOGOS_CPP_SDK_ROOT}. "
                        "Set LOGOS_CPP_SDK_ROOT or run git submodule update --init --recursive.")
endif()

# Root that contains the vendored dependencies (logos-core checkout or script vendor directory)
get_filename_component(LOGOS_DEPS_ROOT "${LOGOS_CPP_SDK_ROOT}" DIRECTORY)

# Find Qt RemoteObjects (needed for LogosAPI)
if(NOT DEFINED QT_VERSION_MAJOR)
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core RemoteObjects)
    if(Qt6_FOUND)
        set(QT_VERSION_MAJOR 6)
    else()
        set(QT_VERSION_MAJOR 5)
    endif()
endif()
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core RemoteObjects)

# Run Logos C++ generator on metadata before compilation (only for source layout)
set(METADATA_JSON "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json")

# Only run generator for source layout - nix builds already have generated files
if(_cpp_sdk_is_source)
    # Source layout: build and run the generator
    # Generate into build directory
    set(PLUGINS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/generated_code")
    set(CPP_GENERATOR_BUILD_DIR "${LOGOS_DEPS_ROOT}/build/cpp-generator")
    set(CPP_GENERATOR "${CPP_GENERATOR_BUILD_DIR}/bin/logos-cpp-generator")
    
    if(NOT TARGET cpp_generator_build)
        add_custom_target(cpp_generator_build
            COMMAND bash "${LOGOS_CPP_SDK_ROOT}/cpp-generator/compile.sh"
            WORKING_DIRECTORY "${LOGOS_DEPS_ROOT}"
            COMMENT "Building logos-cpp-generator via ${LOGOS_CPP_SDK_ROOT}/cpp-generator/compile.sh"
            VERBATIM
        )
    endif()
    
    add_custom_target(run_cpp_generator_wallet
        COMMAND "${CPP_GENERATOR}" --metadata "${METADATA_JSON}" --general-only --output-dir "${PLUGINS_OUTPUT_DIR}"
        WORKING_DIRECTORY "${LOGOS_DEPS_ROOT}"
        COMMENT "Running logos-cpp-generator on ${METADATA_JSON} with output-dir ${PLUGINS_OUTPUT_DIR}"
        VERBATIM
    )
    add_dependencies(run_cpp_generator_wallet cpp_generator_build)
else()
    # Installed/nix layout: files are already in source tree from lib.nix preConfigure
    # lib.nix creates ./generated_code in source directory before CMake runs
    set(PLUGINS_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated_code")
endif()

# Find required packages
find_package(Threads REQUIRED)
find_package(absl QUIET)
find_library(LIBWALLET_PATH
    NAMES libgowalletsdk.so libgowalletsdk.dylib libgowalletsdk.dll
    PATHS 
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    NO_DEFAULT_PATH
)

if(NOT LIBWALLET_PATH)
    message(WARNING "Wallet C library not found in lib/. Build it via build_wallet_lib.sh before linking.")
endif()

# Plugin sources
set(PLUGIN_SOURCES
    wallet_module_plugin.cpp
    wallet_module_plugin.h
    wallet_module_interface.h
)

# Add liblogos interface header
if(_liblogos_is_source)
    list(APPEND PLUGIN_SOURCES ${LOGOS_LIBLOGOS_ROOT}/interface.h)
else()
    list(APPEND PLUGIN_SOURCES ${LOGOS_LIBLOGOS_ROOT}/include/interface.h)
endif()

# Add SDK sources (only if source layout, installed layout uses the library)
if(_cpp_sdk_is_source)
    list(APPEND PLUGIN_SOURCES
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_api.cpp
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_api.h
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_api_client.cpp
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_api_client.h
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_api_consumer.cpp
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_api_consumer.h
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_api_provider.cpp
        ${LOGOS_CPP_SDK_ROOT}/cpp/logos_api_provider.h
        ${LOGOS_CPP_SDK_ROOT}/cpp/token_manager.cpp
        ${LOGOS_CPP_SDK_ROOT}/cpp/token_manager.h
        ${LOGOS_CPP_SDK_ROOT}/cpp/module_proxy.cpp
        ${LOGOS_CPP_SDK_ROOT}/cpp/module_proxy.h
    )
    # Add generated logos_sdk.cpp - will be generated by run_cpp_generator_wallet
    list(APPEND PLUGIN_SOURCES ${PLUGINS_OUTPUT_DIR}/logos_sdk.cpp)
    # Mark as generated so CMake knows to wait for it
    set_source_files_properties(
        ${PLUGINS_OUTPUT_DIR}/logos_sdk.cpp
        PROPERTIES GENERATED TRUE
    )
else()
    # For installed/nix layout, logos_sdk.cpp is already in ${PLUGINS_OUTPUT_DIR}
    # (generated by nix build preConfigure phase in lib.nix before CMake runs)
    list(APPEND PLUGIN_SOURCES ${PLUGINS_OUTPUT_DIR}/logos_sdk.cpp)
endif()

# Create the plugin library
add_library(wallet_module_plugin SHARED ${PLUGIN_SOURCES})

# Set output name without lib prefix and with correct name for generator
set_target_properties(wallet_module_plugin PROPERTIES
    PREFIX ""
    OUTPUT_NAME "wallet_module_plugin")

# Ensure generator runs before building the plugin (only for source layout)
if(_cpp_sdk_is_source)
    add_dependencies(wallet_module_plugin run_cpp_generator_wallet)
endif()

# Link against libraries
target_link_libraries(wallet_module_plugin PRIVATE 
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::RemoteObjects
    Threads::Threads
)

# Link SDK library if using installed layout
if(NOT _cpp_sdk_is_source)
    find_library(LOGOS_SDK_LIB logos_sdk PATHS ${LOGOS_CPP_SDK_ROOT}/lib NO_DEFAULT_PATH REQUIRED)
    target_link_libraries(wallet_module_plugin PRIVATE ${LOGOS_SDK_LIB})
endif()

# Link against Abseil libraries if found
if(absl_FOUND)
    target_link_libraries(wallet_module_plugin PRIVATE
        absl::base
        absl::strings
        absl::log
        absl::check
    )
endif()

# Link the wallet C library if present
if(LIBWALLET_PATH)
    target_link_libraries(wallet_module_plugin PRIVATE ${LIBWALLET_PATH})
endif()

# Include directories
target_include_directories(wallet_module_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PLUGINS_OUTPUT_DIR}
)

# Add include directories based on layout type
if(_liblogos_is_source)
    target_include_directories(wallet_module_plugin PRIVATE ${LOGOS_LIBLOGOS_ROOT})
else()
    target_include_directories(wallet_module_plugin PRIVATE ${LOGOS_LIBLOGOS_ROOT}/include)
endif()

if(_cpp_sdk_is_source)
    target_include_directories(wallet_module_plugin PRIVATE 
        ${LOGOS_CPP_SDK_ROOT}/cpp
        ${LOGOS_CPP_SDK_ROOT}/cpp/generated
    )
else()
    # For nix builds, also include the include subdirectory
    # (lib.nix moves header files to ./generated_code/include/)
    target_include_directories(wallet_module_plugin PRIVATE 
        ${LOGOS_CPP_SDK_ROOT}/include
        ${LOGOS_CPP_SDK_ROOT}/include/cpp
        ${LOGOS_CPP_SDK_ROOT}/include/core
        ${PLUGINS_OUTPUT_DIR}/include
    )
endif()

# Set common properties for both platforms
set_target_properties(wallet_module_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/modules"  # For Windows .dll
    BUILD_WITH_INSTALL_RPATH TRUE
    SKIP_BUILD_RPATH FALSE)

if(APPLE)
    # macOS specific settings
    set_target_properties(wallet_module_plugin PROPERTIES
        INSTALL_RPATH "@loader_path"
        INSTALL_NAME_DIR "@rpath"
        BUILD_WITH_INSTALL_NAME_DIR TRUE)

    # If libgowalletsdk exists, copy and fix install names
    if(LIBWALLET_PATH)
        add_custom_command(TARGET wallet_module_plugin PRE_LINK
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${LIBWALLET_PATH}
            ${CMAKE_BINARY_DIR}/modules/libgowalletsdk.dylib
            COMMENT "Copying libgowalletsdk to modules directory"
            VERBATIM
        )

        add_custom_command(TARGET wallet_module_plugin POST_BUILD
            COMMAND install_name_tool -id "@rpath/wallet_module_plugin.dylib" $<TARGET_FILE:wallet_module_plugin>
            COMMAND ${CMAKE_COMMAND} -DWALLET_LIB_PATH=${CMAKE_BINARY_DIR}/modules/libgowalletsdk.dylib -DPLUGIN_PATH=${CMAKE_BINARY_DIR}/modules/wallet_module_plugin.dylib -P ${CMAKE_CURRENT_SOURCE_DIR}/fix_wallet_install_name.cmake
            COMMENT "Updating library paths for macOS"
        )
    else()
        add_custom_command(TARGET wallet_module_plugin POST_BUILD
            COMMAND install_name_tool -id "@rpath/wallet_module_plugin.dylib" $<TARGET_FILE:wallet_module_plugin>
            COMMENT "Updating library paths for macOS (wallet library not found)"
        )
    endif()
else()
    # Linux specific settings
    set_target_properties(wallet_module_plugin PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        INSTALL_RPATH_USE_LINK_PATH FALSE)

    # First, copy libgowallet.so to the modules directory if present
    if(LIBWALLET_PATH)
        add_custom_command(TARGET wallet_module_plugin PRE_LINK
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${LIBWALLET_PATH}
            ${CMAKE_BINARY_DIR}/modules/libgowalletsdk.so
            COMMENT "Copying libgowalletsdk.so to modules directory"
        )
    endif()
endif()

install(TARGETS wallet_module_plugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/logos/modules
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/logos/modules
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/logos/modules
)

install(FILES ${METADATA_JSON}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/logos-wallet-module
)

install(DIRECTORY "${PLUGINS_OUTPUT_DIR}/"
    DESTINATION ${CMAKE_INSTALL_DATADIR}/logos-wallet-module/generated
    OPTIONAL
)

# Install the wallet library if it exists
if(LIBWALLET_PATH)
    install(FILES ${LIBWALLET_PATH}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/logos/modules
    )
endif()

# Print status messages
message(STATUS "Wallet Plugin configured successfully")


