#pragma once

#include <logos_grpc/grpc_module_base.h>
#include <map>
#include <string>

extern "C" {
    #include "lib/libgowalletsdk.h"
}

class WalletService : public logos::grpc::GrpcModuleBase {
public:
    WalletService();
    ~WalletService();

    std::string name() const override { return "wallet_module"; }
    std::string version() const override { return "1.0.0"; }
    logos::grpc::MethodResult callMethod(const std::string& method,
                                          const logos::grpc::MethodParams& params) override;
    std::vector<logos::grpc::MethodInfo> getMethods() const override;

private:
    std::map<std::string, unsigned long long> ethClientHandles;

    unsigned long long getOrInitEthClient(const std::string& rpcUrl);

    // EthClient operations
    logos::grpc::MethodResult ethClientInit(const logos::grpc::MethodParams& params);
    logos::grpc::MethodResult ethClientClose(const logos::grpc::MethodParams& params);
    logos::grpc::MethodResult ethClientGetClients(const logos::grpc::MethodParams& params);
    logos::grpc::MethodResult ethClientRpcCall(const logos::grpc::MethodParams& params);
    logos::grpc::MethodResult ethClientChainId(const logos::grpc::MethodParams& params);
    logos::grpc::MethodResult ethClientGetBalance(const logos::grpc::MethodParams& params);

    // TxGenerator operations
    logos::grpc::MethodResult txGeneratorCall(const logos::grpc::MethodParams& params,
                                               char* (*goFunc)(char*, char**));

    // Transaction operations
    logos::grpc::MethodResult transactionJsonToRlp(const logos::grpc::MethodParams& params);
    logos::grpc::MethodResult transactionRlpToJson(const logos::grpc::MethodParams& params);
    logos::grpc::MethodResult transactionGetHash(const logos::grpc::MethodParams& params);
};
