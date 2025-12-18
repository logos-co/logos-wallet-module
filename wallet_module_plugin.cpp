#include "wallet_module_plugin.h"
#include <QDebug>
#include <QVariantList>
#include <QDateTime>

WalletModulePlugin::WalletModulePlugin() : walletHandle(0)
{
    qDebug() << "WalletModulePlugin: Initializing...";
}

WalletModulePlugin::~WalletModulePlugin()
{
    if (logosAPI) {
        delete logosAPI;
        logosAPI = nullptr;
    }
    if (walletHandle != 0) {
        GoWSK_ethclient_CloseClient(walletHandle);
        walletHandle = 0;
    }
}

void WalletModulePlugin::initLogos(LogosAPI* logosAPIInstance) {
    if (logosAPI) {
        delete logosAPI;
    }
    logosAPI = logosAPIInstance;
}

void WalletModulePlugin::simple_callback(int callerRet, const char* msg, size_t len, void* userData)
{
    Q_UNUSED(userData);
    qDebug() << "WalletModulePlugin::simple_callback ret:" << callerRet;
    if (msg && len > 0) {
        QString message = QString::fromUtf8(msg, static_cast<int>(len));
        qDebug() << "WalletModulePlugin::simple_callback message:" << message;
    }
}

bool WalletModulePlugin::initWallet(const QString &configJson)
{
    qDebug() << "WalletModulePlugin::initWallet called";
    Q_UNUSED(configJson);

    const char* url = "https://ethereum-rpc.publicnode.com";
    char* err = nullptr;
    walletHandle = GoWSK_ethclient_NewClient((char*)url, &err);
    if (walletHandle == 0) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: Failed to create client:" << emsg;
        return false;
    }
    qDebug() << "WalletModulePlugin: Client created: handle=" << (qulonglong)walletHandle;
    return true;
}

QString WalletModulePlugin::chainId(const QString &rpcUrl)
{
    qDebug() << "WalletModulePlugin::chainId" << rpcUrl;
    Q_UNUSED(rpcUrl);
    if (walletHandle == 0) {
        if (!initWallet(QString())) {
            return QString();
        }
    }
    char* err = nullptr;
    char* chain = GoWSK_ethclient_ChainID(walletHandle, &err);
    if (chain == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ChainID error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(chain);
    GoWSK_FreeCString(chain);
    return result;
}

QString WalletModulePlugin::getEthBalance(const QString &rpcUrl, const QString &address)
{
    qDebug() << "WalletModulePlugin::getEthBalance" << rpcUrl << address;
    Q_UNUSED(rpcUrl);
    if (walletHandle == 0) {
        if (!initWallet(QString())) {
            return QString();
        }
    }
    QByteArray addrUtf8 = address.toUtf8();
    char* err = nullptr;
    char* balance = GoWSK_ethclient_GetBalance(walletHandle, addrUtf8.data(), &err);
    if (balance == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: GetBalance error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(balance);
    GoWSK_FreeCString(balance);
    return result;
}

QString WalletModulePlugin::getErc20Balances(const QString &rpcUrl, const QString &address, const QStringList &tokenAddresses)
{
    qDebug() << "WalletModulePlugin::getErc20Balances" << rpcUrl << address << tokenAddresses;
    Q_UNUSED(rpcUrl);
    Q_UNUSED(address);
    Q_UNUSED(tokenAddresses);
    // Not yet implemented in cshared; stub for future
    return QString();
}

QString WalletModulePlugin::rpcCall(const QString &rpcUrl, const QString &method, const QString &paramsJSON)
{
    qDebug() << "WalletModulePlugin::rpcCall" << rpcUrl << method << paramsJSON;
    Q_UNUSED(rpcUrl);
    if (walletHandle == 0) {
        if (!initWallet(QString())) {
            return QString();
        }
    }
    QByteArray methodUtf8 = method.toUtf8();
    QByteArray paramsJsonUtf8 = paramsJSON.toUtf8();
    char* err = nullptr;
    char* response = GoWSK_ethclient_RPCCall(walletHandle, methodUtf8.data(), paramsJsonUtf8.data(), &err);
    if (response == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: RpcCall error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(response);
    GoWSK_FreeCString(response);
    return result;
}

QString WalletModulePlugin::transactionJsonToRlp(const QString &txJSON)
{
    qDebug() << "WalletModulePlugin::transactionJsonToRlp" << txJSON;
    QByteArray txJsonUtf8 = txJSON.toUtf8();
    char* err = nullptr;
    char* rlpHex = GoWSK_transaction_JSONToRLP(txJsonUtf8.data(), &err);
    if (rlpHex == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModuleTransaction: JSONToRLP error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(rlpHex);
    GoWSK_FreeCString(rlpHex);
    return result;
}

QString WalletModulePlugin::transactionRlpToJson(const QString &rlpHex)
{
    qDebug() << "WalletModulePlugin::transactionRlpToJson" << rlpHex;
    QByteArray rlpHexUtf8 = rlpHex.toUtf8();
    char* err = nullptr;
    char* txJson = GoWSK_transaction_RLPToJSON(rlpHexUtf8.data(), &err);
    if (txJson == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModuleTransaction: RLPToJSON error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(txJson);
    GoWSK_FreeCString(txJson);
    return result;
}

QString WalletModulePlugin::transactionGetHash(const QString &txJSON)
{
    qDebug() << "WalletModulePlugin::transactionGetHash" << txJSON;
    QByteArray txJsonUtf8 = txJSON.toUtf8();
    char* err = nullptr;
    char* hash = GoWSK_transaction_GetHash(txJsonUtf8.data(), &err);
    if (hash == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin::transactionGetHash error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(hash);
    GoWSK_FreeCString(hash);
    return result;
}

QString WalletModulePlugin::txGeneratorTransferETH(const QString &paramsJSON)
{
    qDebug() << "WalletModulePlugin::txGeneratorTransferETH" << paramsJSON;
    QByteArray paramsJsonUtf8 = paramsJSON.toUtf8();
    char* err = nullptr;
    char* txJson = GoWSK_txgenerator_TransferETH(paramsJsonUtf8.data(), &err);
    if (txJson == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin::txGeneratorTransferETH error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(txJson);
    GoWSK_FreeCString(txJson);
    return result;
}

QString WalletModulePlugin::txGeneratorTransferERC20(const QString &paramsJSON)
{
    qDebug() << "WalletModulePlugin::txGeneratorTransferERC20" << paramsJSON;
    QByteArray paramsJsonUtf8 = paramsJSON.toUtf8();
    char* err = nullptr;
    char* txJson = GoWSK_txgenerator_TransferERC20(paramsJsonUtf8.data(), &err);
    if (txJson == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin::txGeneratorTransferERC20 error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(txJson);
    GoWSK_FreeCString(txJson);
    return result;
}

QString WalletModulePlugin::txGeneratorApproveERC20(const QString &paramsJSON)
{
    qDebug() << "WalletModulePlugin::txGeneratorApproveERC20" << paramsJSON;
    QByteArray paramsJsonUtf8 = paramsJSON.toUtf8();
    char* err = nullptr;
    char* txJson = GoWSK_txgenerator_ApproveERC20(paramsJsonUtf8.data(), &err);
    if (txJson == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin::txGeneratorApproveERC20 error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(txJson);
    GoWSK_FreeCString(txJson);
    return result;
}

QString WalletModulePlugin::txGeneratorTransferFromERC721(const QString &paramsJSON)
{
    qDebug() << "WalletModulePlugin::txGeneratorTransferFromERC721" << paramsJSON;
    QByteArray paramsJsonUtf8 = paramsJSON.toUtf8();
    char* err = nullptr;
    char* txJson = GoWSK_txgenerator_TransferFromERC721(paramsJsonUtf8.data(), &err);
    if (txJson == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin::txGeneratorTransferFromERC721 error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(txJson);
    GoWSK_FreeCString(txJson);
    return result;
}

QString WalletModulePlugin::txGeneratorSafeTransferFromERC721(const QString &paramsJSON)
{
    qDebug() << "WalletModulePlugin::txGeneratorSafeTransferFromERC721" << paramsJSON;
    QByteArray paramsJsonUtf8 = paramsJSON.toUtf8();
    char* err = nullptr;
    char* txJson = GoWSK_txgenerator_SafeTransferFromERC721(paramsJsonUtf8.data(), &err);
    if (txJson == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin::txGeneratorSafeTransferFromERC721 error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(txJson);
    GoWSK_FreeCString(txJson);
    return result;
}

QString WalletModulePlugin::txGeneratorApproveERC721(const QString &paramsJSON)
{
    qDebug() << "WalletModulePlugin::txGeneratorApproveERC721" << paramsJSON;
    QByteArray paramsJsonUtf8 = paramsJSON.toUtf8();
    char* err = nullptr;
    char* txJson = GoWSK_txgenerator_ApproveERC721(paramsJsonUtf8.data(), &err);
    if (txJson == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin::txGeneratorApproveERC721 error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(txJson);
    GoWSK_FreeCString(txJson);
    return result;
}

QString WalletModulePlugin::txGeneratorSetApprovalForAllERC721(const QString &paramsJSON)
{
    qDebug() << "WalletModulePlugin::txGeneratorSetApprovalForAllERC721" << paramsJSON;
    QByteArray paramsJsonUtf8 = paramsJSON.toUtf8();
    char* err = nullptr;
    char* txJson = GoWSK_txgenerator_SetApprovalForAllERC721(paramsJsonUtf8.data(), &err);
    if (txJson == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin::txGeneratorSetApprovalForAllERC721 error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(txJson);
    GoWSK_FreeCString(txJson);
    return result;
}

QString WalletModulePlugin::txGeneratorTransferERC1155(const QString &paramsJSON)
{
    qDebug() << "WalletModulePlugin::txGeneratorTransferERC1155" << paramsJSON;
    QByteArray paramsJsonUtf8 = paramsJSON.toUtf8();
    char* err = nullptr;
    char* txJson = GoWSK_txgenerator_TransferERC1155(paramsJsonUtf8.data(), &err);
    if (txJson == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin::txGeneratorTransferERC1155 error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(txJson);
    GoWSK_FreeCString(txJson);
    return result;
}

QString WalletModulePlugin::txGeneratorBatchTransferERC1155(const QString &paramsJSON)
{
    qDebug() << "WalletModulePlugin::txGeneratorBatchTransferERC1155" << paramsJSON;
    QByteArray paramsJsonUtf8 = paramsJSON.toUtf8();
    char* err = nullptr;
    char* txJson = GoWSK_txgenerator_BatchTransferERC1155(paramsJsonUtf8.data(), &err);
    if (txJson == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin::txGeneratorBatchTransferERC1155 error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(txJson);
    GoWSK_FreeCString(txJson);
    return result;
}

QString WalletModulePlugin::txGeneratorSetApprovalForAllERC1155(const QString &paramsJSON)
{
    qDebug() << "WalletModulePlugin::txGeneratorSetApprovalForAllERC1155" << paramsJSON;
    QByteArray paramsJsonUtf8 = paramsJSON.toUtf8();
    char* err = nullptr;
    char* txJson = GoWSK_txgenerator_SetApprovalForAllERC1155(paramsJsonUtf8.data(), &err);
    if (txJson == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin::txGeneratorSetApprovalForAllERC1155 error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(txJson);
    GoWSK_FreeCString(txJson);
    return result;
}
