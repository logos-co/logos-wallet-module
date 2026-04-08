cmake_minimum_required(VERSION 3.14)
project(WalletModuleTests LANGUAGES CXX)

include(LogosTest)

logos_test(
    NAME wallet_module_tests
    MODULE_SOURCES
        ../src/wallet_module_impl.cpp
    TEST_SOURCES
        main.cpp
        test_wallet.cpp
    MOCK_C_SOURCES
        mocks/mock_gowalletsdk.cpp
    EXTRA_INCLUDES
        stubs
)

logos_find_go_static_archive(GOWALLETSDK_LIB gowalletsdk)
if(GOWALLETSDK_LIB)
    message(STATUS "[WalletTests] libgowalletsdk found — building integration tests")
    logos_test(
        NAME wallet_module_integration_tests
        MODULE_SOURCES
            ../src/wallet_module_impl.cpp
        TEST_SOURCES
            main.cpp
            test_wallet_integration.cpp
        EXTRA_INCLUDES
            ../lib
        LINK_GO_STATIC_ARCHIVE
            ${GOWALLETSDK_LIB}
    )
else()
    message(STATUS "[WalletTests] libgowalletsdk not found in ../lib — skipping integration tests")
endif()
