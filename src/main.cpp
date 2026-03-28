#include "wallet_service.h"
#include <logos_grpc/grpc_main.h>

int main(int argc, char* argv[]) {
    WalletService service;
    return logos::grpc::runGrpcModule(service, argc, argv);
}
