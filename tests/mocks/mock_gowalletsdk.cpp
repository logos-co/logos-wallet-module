// Mock implementation of libgowalletsdk for wallet_module_impl unit tests.
// Function names match those called from wallet_module_impl.cpp.

#include <logos_clib_mock.h>
#include <cstdlib>
#include <cstring>

extern "C" {

typedef unsigned long long GoWSKHandle;

void GoWSK_FreeCString(char* str) {
    LOGOS_CMOCK_RECORD("GoWSK_FreeCString");
    if (str) free(str);
}

GoWSKHandle GoWSK_ethclient_NewClient(char* rpcUrl, char** error) {
    LOGOS_CMOCK_RECORD("GoWSK_ethclient_NewClient");
    (void)rpcUrl;
    if (error) *error = nullptr;
    return static_cast<GoWSKHandle>(
        LogosCMockStore::instance().getReturn<int>("GoWSK_ethclient_NewClient"));
}

void GoWSK_ethclient_CloseClient(GoWSKHandle handle) {
    LOGOS_CMOCK_RECORD("GoWSK_ethclient_CloseClient");
    (void)handle;
}

static char* mockDupString(const char* key, const char* fallback) {
    const char* ret = LOGOS_CMOCK_RETURN_STRING(key);
    return strdup(ret ? ret : fallback);
}

char* GoWSK_ethclient_ChainID(GoWSKHandle handle, char** error) {
    LOGOS_CMOCK_RECORD("GoWSK_ethclient_ChainID");
    (void)handle;
    if (error) *error = nullptr;
    return mockDupString("GoWSK_ethclient_ChainID", "0x1");
}

char* GoWSK_ethclient_GetBalance(GoWSKHandle handle, char* address, char** error) {
    LOGOS_CMOCK_RECORD("GoWSK_ethclient_GetBalance");
    (void)handle;
    (void)address;
    if (error) *error = nullptr;
    return mockDupString("GoWSK_ethclient_GetBalance", "0x0");
}

char* GoWSK_ethclient_RPCCall(GoWSKHandle handle, char* method, char* paramsJSON, char** error) {
    LOGOS_CMOCK_RECORD("GoWSK_ethclient_RPCCall");
    (void)handle;
    (void)method;
    (void)paramsJSON;
    if (error) *error = nullptr;
    return mockDupString("GoWSK_ethclient_RPCCall", "{}");
}

char* GoWSK_transaction_JSONToRLP(char* txJSON, char** error) {
    LOGOS_CMOCK_RECORD("GoWSK_transaction_JSONToRLP");
    (void)txJSON;
    if (error) *error = nullptr;
    return mockDupString("GoWSK_transaction_JSONToRLP", "0x00");
}

char* GoWSK_transaction_RLPToJSON(char* rlpHex, char** error) {
    LOGOS_CMOCK_RECORD("GoWSK_transaction_RLPToJSON");
    (void)rlpHex;
    if (error) *error = nullptr;
    return mockDupString("GoWSK_transaction_RLPToJSON", "{}");
}

char* GoWSK_transaction_GetHash(char* txJSON, char** error) {
    LOGOS_CMOCK_RECORD("GoWSK_transaction_GetHash");
    (void)txJSON;
    if (error) *error = nullptr;
    return mockDupString("GoWSK_transaction_GetHash",
        "0xaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
}

#define MOCK_TXGEN(name) \
    char* name(char* paramsJSON, char** error) { \
        LOGOS_CMOCK_RECORD(#name); \
        (void)paramsJSON; \
        if (error) *error = nullptr; \
        return mockDupString(#name, "{\"mock\":true}"); \
    }

MOCK_TXGEN(GoWSK_txgenerator_TransferETH)
MOCK_TXGEN(GoWSK_txgenerator_TransferERC20)
MOCK_TXGEN(GoWSK_txgenerator_ApproveERC20)
MOCK_TXGEN(GoWSK_txgenerator_TransferFromERC721)
MOCK_TXGEN(GoWSK_txgenerator_SafeTransferFromERC721)
MOCK_TXGEN(GoWSK_txgenerator_ApproveERC721)
MOCK_TXGEN(GoWSK_txgenerator_SetApprovalForAllERC721)
MOCK_TXGEN(GoWSK_txgenerator_TransferERC1155)
MOCK_TXGEN(GoWSK_txgenerator_BatchTransferERC1155)
MOCK_TXGEN(GoWSK_txgenerator_SetApprovalForAllERC1155)

} // extern "C"
