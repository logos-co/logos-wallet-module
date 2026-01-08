#pragma once

#include <QtCore/QObject>
#include "wallet_module_interface.h"
#include "logos_api.h"
#include "logos_api_client.h"

extern "C" {
    #include "lib/libgowalletsdk.h"
}

class WalletModulePlugin : public QObject, public WalletModuleInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID WalletModuleInterface_iid FILE "metadata.json")
    Q_INTERFACES(WalletModuleInterface PluginInterface)

public:
    WalletModulePlugin();
    ~WalletModulePlugin();

    QString name() const override { return "wallet_module"; }
    QString version() const override { return "1.0.0"; }
    Q_INVOKABLE void initLogos(LogosAPI* logosAPIInstance);

    // EthClient operations
    Q_INVOKABLE bool ethClientInit(const QString &rpcUrl) override;
    Q_INVOKABLE bool ethClientClose(const QString &rpcUrl) override;
    Q_INVOKABLE QStringList ethClientGetClients() override;
    Q_INVOKABLE QString ethClientRpcCall(const QString &rpcUrl, const QString &method, const QString &paramsJSON) override;
    Q_INVOKABLE QString ethClientChainId(const QString &rpcUrl) override;
    Q_INVOKABLE QString ethClientGetBalance(const QString &rpcUrl, const QString &address) override;

    // TxGenerator operations
    Q_INVOKABLE QString txGeneratorTransferETH(const QString &paramsJSON) override;
    Q_INVOKABLE QString txGeneratorTransferERC20(const QString &paramsJSON) override;
    Q_INVOKABLE QString txGeneratorApproveERC20(const QString &paramsJSON) override;
    Q_INVOKABLE QString txGeneratorTransferFromERC721(const QString &paramsJSON) override;
    Q_INVOKABLE QString txGeneratorSafeTransferFromERC721(const QString &paramsJSON) override;
    Q_INVOKABLE QString txGeneratorApproveERC721(const QString &paramsJSON) override;
    Q_INVOKABLE QString txGeneratorSetApprovalForAllERC721(const QString &paramsJSON) override;
    Q_INVOKABLE QString txGeneratorTransferERC1155(const QString &paramsJSON) override;
    Q_INVOKABLE QString txGeneratorBatchTransferERC1155(const QString &paramsJSON) override;
    Q_INVOKABLE QString txGeneratorSetApprovalForAllERC1155(const QString &paramsJSON) override;

    // Transaction operations
    Q_INVOKABLE QString transactionJsonToRlp(const QString &txJSON) override;
    Q_INVOKABLE QString transactionRlpToJson(const QString &rlpHex) override;
    Q_INVOKABLE QString transactionGetHash(const QString &txJSON) override;

signals:
    void eventResponse(const QString& eventName, const QVariantList& data);

private:
    QMap<QString, unsigned long long> ethClientHandles;

    static void simple_callback(int callerRet, const char* msg, size_t len, void* userData);

    unsigned long long getOrInitEthClient(const QString &rpcUrl);
};


