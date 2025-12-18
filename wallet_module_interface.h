#pragma once

#include <QtCore/QObject>
#include "interface.h"

class WalletModuleInterface : public PluginInterface
{
public:
    virtual ~WalletModuleInterface() {}

    // EthClient operations
    Q_INVOKABLE virtual bool initWallet(const QString &configJson) = 0;
    Q_INVOKABLE virtual QString rpcCall(const QString &rpcUrl, const QString &method, const QString &paramsJSON) = 0;
    Q_INVOKABLE virtual QString chainId(const QString &rpcUrl) = 0;
    Q_INVOKABLE virtual QString getEthBalance(const QString &rpcUrl, const QString &address) = 0;
    Q_INVOKABLE virtual QString getErc20Balances(const QString &rpcUrl, const QString &address, const QStringList &tokenAddresses) = 0;

    // TxGenerator operations
    Q_INVOKABLE virtual QString txGeneratorTransferETH(const QString &paramsJSON) = 0;
    Q_INVOKABLE virtual QString txGeneratorTransferERC20(const QString &paramsJSON) = 0;
    Q_INVOKABLE virtual QString txGeneratorApproveERC20(const QString &paramsJSON) = 0;
    Q_INVOKABLE virtual QString txGeneratorTransferFromERC721(const QString &paramsJSON) = 0;
    Q_INVOKABLE virtual QString txGeneratorSafeTransferFromERC721(const QString &paramsJSON) = 0;
    Q_INVOKABLE virtual QString txGeneratorApproveERC721(const QString &paramsJSON) = 0;
    Q_INVOKABLE virtual QString txGeneratorSetApprovalForAllERC721(const QString &paramsJSON) = 0;
    Q_INVOKABLE virtual QString txGeneratorTransferERC1155(const QString &paramsJSON) = 0;
    Q_INVOKABLE virtual QString txGeneratorBatchTransferERC1155(const QString &paramsJSON) = 0;
    Q_INVOKABLE virtual QString txGeneratorSetApprovalForAllERC1155(const QString &paramsJSON) = 0;

    // Transaction operations
    Q_INVOKABLE virtual QString transactionJsonToRlp(const QString &txJSON) = 0;
    Q_INVOKABLE virtual QString transactionRlpToJson(const QString &rlpHex) = 0;
    Q_INVOKABLE virtual QString transactionGetHash(const QString &txJSON) = 0;

signals:
    void eventResponse(const QString& eventName, const QVariantList& data);
};

#define WalletModuleInterface_iid "org.logos.WalletModuleInterface"
Q_DECLARE_INTERFACE(WalletModuleInterface, WalletModuleInterface_iid)


