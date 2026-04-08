// Unit tests for WalletModuleImpl. Go Wallet SDK calls are mocked via mock_gowalletsdk.cpp.

#include <logos_test.h>
#include "wallet_module_impl.h"

LOGOS_TEST(ethClientInit_succeeds_when_new_client_returns_handle) {
    auto t = LogosTestContext("wallet_module");
    t.mockCFunction("GoWSK_ethclient_NewClient").returns(1);

    WalletModuleImpl impl;
    LOGOS_ASSERT_TRUE(impl.ethClientInit("https://rpc.example"));
    LOGOS_ASSERT(t.cFunctionCalled("GoWSK_ethclient_NewClient"));
}

LOGOS_TEST(ethClientInit_duplicate_url_returns_false) {
    auto t = LogosTestContext("wallet_module");
    t.mockCFunction("GoWSK_ethclient_NewClient").returns(1);

    WalletModuleImpl impl;
    LOGOS_ASSERT_TRUE(impl.ethClientInit("https://rpc.example"));
    LOGOS_ASSERT_FALSE(impl.ethClientInit("https://rpc.example"));
    LOGOS_ASSERT_EQ(t.cFunctionCallCount("GoWSK_ethclient_NewClient"), 1);
}

LOGOS_TEST(ethClientInit_fails_when_new_client_returns_zero) {
    auto t = LogosTestContext("wallet_module");
    t.mockCFunction("GoWSK_ethclient_NewClient").returns(0);

    WalletModuleImpl impl;
    LOGOS_ASSERT_FALSE(impl.ethClientInit("https://rpc.example"));
}

LOGOS_TEST(ethClientClose_fails_without_init) {
    auto t = LogosTestContext("wallet_module");
    WalletModuleImpl impl;
    LOGOS_ASSERT_FALSE(impl.ethClientClose("https://rpc.example"));
}

LOGOS_TEST(ethClientClose_succeeds_after_init) {
    auto t = LogosTestContext("wallet_module");
    t.mockCFunction("GoWSK_ethclient_NewClient").returns(1);

    WalletModuleImpl impl;
    LOGOS_ASSERT_TRUE(impl.ethClientInit("https://rpc.example"));
    LOGOS_ASSERT_TRUE(impl.ethClientClose("https://rpc.example"));
    LOGOS_ASSERT(t.cFunctionCalled("GoWSK_ethclient_CloseClient"));
}

LOGOS_TEST(ethClientGetClients_lists_initialized_urls) {
    auto t = LogosTestContext("wallet_module");
    t.mockCFunction("GoWSK_ethclient_NewClient").returns(1);

    WalletModuleImpl impl;
    LOGOS_ASSERT_EQ(static_cast<int>(impl.ethClientGetClients().size()), 0);
    impl.ethClientInit("https://a");
    impl.ethClientInit("https://b");
    auto clients = impl.ethClientGetClients();
    LOGOS_ASSERT_EQ(static_cast<int>(clients.size()), 2);
}

LOGOS_TEST(ethClientChainId_returns_mocked_string) {
    auto t = LogosTestContext("wallet_module");
    t.mockCFunction("GoWSK_ethclient_NewClient").returns(1);
    t.mockCFunction("GoWSK_ethclient_ChainID").returns("0x539");

    WalletModuleImpl impl;
    impl.ethClientInit("https://rpc.example");
    std::string id = impl.ethClientChainId("https://rpc.example");
    LOGOS_ASSERT_EQ(id, std::string("0x539"));
    LOGOS_ASSERT(t.cFunctionCalled("GoWSK_ethclient_ChainID"));
}

LOGOS_TEST(ethClientGetBalance_returns_mocked_string) {
    auto t = LogosTestContext("wallet_module");
    t.mockCFunction("GoWSK_ethclient_NewClient").returns(1);
    t.mockCFunction("GoWSK_ethclient_GetBalance").returns("0xdeadbeef");

    WalletModuleImpl impl;
    impl.ethClientInit("https://rpc.example");
    std::string bal = impl.ethClientGetBalance("https://rpc.example", "0xabc");
    LOGOS_ASSERT_EQ(bal, std::string("0xdeadbeef"));
}

LOGOS_TEST(ethClientRpcCall_returns_mocked_string) {
    auto t = LogosTestContext("wallet_module");
    t.mockCFunction("GoWSK_ethclient_NewClient").returns(1);
    t.mockCFunction("GoWSK_ethclient_RPCCall").returns(R"({"result":"ok"})");

    WalletModuleImpl impl;
    impl.ethClientInit("https://rpc.example");
    std::string out = impl.ethClientRpcCall("https://rpc.example", "eth_blockNumber", "[]");
    LOGOS_ASSERT_EQ(out, std::string(R"({"result":"ok"})"));
}

LOGOS_TEST(transactionJsonToRlp_returns_mocked_hex) {
    auto t = LogosTestContext("wallet_module");
    t.mockCFunction("GoWSK_transaction_JSONToRLP").returns("0xc0");

    WalletModuleImpl impl;
    std::string rlp = impl.transactionJsonToRlp("{}");
    LOGOS_ASSERT_EQ(rlp, std::string("0xc0"));
}

LOGOS_TEST(transactionGetHash_returns_mocked_hash) {
    auto t = LogosTestContext("wallet_module");
    t.mockCFunction("GoWSK_transaction_GetHash").returns(
        "0xbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");

    WalletModuleImpl impl;
    std::string h = impl.transactionGetHash("{}");
    LOGOS_ASSERT_EQ(h, std::string("0xbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"));
}

LOGOS_TEST(txGeneratorTransferETH_returns_mocked_json) {
    auto t = LogosTestContext("wallet_module");
    t.mockCFunction("GoWSK_txgenerator_TransferETH").returns(R"({"to":"0x1"})");

    WalletModuleImpl impl;
    std::string tx = impl.txGeneratorTransferETH("{}");
    LOGOS_ASSERT_EQ(tx, std::string(R"({"to":"0x1"})"));
}
