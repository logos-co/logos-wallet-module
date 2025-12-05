#include "wallet_module_plugin.h"
#include <QDebug>
#include <QVariantList>
#include <QDateTime>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonValue>
#include <QtCore/QJsonParseError>

WalletModulePlugin::WalletModulePlugin() : walletHandle(0), keystoreHandle(0), extkeystoreHandle(0)
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
    if (keystoreHandle != 0) {
        GoWSK_accounts_keystore_CloseKeyStore(keystoreHandle);
        keystoreHandle = 0;
    }
    if (extkeystoreHandle != 0) {
        GoWSK_accounts_extkeystore_CloseKeyStore(extkeystoreHandle);
        extkeystoreHandle = 0;
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

QString WalletModulePlugin::rpcCall(const QString &rpcUrl, const QString &method, const QString &params)
{
    qDebug() << "WalletModulePlugin::rpcCall" << rpcUrl << method << params;
    Q_UNUSED(rpcUrl);
    if (walletHandle == 0) {
        if (!initWallet(QString())) {
            return QString();
        }
    }
    QString requestJson = QString("{\"method\":\"%1\",\"params\":%2}").arg(method, params.isEmpty() ? "[]" : params);
    QByteArray requestJsonBytes = requestJson.toUtf8();
    char* err = nullptr;
    char* response = GoWSK_ethclient_RPCCall(walletHandle, requestJsonBytes.data(), &err);
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

// Keystore operations
bool WalletModulePlugin::initKeystore(const QString &dir)
{
    qDebug() << "WalletModulePlugin::initKeystore" << dir;
    if (keystoreHandle != 0) {
        GoWSK_accounts_keystore_CloseKeyStore(keystoreHandle);
    }
    QByteArray dirUtf8 = dir.toUtf8();
    char* err = nullptr;
    keystoreHandle = GoWSK_accounts_keystore_NewKeyStore(dirUtf8.data(), 262144, 1, &err);
    if (keystoreHandle == 0) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: Failed to create keystore:" << emsg;
        return false;
    }
    qDebug() << "WalletModulePlugin: Keystore created: handle=" << (qulonglong)keystoreHandle;
    return true;
}

bool WalletModulePlugin::closeKeystore(const QString &privateKey)
{
    Q_UNUSED(privateKey);
    qDebug() << "WalletModulePlugin::closeKeystore";
    if (keystoreHandle != 0) {
        GoWSK_accounts_keystore_CloseKeyStore(keystoreHandle);
        keystoreHandle = 0;
        return true;
    }
    return false;
}

QString WalletModulePlugin::keystoreAccounts()
{
    qDebug() << "WalletModulePlugin::keystoreAccounts";
    if (keystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Keystore not initialized";
        return QString();
    }
    char* err = nullptr;
    char* accountsJson = GoWSK_accounts_keystore_Accounts(keystoreHandle, &err);
    if (accountsJson == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: Accounts error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(accountsJson);
    GoWSK_FreeCString(accountsJson);
    return result;
}

QString WalletModulePlugin::keystoreNewAccount(const QString &passphrase)
{
    qDebug() << "WalletModulePlugin::keystoreNewAccount";
    if (keystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Keystore not initialized";
        return QString();
    }
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    char* err = nullptr;
    char* address = GoWSK_accounts_keystore_NewAccount(keystoreHandle, passphraseUtf8.data(), &err);
    if (address == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: NewAccount error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(address);
    GoWSK_FreeCString(address);
    return result;
}

QString WalletModulePlugin::keystoreImport(const QString &keyJSON, const QString &passphrase, const QString &newPassphrase)
{
    qDebug() << "WalletModulePlugin::keystoreImport";
    if (keystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Keystore not initialized";
        return QString();
    }
    QByteArray keyJsonUtf8 = keyJSON.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    QByteArray newPassphraseUtf8 = newPassphrase.toUtf8();
    char* err = nullptr;
    char* address = GoWSK_accounts_keystore_Import(keystoreHandle, keyJsonUtf8.data(), passphraseUtf8.data(), newPassphraseUtf8.data(), &err);
    if (address == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: Import error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(address);
    GoWSK_FreeCString(address);
    return result;
}

QString WalletModulePlugin::keystoreExport(const QString &address, const QString &passphrase, const QString &newPassphrase)
{
    qDebug() << "WalletModulePlugin::keystoreExport";
    if (keystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Keystore not initialized";
        return QString();
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    QByteArray newPassphraseUtf8 = newPassphrase.toUtf8();
    char* err = nullptr;
    char* keyJson = GoWSK_accounts_keystore_Export(keystoreHandle, addressUtf8.data(), passphraseUtf8.data(), newPassphraseUtf8.data(), &err);
    if (keyJson == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: Export error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(keyJson);
    GoWSK_FreeCString(keyJson);
    return result;
}

bool WalletModulePlugin::keystoreDelete(const QString &address, const QString &passphrase)
{
    qDebug() << "WalletModulePlugin::keystoreDelete";
    if (keystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Keystore not initialized";
        return false;
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    char* err = nullptr;
    GoWSK_accounts_keystore_Delete(keystoreHandle, addressUtf8.data(), passphraseUtf8.data(), &err);
    if (err != nullptr) {
        QString emsg = QString::fromUtf8(err);
        GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: Delete error:" << emsg;
        return false;
    }
    return true;
}

bool WalletModulePlugin::keystoreHasAddress(const QString &address)
{
    qDebug() << "WalletModulePlugin::keystoreHasAddress";
    if (keystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Keystore not initialized";
        return false;
    }
    QByteArray addressUtf8 = address.toUtf8();
    char* err = nullptr;
    int result = GoWSK_accounts_keystore_HasAddress(keystoreHandle, addressUtf8.data(), &err);
    if (err != nullptr) {
        QString emsg = QString::fromUtf8(err);
        GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: HasAddress error:" << emsg;
        return false;
    }
    return result != 0;
}

bool WalletModulePlugin::keystoreUnlock(const QString &address, const QString &passphrase)
{
    qDebug() << "WalletModulePlugin::keystoreUnlock";
    if (keystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Keystore not initialized";
        return false;
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    char* err = nullptr;
    GoWSK_accounts_keystore_Unlock(keystoreHandle, addressUtf8.data(), passphraseUtf8.data(), &err);
    if (err != nullptr) {
        QString emsg = QString::fromUtf8(err);
        GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: Unlock error:" << emsg;
        return false;
    }
    return true;
}

bool WalletModulePlugin::keystoreLock(const QString &address)
{
    qDebug() << "WalletModulePlugin::keystoreLock";
    if (keystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Keystore not initialized";
        return false;
    }
    QByteArray addressUtf8 = address.toUtf8();
    char* err = nullptr;
    GoWSK_accounts_keystore_Lock(keystoreHandle, addressUtf8.data(), &err);
    if (err != nullptr) {
        QString emsg = QString::fromUtf8(err);
        GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: Lock error:" << emsg;
        return false;
    }
    return true;
}

bool WalletModulePlugin::keystoreTimedUnlock(const QString &address, const QString &passphrase, unsigned long timeoutSeconds)
{
    qDebug() << "WalletModulePlugin::keystoreTimedUnlock";
    if (keystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Keystore not initialized";
        return false;
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    char* err = nullptr;
    GoWSK_accounts_keystore_TimedUnlock(keystoreHandle, addressUtf8.data(), passphraseUtf8.data(), timeoutSeconds, &err);
    if (err != nullptr) {
        QString emsg = QString::fromUtf8(err);
        GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: TimedUnlock error:" << emsg;
        return false;
    }
    return true;
}

bool WalletModulePlugin::keystoreUpdate(const QString &address, const QString &passphrase, const QString &newPassphrase)
{
    qDebug() << "WalletModulePlugin::keystoreUpdate";
    if (keystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Keystore not initialized";
        return false;
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    QByteArray newPassphraseUtf8 = newPassphrase.toUtf8();
    char* err = nullptr;
    GoWSK_accounts_keystore_Update(keystoreHandle, addressUtf8.data(), passphraseUtf8.data(), newPassphraseUtf8.data(), &err);
    if (err != nullptr) {
        QString emsg = QString::fromUtf8(err);
        GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: Update error:" << emsg;
        return false;
    }
    return true;
}

QString WalletModulePlugin::keystoreSignHash(const QString &address, const QString &hashHex)
{
    qDebug() << "WalletModulePlugin::keystoreSignHash";
    if (keystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Keystore not initialized";
        return QString();
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray hashHexUtf8 = hashHex.toUtf8();
    char* err = nullptr;
    char* signature = GoWSK_accounts_keystore_SignHash(keystoreHandle, addressUtf8.data(), hashHexUtf8.data(), &err);
    if (signature == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: SignHash error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(signature);
    GoWSK_FreeCString(signature);
    return result;
}

QString WalletModulePlugin::keystoreSignHashWithPassphrase(const QString &address, const QString &passphrase, const QString &hashHex)
{
    qDebug() << "WalletModulePlugin::keystoreSignHashWithPassphrase";
    if (keystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Keystore not initialized";
        return QString();
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    QByteArray hashHexUtf8 = hashHex.toUtf8();
    char* err = nullptr;
    char* signature = GoWSK_accounts_keystore_SignHashWithPassphrase(keystoreHandle, addressUtf8.data(), passphraseUtf8.data(), hashHexUtf8.data(), &err);
    if (signature == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: SignHashWithPassphrase error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(signature);
    GoWSK_FreeCString(signature);
    return result;
}

QString WalletModulePlugin::keystoreImportECDSA(const QString &privateKeyHex, const QString &passphrase)
{
    qDebug() << "WalletModulePlugin::keystoreImportECDSA";
    if (keystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Keystore not initialized";
        return QString();
    }
    QByteArray privateKeyHexUtf8 = privateKeyHex.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    char* err = nullptr;
    char* address = GoWSK_accounts_keystore_ImportECDSA(keystoreHandle, privateKeyHexUtf8.data(), passphraseUtf8.data(), &err);
    if (address == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ImportECDSA error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(address);
    GoWSK_FreeCString(address);
    return result;
}

QString WalletModulePlugin::keystoreSignTx(const QString &address, const QString &txJSON, const QString &chainIDHex)
{
    qDebug() << "WalletModulePlugin::keystoreSignTx";
    if (keystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Keystore not initialized";
        return QString();
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray txJsonUtf8 = txJSON.toUtf8();
    QByteArray chainIdHexUtf8 = chainIDHex.toUtf8();
    char* err = nullptr;
    char* signedTx = GoWSK_accounts_keystore_SignTx(keystoreHandle, addressUtf8.data(), txJsonUtf8.data(), chainIdHexUtf8.data(), &err);
    if (signedTx == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: SignTx error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(signedTx);
    GoWSK_FreeCString(signedTx);
    return result;
}

QString WalletModulePlugin::keystoreSignTxWithPassphrase(const QString &address, const QString &passphrase, const QString &txJSON, const QString &chainIDHex)
{
    qDebug() << "WalletModulePlugin::keystoreSignTxWithPassphrase";
    if (keystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Keystore not initialized";
        return QString();
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    QByteArray txJsonUtf8 = txJSON.toUtf8();
    QByteArray chainIdHexUtf8 = chainIDHex.toUtf8();
    char* err = nullptr;
    char* signedTx = GoWSK_accounts_keystore_SignTxWithPassphrase(keystoreHandle, addressUtf8.data(), passphraseUtf8.data(), txJsonUtf8.data(), chainIdHexUtf8.data(), &err);
    if (signedTx == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: SignTxWithPassphrase error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(signedTx);
    GoWSK_FreeCString(signedTx);
    return result;
}

QString WalletModulePlugin::keystoreFind(const QString &address, const QString &url)
{
    qDebug() << "WalletModulePlugin::keystoreFind";
    if (keystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Keystore not initialized";
        return QString();
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray urlUtf8 = url.toUtf8();
    char* err = nullptr;
    char* resultStr = GoWSK_accounts_keystore_Find(keystoreHandle, addressUtf8.data(), urlUtf8.data(), &err);
    if (resultStr == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: Find error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(resultStr);
    GoWSK_FreeCString(resultStr);
    return result;
}

// Extended keystore operations
bool WalletModulePlugin::initExtKeystore(const QString &dir)
{
    qDebug() << "WalletModulePlugin::initExtKeystore" << dir;
    if (extkeystoreHandle != 0) {
        GoWSK_accounts_extkeystore_CloseKeyStore(extkeystoreHandle);
    }
    QByteArray dirUtf8 = dir.toUtf8();
    char* err = nullptr;
    extkeystoreHandle = GoWSK_accounts_extkeystore_NewKeyStore(dirUtf8.data(), 262144, 1, &err);
    if (extkeystoreHandle == 0) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: Failed to create ext keystore:" << emsg;
        return false;
    }
    qDebug() << "WalletModulePlugin: Ext keystore created: handle=" << (qulonglong)extkeystoreHandle;
    return true;
}

bool WalletModulePlugin::closeExtKeystore()
{
    qDebug() << "WalletModulePlugin::closeExtKeystore";
    if (extkeystoreHandle != 0) {
        GoWSK_accounts_extkeystore_CloseKeyStore(extkeystoreHandle);
        extkeystoreHandle = 0;
        return true;
    }
    return false;
}

QString WalletModulePlugin::extKeystoreAccounts()
{
    qDebug() << "WalletModulePlugin::extKeystoreAccounts";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return QString();
    }
    char* err = nullptr;
    char* accountsJson = GoWSK_accounts_extkeystore_Accounts(extkeystoreHandle, &err);
    if (accountsJson == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtAccounts error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(accountsJson);
    GoWSK_FreeCString(accountsJson);
    return result;
}

QString WalletModulePlugin::extKeystoreNewAccount(const QString &passphrase)
{
    qDebug() << "WalletModulePlugin::extKeystoreNewAccount";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return QString();
    }
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    char* err = nullptr;
    char* address = GoWSK_accounts_extkeystore_NewAccount(extkeystoreHandle, passphraseUtf8.data(), &err);
    if (address == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtNewAccount error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(address);
    GoWSK_FreeCString(address);
    return result;
}

QString WalletModulePlugin::extKeystoreImport(const QString &keyJSON, const QString &passphrase, const QString &newPassphrase)
{
    qDebug() << "WalletModulePlugin::extKeystoreImport";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return QString();
    }
    QByteArray keyJsonUtf8 = keyJSON.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    QByteArray newPassphraseUtf8 = newPassphrase.toUtf8();
    char* err = nullptr;
    char* address = GoWSK_accounts_extkeystore_Import(extkeystoreHandle, keyJsonUtf8.data(), passphraseUtf8.data(), newPassphraseUtf8.data(), &err);
    if (address == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtImport error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(address);
    GoWSK_FreeCString(address);
    return result;
}

QString WalletModulePlugin::extKeystoreImportExtendedKey(const QString &extKeyStr, const QString &passphrase)
{
    qDebug() << "WalletModulePlugin::extKeystoreImportExtendedKey";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return QString();
    }
    QByteArray extKeyStrUtf8 = extKeyStr.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    char* err = nullptr;
    char* address = GoWSK_accounts_extkeystore_ImportExtendedKey(extkeystoreHandle, extKeyStrUtf8.data(), passphraseUtf8.data(), &err);
    if (address == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtImportExtendedKey error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(address);
    GoWSK_FreeCString(address);
    return result;
}

QString WalletModulePlugin::extKeystoreExportExt(const QString &address, const QString &passphrase, const QString &newPassphrase)
{
    qDebug() << "WalletModulePlugin::extKeystoreExportExt";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return QString();
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    QByteArray newPassphraseUtf8 = newPassphrase.toUtf8();
    char* err = nullptr;
    char* extKey = GoWSK_accounts_extkeystore_ExportExt(extkeystoreHandle, addressUtf8.data(), passphraseUtf8.data(), newPassphraseUtf8.data(), &err);
    if (extKey == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtExportExt error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(extKey);
    GoWSK_FreeCString(extKey);
    return result;
}

QString WalletModulePlugin::extKeystoreExportPriv(const QString &address, const QString &passphrase, const QString &newPassphrase)
{
    qDebug() << "WalletModulePlugin::extKeystoreExportPriv";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return QString();
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    QByteArray newPassphraseUtf8 = newPassphrase.toUtf8();
    char* err = nullptr;
    char* privKey = GoWSK_accounts_extkeystore_ExportPriv(extkeystoreHandle, addressUtf8.data(), passphraseUtf8.data(), newPassphraseUtf8.data(), &err);
    if (privKey == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtExportPriv error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(privKey);
    GoWSK_FreeCString(privKey);
    return result;
}

bool WalletModulePlugin::extKeystoreDelete(const QString &address, const QString &passphrase)
{
    qDebug() << "WalletModulePlugin::extKeystoreDelete";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return false;
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    char* err = nullptr;
    GoWSK_accounts_extkeystore_Delete(extkeystoreHandle, addressUtf8.data(), passphraseUtf8.data(), &err);
    if (err != nullptr) {
        QString emsg = QString::fromUtf8(err);
        GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtDelete error:" << emsg;
        return false;
    }
    return true;
}

bool WalletModulePlugin::extKeystoreHasAddress(const QString &address)
{
    qDebug() << "WalletModulePlugin::extKeystoreHasAddress";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return false;
    }
    QByteArray addressUtf8 = address.toUtf8();
    char* err = nullptr;
    int result = GoWSK_accounts_extkeystore_HasAddress(extkeystoreHandle, addressUtf8.data(), &err);
    if (err != nullptr) {
        QString emsg = QString::fromUtf8(err);
        GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtHasAddress error:" << emsg;
        return false;
    }
    return result != 0;
}

bool WalletModulePlugin::extKeystoreUnlock(const QString &address, const QString &passphrase)
{
    qDebug() << "WalletModulePlugin::extKeystoreUnlock";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return false;
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    char* err = nullptr;
    GoWSK_accounts_extkeystore_Unlock(extkeystoreHandle, addressUtf8.data(), passphraseUtf8.data(), &err);
    if (err != nullptr) {
        QString emsg = QString::fromUtf8(err);
        GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtUnlock error:" << emsg;
        return false;
    }
    return true;
}

bool WalletModulePlugin::extKeystoreLock(const QString &address)
{
    qDebug() << "WalletModulePlugin::extKeystoreLock";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return false;
    }
    QByteArray addressUtf8 = address.toUtf8();
    char* err = nullptr;
    GoWSK_accounts_extkeystore_Lock(extkeystoreHandle, addressUtf8.data(), &err);
    if (err != nullptr) {
        QString emsg = QString::fromUtf8(err);
        GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtLock error:" << emsg;
        return false;
    }
    return true;
}

bool WalletModulePlugin::extKeystoreTimedUnlock(const QString &address, const QString &passphrase, unsigned long timeoutSeconds)
{
    qDebug() << "WalletModulePlugin::extKeystoreTimedUnlock";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return false;
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    char* err = nullptr;
    GoWSK_accounts_extkeystore_TimedUnlock(extkeystoreHandle, addressUtf8.data(), passphraseUtf8.data(), timeoutSeconds, &err);
    if (err != nullptr) {
        QString emsg = QString::fromUtf8(err);
        GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtTimedUnlock error:" << emsg;
        return false;
    }
    return true;
}

bool WalletModulePlugin::extKeystoreUpdate(const QString &address, const QString &passphrase, const QString &newPassphrase)
{
    qDebug() << "WalletModulePlugin::extKeystoreUpdate";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return false;
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    QByteArray newPassphraseUtf8 = newPassphrase.toUtf8();
    char* err = nullptr;
    GoWSK_accounts_extkeystore_Update(extkeystoreHandle, addressUtf8.data(), passphraseUtf8.data(), newPassphraseUtf8.data(), &err);
    if (err != nullptr) {
        QString emsg = QString::fromUtf8(err);
        GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtUpdate error:" << emsg;
        return false;
    }
    return true;
}

QString WalletModulePlugin::extKeystoreSignHash(const QString &address, const QString &hashHex)
{
    qDebug() << "WalletModulePlugin::extKeystoreSignHash";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return QString();
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray hashHexUtf8 = hashHex.toUtf8();
    char* err = nullptr;
    char* signature = GoWSK_accounts_extkeystore_SignHash(extkeystoreHandle, addressUtf8.data(), hashHexUtf8.data(), &err);
    if (signature == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtSignHash error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(signature);
    GoWSK_FreeCString(signature);
    return result;
}

QString WalletModulePlugin::extKeystoreSignHashWithPassphrase(const QString &address, const QString &passphrase, const QString &hashHex)
{
    qDebug() << "WalletModulePlugin::extKeystoreSignHashWithPassphrase";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return QString();
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    QByteArray hashHexUtf8 = hashHex.toUtf8();
    char* err = nullptr;
    char* signature = GoWSK_accounts_extkeystore_SignHashWithPassphrase(extkeystoreHandle, addressUtf8.data(), passphraseUtf8.data(), hashHexUtf8.data(), &err);
    if (signature == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtSignHashWithPassphrase error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(signature);
    GoWSK_FreeCString(signature);
    return result;
}

QString WalletModulePlugin::extKeystoreSignTx(const QString &address, const QString &txJSON, const QString &chainIDHex)
{
    qDebug() << "WalletModulePlugin::extKeystoreSignTx";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return QString();
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray txJsonUtf8 = txJSON.toUtf8();
    QByteArray chainIdHexUtf8 = chainIDHex.toUtf8();
    char* err = nullptr;
    char* signedTx = GoWSK_accounts_extkeystore_SignTx(extkeystoreHandle, addressUtf8.data(), txJsonUtf8.data(), chainIdHexUtf8.data(), &err);
    if (signedTx == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtSignTx error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(signedTx);
    GoWSK_FreeCString(signedTx);
    return result;
}

QString WalletModulePlugin::extKeystoreSignTxWithPassphrase(const QString &address, const QString &passphrase, const QString &txJSON, const QString &chainIDHex)
{
    qDebug() << "WalletModulePlugin::extKeystoreSignTxWithPassphrase";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return QString();
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    QByteArray txJsonUtf8 = txJSON.toUtf8();
    QByteArray chainIdHexUtf8 = chainIDHex.toUtf8();
    char* err = nullptr;
    char* signedTx = GoWSK_accounts_extkeystore_SignTxWithPassphrase(extkeystoreHandle, addressUtf8.data(), passphraseUtf8.data(), txJsonUtf8.data(), chainIdHexUtf8.data(), &err);
    if (signedTx == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtSignTxWithPassphrase error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(signedTx);
    GoWSK_FreeCString(signedTx);
    return result;
}

QString WalletModulePlugin::extKeystoreDerive(const QString &address, const QString &derivationPath)
{
    qDebug() << "WalletModulePlugin::extKeystoreDerive";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return QString();
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray derivationPathUtf8 = derivationPath.toUtf8();
    char* err = nullptr;
    char* derivedAddress = GoWSK_accounts_extkeystore_Derive(extkeystoreHandle, addressUtf8.data(), derivationPathUtf8.data(), true, &err);
    if (derivedAddress == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtDerive error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(derivedAddress);
    GoWSK_FreeCString(derivedAddress);
    return result;
}

QString WalletModulePlugin::extKeystoreDeriveWithPassphrase(const QString &address, const QString &derivationPath, const QString &passphrase, const QString &newPassphrase)
{
    qDebug() << "WalletModulePlugin::extKeystoreDeriveWithPassphrase";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return QString();
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray derivationPathUtf8 = derivationPath.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    QByteArray newPassphraseUtf8 = newPassphrase.toUtf8();
    char* err = nullptr;
    char* derivedAddress = GoWSK_accounts_extkeystore_DeriveWithPassphrase(extkeystoreHandle, addressUtf8.data(), derivationPathUtf8.data(), true, passphraseUtf8.data(), newPassphraseUtf8.data(), &err);
    if (derivedAddress == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtDeriveWithPassphrase error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(derivedAddress);
    GoWSK_FreeCString(derivedAddress);
    return result;
}

QString WalletModulePlugin::extKeystoreFind(const QString &address, const QString &url)
{
    qDebug() << "WalletModulePlugin::extKeystoreFind";
    if (extkeystoreHandle == 0) {
        qWarning() << "WalletModulePlugin: Ext keystore not initialized";
        return QString();
    }
    QByteArray addressUtf8 = address.toUtf8();
    QByteArray urlUtf8 = url.toUtf8();
    char* err = nullptr;
    char* resultStr = GoWSK_accounts_extkeystore_Find(extkeystoreHandle, addressUtf8.data(), urlUtf8.data(), &err);
    if (resultStr == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtFind error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(resultStr);
    GoWSK_FreeCString(resultStr);
    return result;
}

// Key operations
QString WalletModulePlugin::createExtKeyFromMnemonic(const QString &phrase, const QString &passphrase)
{
    qDebug() << "WalletModulePlugin::createExtKeyFromMnemonic";
    QByteArray phraseUtf8 = phrase.toUtf8();
    QByteArray passphraseUtf8 = passphrase.toUtf8();
    char* err = nullptr;
    char* extKey = GoWSK_accounts_keys_CreateExtKeyFromMnemonic(phraseUtf8.data(), passphraseUtf8.data(), &err);
    if (extKey == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: CreateExtKeyFromMnemonic error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(extKey);
    GoWSK_FreeCString(extKey);
    return result;
}

QString WalletModulePlugin::deriveExtKey(const QString &extKeyStr, const QString &pathStr)
{
    qDebug() << "WalletModulePlugin::deriveExtKey";
    QByteArray extKeyStrUtf8 = extKeyStr.toUtf8();
    QByteArray pathStrUtf8 = pathStr.toUtf8();
    char* err = nullptr;
    char* derivedKey = GoWSK_accounts_keys_DeriveExtKey(extKeyStrUtf8.data(), pathStrUtf8.data(), &err);
    if (derivedKey == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: DeriveExtKey error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(derivedKey);
    GoWSK_FreeCString(derivedKey);
    return result;
}

QString WalletModulePlugin::extKeyToECDSA(const QString &extKeyStr)
{
    qDebug() << "WalletModulePlugin::extKeyToECDSA";
    QByteArray extKeyStrUtf8 = extKeyStr.toUtf8();
    char* err = nullptr;
    char* ecdsaKey = GoWSK_accounts_keys_ExtKeyToECDSA(extKeyStrUtf8.data(), &err);
    if (ecdsaKey == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ExtKeyToECDSA error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(ecdsaKey);
    GoWSK_FreeCString(ecdsaKey);
    return result;
}

QString WalletModulePlugin::ecdsaToPublicKey(const QString &privateKeyECDSAStr)
{
    qDebug() << "WalletModulePlugin::ecdsaToPublicKey";
    QByteArray privateKeyECDSAStrUtf8 = privateKeyECDSAStr.toUtf8();
    char* err = nullptr;
    char* publicKey = GoWSK_accounts_keys_ECDSAToPublicKey(privateKeyECDSAStrUtf8.data(), &err);
    if (publicKey == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: ECDSAToPublicKey error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(publicKey);
    GoWSK_FreeCString(publicKey);
    return result;
}

QString WalletModulePlugin::publicKeyToAddress(const QString &publicKeyStr)
{
    qDebug() << "WalletModulePlugin::publicKeyToAddress";
    QByteArray publicKeyStrUtf8 = publicKeyStr.toUtf8();
    char* err = nullptr;
    char* address = GoWSK_accounts_keys_PublicKeyToAddress(publicKeyStrUtf8.data(), &err);
    if (address == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: PublicKeyToAddress error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(address);
    GoWSK_FreeCString(address);
    return result;
}

// Mnemonic operations
QString WalletModulePlugin::createRandomMnemonic(int length)
{
    qDebug() << "WalletModulePlugin::createRandomMnemonic" << length;
    char* err = nullptr;
    char* mnemonic = GoWSK_accounts_mnemonic_CreateRandomMnemonic(length, &err);
    if (mnemonic == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: CreateRandomMnemonic error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(mnemonic);
    GoWSK_FreeCString(mnemonic);
    return result;
}

QString WalletModulePlugin::createRandomMnemonicWithDefaultLength()
{
    qDebug() << "WalletModulePlugin::createRandomMnemonicWithDefaultLength";
    char* err = nullptr;
    char* mnemonic = GoWSK_accounts_mnemonic_CreateRandomMnemonicWithDefaultLength(&err);
    if (mnemonic == nullptr) {
        QString emsg = err ? QString::fromUtf8(err) : QString("unknown error");
        if (err) GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: CreateRandomMnemonicWithDefaultLength error:" << emsg;
        return QString();
    }
    QString result = QString::fromUtf8(mnemonic);
    GoWSK_FreeCString(mnemonic);
    return result;
}

uint32_t WalletModulePlugin::lengthToEntropyStrength(int length)
{
    qDebug() << "WalletModulePlugin::lengthToEntropyStrength" << length;
    char* err = nullptr;
    uint32_t result = GoWSK_accounts_mnemonic_LengthToEntropyStrength(length, &err);
    if (err != nullptr) {
        QString emsg = QString::fromUtf8(err);
        GoWSK_FreeCString(err);
        qWarning() << "WalletModulePlugin: LengthToEntropyStrength error:" << emsg;
        return 0;
    }
    return result;
}
