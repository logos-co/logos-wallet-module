#include "wallet_service.h"
#include <iostream>
#include <sstream>

WalletService::WalletService() {
    std::cout << "WalletService: Initializing..." << std::endl;
}

WalletService::~WalletService() {
    for (auto& [url, handle] : ethClientHandles) {
        GoWSK_ethclient_CloseClient(handle);
    }
    ethClientHandles.clear();
}

unsigned long long WalletService::getOrInitEthClient(const std::string& rpcUrl) {
    auto it = ethClientHandles.find(rpcUrl);
    if (it != ethClientHandles.end()) {
        return it->second;
    }
    char* err = nullptr;
    unsigned long long handle = GoWSK_ethclient_NewClient(
        const_cast<char*>(rpcUrl.c_str()), &err);
    if (handle == 0) {
        std::string emsg = err ? std::string(err) : "unknown error";
        if (err) GoWSK_FreeCString(err);
        std::cerr << "WalletService: Failed to create client: " << rpcUrl << " " << emsg << std::endl;
        return 0;
    }
    ethClientHandles[rpcUrl] = handle;
    return handle;
}

logos::grpc::MethodResult WalletService::ethClientInit(const logos::grpc::MethodParams& params) {
    if (params.size() < 1)
        return {false, "", "ethClientInit requires 1 parameter: rpcUrl"};

    std::string rpcUrl = params.getString(0);
    if (ethClientHandles.count(rpcUrl))
        return {false, "", "Client already initialized: " + rpcUrl};

    char* err = nullptr;
    unsigned long long handle = GoWSK_ethclient_NewClient(
        const_cast<char*>(rpcUrl.c_str()), &err);
    if (handle == 0) {
        std::string emsg = err ? std::string(err) : "unknown error";
        if (err) GoWSK_FreeCString(err);
        return {false, "", "Failed to create client: " + emsg};
    }
    ethClientHandles[rpcUrl] = handle;
    return {true, "true", ""};
}

logos::grpc::MethodResult WalletService::ethClientClose(const logos::grpc::MethodParams& params) {
    if (params.size() < 1)
        return {false, "", "ethClientClose requires 1 parameter: rpcUrl"};

    std::string rpcUrl = params.getString(0);
    auto it = ethClientHandles.find(rpcUrl);
    if (it == ethClientHandles.end())
        return {false, "", "Client not initialized: " + rpcUrl};

    GoWSK_ethclient_CloseClient(it->second);
    ethClientHandles.erase(it);
    return {true, "true", ""};
}

logos::grpc::MethodResult WalletService::ethClientGetClients(const logos::grpc::MethodParams& /*params*/) {
    std::string result = "[";
    bool first = true;
    for (const auto& [url, _] : ethClientHandles) {
        if (!first) result += ",";
        result += "\"" + url + "\"";
        first = false;
    }
    result += "]";
    return {true, result, ""};
}

logos::grpc::MethodResult WalletService::ethClientChainId(const logos::grpc::MethodParams& params) {
    if (params.size() < 1)
        return {false, "", "ethClientChainId requires 1 parameter: rpcUrl"};

    std::string rpcUrl = params.getString(0);
    unsigned long long handle = getOrInitEthClient(rpcUrl);
    if (handle == 0)
        return {false, "", "Failed to get or initialize client: " + rpcUrl};

    char* err = nullptr;
    char* chain = GoWSK_ethclient_ChainID(handle, &err);
    if (!chain) {
        std::string emsg = err ? std::string(err) : "unknown error";
        if (err) GoWSK_FreeCString(err);
        return {false, "", "ChainID error: " + emsg};
    }
    std::string result(chain);
    GoWSK_FreeCString(chain);
    return {true, result, ""};
}

logos::grpc::MethodResult WalletService::ethClientGetBalance(const logos::grpc::MethodParams& params) {
    if (params.size() < 2)
        return {false, "", "ethClientGetBalance requires 2 parameters: rpcUrl, address"};

    std::string rpcUrl = params.getString(0);
    std::string address = params.getString(1);
    unsigned long long handle = getOrInitEthClient(rpcUrl);
    if (handle == 0)
        return {false, "", "Failed to get or initialize client: " + rpcUrl};

    char* err = nullptr;
    char* balance = GoWSK_ethclient_GetBalance(handle,
        const_cast<char*>(address.c_str()), &err);
    if (!balance) {
        std::string emsg = err ? std::string(err) : "unknown error";
        if (err) GoWSK_FreeCString(err);
        return {false, "", "GetBalance error: " + emsg};
    }
    std::string result(balance);
    GoWSK_FreeCString(balance);
    return {true, result, ""};
}

logos::grpc::MethodResult WalletService::ethClientRpcCall(const logos::grpc::MethodParams& params) {
    if (params.size() < 3)
        return {false, "", "ethClientRpcCall requires 3 parameters: rpcUrl, method, paramsJSON"};

    std::string rpcUrl = params.getString(0);
    std::string method = params.getString(1);
    std::string paramsJSON = params.getString(2);
    unsigned long long handle = getOrInitEthClient(rpcUrl);
    if (handle == 0)
        return {false, "", "Failed to get or initialize client: " + rpcUrl};

    char* err = nullptr;
    char* response = GoWSK_ethclient_RPCCall(handle,
        const_cast<char*>(method.c_str()),
        const_cast<char*>(paramsJSON.c_str()), &err);
    if (!response) {
        std::string emsg = err ? std::string(err) : "unknown error";
        if (err) GoWSK_FreeCString(err);
        return {false, "", "RpcCall error: " + emsg};
    }
    std::string result(response);
    GoWSK_FreeCString(response);
    return {true, result, ""};
}

logos::grpc::MethodResult WalletService::txGeneratorCall(
    const logos::grpc::MethodParams& params,
    char* (*goFunc)(char*, char**))
{
    if (params.size() < 1)
        return {false, "", "txGenerator requires 1 parameter: paramsJSON"};

    std::string paramsJSON = params.getString(0);
    char* err = nullptr;
    char* txJson = goFunc(const_cast<char*>(paramsJSON.c_str()), &err);
    if (!txJson) {
        std::string emsg = err ? std::string(err) : "unknown error";
        if (err) GoWSK_FreeCString(err);
        return {false, "", "txGenerator error: " + emsg};
    }
    std::string result(txJson);
    GoWSK_FreeCString(txJson);
    return {true, result, ""};
}

logos::grpc::MethodResult WalletService::transactionJsonToRlp(const logos::grpc::MethodParams& params) {
    if (params.size() < 1)
        return {false, "", "transactionJsonToRlp requires 1 parameter: txJSON"};
    std::string txJSON = params.getString(0);
    char* err = nullptr;
    char* rlpHex = GoWSK_transaction_JSONToRLP(const_cast<char*>(txJSON.c_str()), &err);
    if (!rlpHex) {
        std::string emsg = err ? std::string(err) : "unknown error";
        if (err) GoWSK_FreeCString(err);
        return {false, "", "JSONToRLP error: " + emsg};
    }
    std::string result(rlpHex);
    GoWSK_FreeCString(rlpHex);
    return {true, result, ""};
}

logos::grpc::MethodResult WalletService::transactionRlpToJson(const logos::grpc::MethodParams& params) {
    if (params.size() < 1)
        return {false, "", "transactionRlpToJson requires 1 parameter: rlpHex"};
    std::string rlpHex = params.getString(0);
    char* err = nullptr;
    char* txJson = GoWSK_transaction_RLPToJSON(const_cast<char*>(rlpHex.c_str()), &err);
    if (!txJson) {
        std::string emsg = err ? std::string(err) : "unknown error";
        if (err) GoWSK_FreeCString(err);
        return {false, "", "RLPToJSON error: " + emsg};
    }
    std::string result(txJson);
    GoWSK_FreeCString(txJson);
    return {true, result, ""};
}

logos::grpc::MethodResult WalletService::transactionGetHash(const logos::grpc::MethodParams& params) {
    if (params.size() < 1)
        return {false, "", "transactionGetHash requires 1 parameter: txJSON"};
    std::string txJSON = params.getString(0);
    char* err = nullptr;
    char* hash = GoWSK_transaction_GetHash(const_cast<char*>(txJSON.c_str()), &err);
    if (!hash) {
        std::string emsg = err ? std::string(err) : "unknown error";
        if (err) GoWSK_FreeCString(err);
        return {false, "", "GetHash error: " + emsg};
    }
    std::string result(hash);
    GoWSK_FreeCString(hash);
    return {true, result, ""};
}

logos::grpc::MethodResult WalletService::callMethod(const std::string& method,
                                                      const logos::grpc::MethodParams& params) {
    // EthClient operations
    if (method == "ethClientInit")       return ethClientInit(params);
    if (method == "ethClientClose")      return ethClientClose(params);
    if (method == "ethClientGetClients") return ethClientGetClients(params);
    if (method == "ethClientRpcCall")    return ethClientRpcCall(params);
    if (method == "ethClientChainId")    return ethClientChainId(params);
    if (method == "ethClientGetBalance") return ethClientGetBalance(params);

    // TxGenerator operations
    if (method == "txGeneratorTransferETH")
        return txGeneratorCall(params, &GoWSK_txgenerator_TransferETH);
    if (method == "txGeneratorTransferERC20")
        return txGeneratorCall(params, &GoWSK_txgenerator_TransferERC20);
    if (method == "txGeneratorApproveERC20")
        return txGeneratorCall(params, &GoWSK_txgenerator_ApproveERC20);
    if (method == "txGeneratorTransferFromERC721")
        return txGeneratorCall(params, &GoWSK_txgenerator_TransferFromERC721);
    if (method == "txGeneratorSafeTransferFromERC721")
        return txGeneratorCall(params, &GoWSK_txgenerator_SafeTransferFromERC721);
    if (method == "txGeneratorApproveERC721")
        return txGeneratorCall(params, &GoWSK_txgenerator_ApproveERC721);
    if (method == "txGeneratorSetApprovalForAllERC721")
        return txGeneratorCall(params, &GoWSK_txgenerator_SetApprovalForAllERC721);
    if (method == "txGeneratorTransferERC1155")
        return txGeneratorCall(params, &GoWSK_txgenerator_TransferERC1155);
    if (method == "txGeneratorBatchTransferERC1155")
        return txGeneratorCall(params, &GoWSK_txgenerator_BatchTransferERC1155);
    if (method == "txGeneratorSetApprovalForAllERC1155")
        return txGeneratorCall(params, &GoWSK_txgenerator_SetApprovalForAllERC1155);

    // Transaction operations
    if (method == "transactionJsonToRlp") return transactionJsonToRlp(params);
    if (method == "transactionRlpToJson") return transactionRlpToJson(params);
    if (method == "transactionGetHash")   return transactionGetHash(params);

    return {false, "", "Unknown method: " + method};
}

std::vector<logos::grpc::MethodInfo> WalletService::getMethods() const {
    using MI = logos::grpc::MethodInfo;
    using PI = logos::grpc::ParamInfo;

    return {
        MI{"ethClientInit",       "string", {PI{"rpcUrl", "string"}}},
        MI{"ethClientClose",      "string", {PI{"rpcUrl", "string"}}},
        MI{"ethClientGetClients", "string", {}},
        MI{"ethClientRpcCall",    "string", {PI{"rpcUrl", "string"}, PI{"method", "string"}, PI{"paramsJSON", "string"}}},
        MI{"ethClientChainId",    "string", {PI{"rpcUrl", "string"}}},
        MI{"ethClientGetBalance", "string", {PI{"rpcUrl", "string"}, PI{"address", "string"}}},
        MI{"txGeneratorTransferETH",                "string", {PI{"paramsJSON", "string"}}},
        MI{"txGeneratorTransferERC20",              "string", {PI{"paramsJSON", "string"}}},
        MI{"txGeneratorApproveERC20",               "string", {PI{"paramsJSON", "string"}}},
        MI{"txGeneratorTransferFromERC721",         "string", {PI{"paramsJSON", "string"}}},
        MI{"txGeneratorSafeTransferFromERC721",     "string", {PI{"paramsJSON", "string"}}},
        MI{"txGeneratorApproveERC721",              "string", {PI{"paramsJSON", "string"}}},
        MI{"txGeneratorSetApprovalForAllERC721",    "string", {PI{"paramsJSON", "string"}}},
        MI{"txGeneratorTransferERC1155",            "string", {PI{"paramsJSON", "string"}}},
        MI{"txGeneratorBatchTransferERC1155",       "string", {PI{"paramsJSON", "string"}}},
        MI{"txGeneratorSetApprovalForAllERC1155",   "string", {PI{"paramsJSON", "string"}}},
        MI{"transactionJsonToRlp", "string", {PI{"txJSON", "string"}}},
        MI{"transactionRlpToJson", "string", {PI{"rlpHex", "string"}}},
        MI{"transactionGetHash",   "string", {PI{"txJSON", "string"}}},
    };
}
