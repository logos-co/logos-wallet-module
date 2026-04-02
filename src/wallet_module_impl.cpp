#include "wallet_module_impl.h"
#include <cstdio>

WalletModuleImpl::WalletModuleImpl()
{
    fprintf(stderr, "WalletModuleImpl: Initializing...\n");
}

WalletModuleImpl::~WalletModuleImpl()
{
    for (auto& pair : ethClientHandles) {
        GoWSK_ethclient_CloseClient(pair.second);
    }
    ethClientHandles.clear();
}

bool WalletModuleImpl::ethClientInit(const std::string& rpcUrl)
{
    fprintf(stderr, "WalletModuleImpl::ethClientInit called\n");

    if (ethClientHandles.count(rpcUrl) > 0) {
        fprintf(stderr, "WalletModuleImpl: Client already initialized: %s\n", rpcUrl.c_str());
        return false;
    }

    char* err = nullptr;
    unsigned long long ethClientHandle = GoWSK_ethclient_NewClient(const_cast<char*>(rpcUrl.c_str()), &err);
    if (ethClientHandle == 0) {
        std::string emsg = err ? std::string(err) : "unknown error";
        if (err) GoWSK_FreeCString(err);
        fprintf(stderr, "WalletModuleImpl: Failed to create client: %s %s\n", rpcUrl.c_str(), emsg.c_str());
        return false;
    }
    ethClientHandles[rpcUrl] = ethClientHandle;
    return true;
}

bool WalletModuleImpl::ethClientClose(const std::string& rpcUrl)
{
    fprintf(stderr, "WalletModuleImpl::ethClientClose %s\n", rpcUrl.c_str());
    if (ethClientHandles.count(rpcUrl) == 0) {
        fprintf(stderr, "WalletModuleImpl: Client not initialized: %s\n", rpcUrl.c_str());
        return false;
    }
    unsigned long long ethClientHandle = ethClientHandles[rpcUrl];
    GoWSK_ethclient_CloseClient(ethClientHandle);
    ethClientHandles.erase(rpcUrl);
    return true;
}

std::vector<std::string> WalletModuleImpl::ethClientGetClients()
{
    fprintf(stderr, "WalletModuleImpl::ethClientGetClients\n");
    std::vector<std::string> clients;
    for (const auto& pair : ethClientHandles) {
        clients.push_back(pair.first);
    }
    return clients;
}

unsigned long long WalletModuleImpl::getOrInitEthClient(const std::string& rpcUrl)
{
    if (ethClientHandles.count(rpcUrl) == 0) {
        if (!ethClientInit(rpcUrl)) {
            return 0;
        }
    }
    return ethClientHandles[rpcUrl];
}

std::string WalletModuleImpl::ethClientChainId(const std::string& rpcUrl)
{
    fprintf(stderr, "WalletModuleImpl::ethClientChainId %s\n", rpcUrl.c_str());
    unsigned long long ethClientHandle = getOrInitEthClient(rpcUrl);
    if (ethClientHandle == 0) {
        fprintf(stderr, "WalletModuleImpl: Failed to get or initialize client: %s\n", rpcUrl.c_str());
        return {};
    }
    char* err = nullptr;
    char* chain = GoWSK_ethclient_ChainID(ethClientHandle, &err);
    if (chain == nullptr) {
        std::string emsg = err ? std::string(err) : "unknown error";
        if (err) GoWSK_FreeCString(err);
        fprintf(stderr, "WalletModuleImpl: ChainID error: %s\n", emsg.c_str());
        return {};
    }
    std::string result(chain);
    GoWSK_FreeCString(chain);
    return result;
}

std::string WalletModuleImpl::ethClientGetBalance(const std::string& rpcUrl, const std::string& address)
{
    fprintf(stderr, "WalletModuleImpl::ethClientGetBalance %s %s\n", rpcUrl.c_str(), address.c_str());
    unsigned long long ethClientHandle = getOrInitEthClient(rpcUrl);
    if (ethClientHandle == 0) {
        fprintf(stderr, "WalletModuleImpl: Failed to get or initialize client: %s\n", rpcUrl.c_str());
        return {};
    }
    char* err = nullptr;
    char* balance = GoWSK_ethclient_GetBalance(ethClientHandle, const_cast<char*>(address.c_str()), &err);
    if (balance == nullptr) {
        std::string emsg = err ? std::string(err) : "unknown error";
        if (err) GoWSK_FreeCString(err);
        fprintf(stderr, "WalletModuleImpl: GetBalance error: %s\n", emsg.c_str());
        return {};
    }
    std::string result(balance);
    GoWSK_FreeCString(balance);
    return result;
}

std::string WalletModuleImpl::ethClientRpcCall(const std::string& rpcUrl, const std::string& method, const std::string& paramsJSON)
{
    fprintf(stderr, "WalletModuleImpl::ethClientRpcCall %s %s %s\n", rpcUrl.c_str(), method.c_str(), paramsJSON.c_str());
    unsigned long long ethClientHandle = getOrInitEthClient(rpcUrl);
    if (ethClientHandle == 0) {
        fprintf(stderr, "WalletModuleImpl: Failed to get or initialize client: %s\n", rpcUrl.c_str());
        return {};
    }
    char* err = nullptr;
    char* response = GoWSK_ethclient_RPCCall(ethClientHandle, const_cast<char*>(method.c_str()), const_cast<char*>(paramsJSON.c_str()), &err);
    if (response == nullptr) {
        std::string emsg = err ? std::string(err) : "unknown error";
        if (err) GoWSK_FreeCString(err);
        fprintf(stderr, "WalletModuleImpl: RpcCall error: %s\n", emsg.c_str());
        return {};
    }
    std::string result(response);
    GoWSK_FreeCString(response);
    return result;
}

std::string WalletModuleImpl::transactionJsonToRlp(const std::string& txJSON)
{
    fprintf(stderr, "WalletModuleImpl::transactionJsonToRlp %s\n", txJSON.c_str());
    char* err = nullptr;
    char* rlpHex = GoWSK_transaction_JSONToRLP(const_cast<char*>(txJSON.c_str()), &err);
    if (rlpHex == nullptr) {
        std::string emsg = err ? std::string(err) : "unknown error";
        if (err) GoWSK_FreeCString(err);
        fprintf(stderr, "WalletModuleImpl: JSONToRLP error: %s\n", emsg.c_str());
        return {};
    }
    std::string result(rlpHex);
    GoWSK_FreeCString(rlpHex);
    return result;
}

std::string WalletModuleImpl::transactionRlpToJson(const std::string& rlpHex)
{
    fprintf(stderr, "WalletModuleImpl::transactionRlpToJson %s\n", rlpHex.c_str());
    char* err = nullptr;
    char* txJson = GoWSK_transaction_RLPToJSON(const_cast<char*>(rlpHex.c_str()), &err);
    if (txJson == nullptr) {
        std::string emsg = err ? std::string(err) : "unknown error";
        if (err) GoWSK_FreeCString(err);
        fprintf(stderr, "WalletModuleImpl: RLPToJSON error: %s\n", emsg.c_str());
        return {};
    }
    std::string result(txJson);
    GoWSK_FreeCString(txJson);
    return result;
}

std::string WalletModuleImpl::transactionGetHash(const std::string& txJSON)
{
    fprintf(stderr, "WalletModuleImpl::transactionGetHash %s\n", txJSON.c_str());
    char* err = nullptr;
    char* hash = GoWSK_transaction_GetHash(const_cast<char*>(txJSON.c_str()), &err);
    if (hash == nullptr) {
        std::string emsg = err ? std::string(err) : "unknown error";
        if (err) GoWSK_FreeCString(err);
        fprintf(stderr, "WalletModuleImpl::transactionGetHash error: %s\n", emsg.c_str());
        return {};
    }
    std::string result(hash);
    GoWSK_FreeCString(hash);
    return result;
}

namespace {
    using TxGeneratorFunc = char* (*)(char*, char**);

    std::string callGoTxGenerator(const std::string& paramsJSON,
                                  const char* logPrefix,
                                  TxGeneratorFunc func)
    {
        fprintf(stderr, "%s %s\n", logPrefix, paramsJSON.c_str());
        char* err = nullptr;
        char* txJson = func(const_cast<char*>(paramsJSON.c_str()), &err);
        if (txJson == nullptr) {
            std::string emsg = err ? std::string(err) : "unknown error";
            if (err) GoWSK_FreeCString(err);
            fprintf(stderr, "%s error: %s\n", logPrefix, emsg.c_str());
            return {};
        }
        std::string result(txJson);
        GoWSK_FreeCString(txJson);
        return result;
    }
}

std::string WalletModuleImpl::txGeneratorTransferETH(const std::string& paramsJSON)
{
    return callGoTxGenerator(
        paramsJSON,
        "WalletModuleImpl::txGeneratorTransferETH",
        &GoWSK_txgenerator_TransferETH
    );
}

std::string WalletModuleImpl::txGeneratorTransferERC20(const std::string& paramsJSON)
{
    return callGoTxGenerator(
        paramsJSON,
        "WalletModuleImpl::txGeneratorTransferERC20",
        &GoWSK_txgenerator_TransferERC20
    );
}

std::string WalletModuleImpl::txGeneratorApproveERC20(const std::string& paramsJSON)
{
    return callGoTxGenerator(
        paramsJSON,
        "WalletModuleImpl::txGeneratorApproveERC20",
        &GoWSK_txgenerator_ApproveERC20
    );
}

std::string WalletModuleImpl::txGeneratorTransferFromERC721(const std::string& paramsJSON)
{
    return callGoTxGenerator(
        paramsJSON,
        "WalletModuleImpl::txGeneratorTransferFromERC721",
        &GoWSK_txgenerator_TransferFromERC721
    );
}

std::string WalletModuleImpl::txGeneratorSafeTransferFromERC721(const std::string& paramsJSON)
{
    return callGoTxGenerator(
        paramsJSON,
        "WalletModuleImpl::txGeneratorSafeTransferFromERC721",
        &GoWSK_txgenerator_SafeTransferFromERC721
    );
}

std::string WalletModuleImpl::txGeneratorApproveERC721(const std::string& paramsJSON)
{
    return callGoTxGenerator(
        paramsJSON,
        "WalletModuleImpl::txGeneratorApproveERC721",
        &GoWSK_txgenerator_ApproveERC721
    );
}

std::string WalletModuleImpl::txGeneratorSetApprovalForAllERC721(const std::string& paramsJSON)
{
    return callGoTxGenerator(
        paramsJSON,
        "WalletModuleImpl::txGeneratorSetApprovalForAllERC721",
        &GoWSK_txgenerator_SetApprovalForAllERC721
    );
}

std::string WalletModuleImpl::txGeneratorTransferERC1155(const std::string& paramsJSON)
{
    return callGoTxGenerator(
        paramsJSON,
        "WalletModuleImpl::txGeneratorTransferERC1155",
        &GoWSK_txgenerator_TransferERC1155
    );
}

std::string WalletModuleImpl::txGeneratorBatchTransferERC1155(const std::string& paramsJSON)
{
    return callGoTxGenerator(
        paramsJSON,
        "WalletModuleImpl::txGeneratorBatchTransferERC1155",
        &GoWSK_txgenerator_BatchTransferERC1155
    );
}

std::string WalletModuleImpl::txGeneratorSetApprovalForAllERC1155(const std::string& paramsJSON)
{
    return callGoTxGenerator(
        paramsJSON,
        "WalletModuleImpl::txGeneratorSetApprovalForAllERC1155",
        &GoWSK_txgenerator_SetApprovalForAllERC1155
    );
}
