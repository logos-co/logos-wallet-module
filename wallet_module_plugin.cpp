#include "wallet_module_plugin.h"
#include <QDebug>
#include <QVariantList>
#include <QDateTime>

WalletModulePlugin::WalletModulePlugin()
{
    qDebug() << "WalletModulePlugin: Initializing...";
}

WalletModulePlugin::~WalletModulePlugin()
{
    if (logosAPI) {
        delete logosAPI;
        logosAPI = nullptr;
    }
    foreach (unsigned long long handle, ethClientHandles.values()) {
        GoWSK_ethclient_CloseClient(handle);
    }
    ethClientHandles.clear();
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

bool WalletModulePlugin::ethClientInit(const QString &rpcUrl)
{
    qDebug() << "WalletModulePlugin::initWallet called";

    if (ethClientHandles.contains(rpcUrl)) {
        qWarning() << "WalletModulePlugin: Client already initialized:" << rpcUrl;
        return false;
    }

    char* err = nullptr;
    unsigned long long ethClientHandle = GoWSK_ethclient_NewClient((char*)rpcUrl.toUtf8().data(), &err);
    if (ethClientHandle == 0) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: Failed to create client:" << rpcUrl << emsg;
        return false;
    }
    ethClientHandles[rpcUrl] = ethClientHandle;
    return true;
}

bool WalletModulePlugin::ethClientClose(const QString &rpcUrl)
{
    qDebug() << "WalletModulePlugin::ethClientClose" << rpcUrl;
    if (!ethClientHandles.contains(rpcUrl)) {
        qWarning() << "WalletModulePlugin: Client not initialized:" << rpcUrl;
        return false;
    }
    unsigned long long ethClientHandle = ethClientHandles[rpcUrl];
    GoWSK_ethclient_CloseClient(ethClientHandle);
    ethClientHandles.remove(rpcUrl);
    return true;
}

QStringList WalletModulePlugin::ethClientGetClients()
{
    qDebug() << "WalletModulePlugin::ethClientGetClients";
    return ethClientHandles.keys();
}

unsigned long long WalletModulePlugin::getOrInitEthClient(const QString &rpcUrl)
{
    if (!ethClientHandles.contains(rpcUrl)) {
        if (!ethClientInit(rpcUrl)) {
            return 0;
        }
    }
    return ethClientHandles[rpcUrl];
}

QString WalletModulePlugin::ethClientChainId(const QString &rpcUrl)
{
    qDebug() << "WalletModulePlugin::ethClientChainId" << rpcUrl;
    unsigned long long ethClientHandle = getOrInitEthClient(rpcUrl);
    if (ethClientHandle == 0) {
        qWarning() << "WalletModulePlugin: Failed to get or initialize client:" << rpcUrl;
        return QString();
    }
    char* err = nullptr;
    char* chain = GoWSK_ethclient_ChainID(ethClientHandle, &err);
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

QString WalletModulePlugin::ethClientGetBalance(const QString &rpcUrl, const QString &address)
{
    qDebug() << "WalletModulePlugin::getEthBalance" << rpcUrl << address;
    unsigned long long ethClientHandle = getOrInitEthClient(rpcUrl);
    if (ethClientHandle == 0) {
        qWarning() << "WalletModulePlugin: Failed to get or initialize client:" << rpcUrl;
        return QString();
    }
    QByteArray addrUtf8 = address.toUtf8();
    char* err = nullptr;
    char* balance = GoWSK_ethclient_GetBalance(ethClientHandle, addrUtf8.data(), &err);
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

QString WalletModulePlugin::ethClientRpcCall(const QString &rpcUrl, const QString &method, const QString &paramsJSON)
{
    qDebug() << "WalletModulePlugin::ethClientRpcCall" << rpcUrl << method << paramsJSON;
    Q_UNUSED(rpcUrl);
    unsigned long long ethClientHandle = getOrInitEthClient(rpcUrl);
    if (ethClientHandle == 0) {
        qWarning() << "WalletModulePlugin: Failed to get or initialize client:" << rpcUrl;
        return QString();
    }
    QByteArray methodUtf8 = method.toUtf8();
    QByteArray paramsJsonUtf8 = paramsJSON.toUtf8();
    char* err = nullptr;
    char* response = GoWSK_ethclient_RPCCall(ethClientHandle, methodUtf8.data(), paramsJsonUtf8.data(), &err);
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
