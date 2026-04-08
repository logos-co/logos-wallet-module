#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

extern "C" {
    #include "lib/libgowalletsdk.h"
}

class WalletModuleImpl {
public:
    WalletModuleImpl();
    ~WalletModuleImpl();

    // EthClient operations
    bool ethClientInit(const std::string& rpcUrl);
    bool ethClientClose(const std::string& rpcUrl);
    std::vector<std::string> ethClientGetClients();
    std::string ethClientRpcCall(const std::string& rpcUrl, const std::string& method, const std::string& paramsJSON);
    std::string ethClientChainId(const std::string& rpcUrl);
    std::string ethClientGetBalance(const std::string& rpcUrl, const std::string& address);

    // TxGenerator operations
    std::string txGeneratorTransferETH(const std::string& paramsJSON);
    std::string txGeneratorTransferERC20(const std::string& paramsJSON);
    std::string txGeneratorApproveERC20(const std::string& paramsJSON);
    std::string txGeneratorTransferFromERC721(const std::string& paramsJSON);
    std::string txGeneratorSafeTransferFromERC721(const std::string& paramsJSON);
    std::string txGeneratorApproveERC721(const std::string& paramsJSON);
    std::string txGeneratorSetApprovalForAllERC721(const std::string& paramsJSON);
    std::string txGeneratorTransferERC1155(const std::string& paramsJSON);
    std::string txGeneratorBatchTransferERC1155(const std::string& paramsJSON);
    std::string txGeneratorSetApprovalForAllERC1155(const std::string& paramsJSON);

    // Transaction operations
    std::string transactionJsonToRlp(const std::string& txJSON);
    std::string transactionRlpToJson(const std::string& rlpHex);
    std::string transactionGetHash(const std::string& txJSON);

private:
    std::map<std::string, unsigned long long> ethClientHandles;
    unsigned long long getOrInitEthClient(const std::string& rpcUrl);
};
