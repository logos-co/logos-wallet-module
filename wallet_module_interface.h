#pragma once

#include <QtCore/QObject>
#include "interface.h"

class WalletModuleInterface : public PluginInterface
{
public:
    virtual ~WalletModuleInterface() {}

    // Example operations; will be expanded as we integrate SDK
    Q_INVOKABLE virtual bool initWallet(const QString &configJson) = 0;
    Q_INVOKABLE virtual QString rpcCall(const QString &rpcUrl, const QString &method, const QString &params) = 0;
    Q_INVOKABLE virtual QString chainId(const QString &rpcUrl) = 0;
    Q_INVOKABLE virtual QString getEthBalance(const QString &rpcUrl, const QString &address) = 0;
    Q_INVOKABLE virtual QString getErc20Balances(const QString &rpcUrl, const QString &address, const QStringList &tokenAddresses) = 0;

    // Keystore operations
    Q_INVOKABLE virtual bool initKeystore(const QString &dir) = 0;
    Q_INVOKABLE virtual bool closeKeystore(const QString &privateKey) = 0;
    Q_INVOKABLE virtual QString keystoreAccounts() = 0;
    Q_INVOKABLE virtual QString keystoreNewAccount(const QString &passphrase) = 0;
    Q_INVOKABLE virtual QString keystoreImport(const QString &keyJSON, const QString &passphrase, const QString &newPassphrase) = 0;
    Q_INVOKABLE virtual QString keystoreExport(const QString &address, const QString &passphrase, const QString &newPassphrase) = 0;
    Q_INVOKABLE virtual bool keystoreDelete(const QString &address, const QString &passphrase) = 0;
    Q_INVOKABLE virtual bool keystoreHasAddress(const QString &address) = 0;
    Q_INVOKABLE virtual bool keystoreUnlock(const QString &address, const QString &passphrase) = 0;
    Q_INVOKABLE virtual bool keystoreLock(const QString &address) = 0;
    Q_INVOKABLE virtual bool keystoreTimedUnlock(const QString &address, const QString &passphrase, unsigned long timeoutSeconds) = 0;
    Q_INVOKABLE virtual bool keystoreUpdate(const QString &address, const QString &passphrase, const QString &newPassphrase) = 0;
    Q_INVOKABLE virtual QString keystoreSignHash(const QString &address, const QString &hashHex) = 0;
    Q_INVOKABLE virtual QString keystoreSignHashWithPassphrase(const QString &address, const QString &passphrase, const QString &hashHex) = 0;
    Q_INVOKABLE virtual QString keystoreImportECDSA(const QString &privateKeyHex, const QString &passphrase) = 0;
    Q_INVOKABLE virtual QString keystoreSignTx(const QString &address, const QString &txJSON, const QString &chainIDHex) = 0;
    Q_INVOKABLE virtual QString keystoreSignTxWithPassphrase(const QString &address, const QString &passphrase, const QString &txJSON, const QString &chainIDHex) = 0;
    Q_INVOKABLE virtual QString keystoreFind(const QString &address, const QString &url) = 0;

    // Extended keystore operations
    Q_INVOKABLE virtual bool initExtKeystore(const QString &dir) = 0;
    Q_INVOKABLE virtual bool closeExtKeystore() = 0;
    Q_INVOKABLE virtual QString extKeystoreAccounts() = 0;
    Q_INVOKABLE virtual QString extKeystoreNewAccount(const QString &passphrase) = 0;
    Q_INVOKABLE virtual QString extKeystoreImport(const QString &keyJSON, const QString &passphrase, const QString &newPassphrase) = 0;
    Q_INVOKABLE virtual QString extKeystoreImportExtendedKey(const QString &extKeyStr, const QString &passphrase) = 0;
    Q_INVOKABLE virtual QString extKeystoreExportExt(const QString &address, const QString &passphrase, const QString &newPassphrase) = 0;
    Q_INVOKABLE virtual QString extKeystoreExportPriv(const QString &address, const QString &passphrase, const QString &newPassphrase) = 0;
    Q_INVOKABLE virtual bool extKeystoreDelete(const QString &address, const QString &passphrase) = 0;
    Q_INVOKABLE virtual bool extKeystoreHasAddress(const QString &address) = 0;
    Q_INVOKABLE virtual bool extKeystoreUnlock(const QString &address, const QString &passphrase) = 0;
    Q_INVOKABLE virtual bool extKeystoreLock(const QString &address) = 0;
    Q_INVOKABLE virtual bool extKeystoreTimedUnlock(const QString &address, const QString &passphrase, unsigned long timeoutSeconds) = 0;
    Q_INVOKABLE virtual bool extKeystoreUpdate(const QString &address, const QString &passphrase, const QString &newPassphrase) = 0;
    Q_INVOKABLE virtual QString extKeystoreSignHash(const QString &address, const QString &hashHex) = 0;
    Q_INVOKABLE virtual QString extKeystoreSignHashWithPassphrase(const QString &address, const QString &passphrase, const QString &hashHex) = 0;
    Q_INVOKABLE virtual QString extKeystoreSignTx(const QString &address, const QString &txJSON, const QString &chainIDHex) = 0;
    Q_INVOKABLE virtual QString extKeystoreSignTxWithPassphrase(const QString &address, const QString &passphrase, const QString &txJSON, const QString &chainIDHex) = 0;
    Q_INVOKABLE virtual QString extKeystoreDerive(const QString &address, const QString &derivationPath) = 0;
    Q_INVOKABLE virtual QString extKeystoreDeriveWithPassphrase(const QString &address, const QString &derivationPath, const QString &passphrase, const QString &newPassphrase) = 0;
    Q_INVOKABLE virtual QString extKeystoreFind(const QString &address, const QString &url) = 0;

    // Key operations
    Q_INVOKABLE virtual QString createExtKeyFromMnemonic(const QString &phrase, const QString &passphrase) = 0;
    Q_INVOKABLE virtual QString deriveExtKey(const QString &extKeyStr, const QString &pathStr) = 0;
    Q_INVOKABLE virtual QString extKeyToECDSA(const QString &extKeyStr) = 0;
    Q_INVOKABLE virtual QString ecdsaToPublicKey(const QString &privateKeyECDSAStr) = 0;
    Q_INVOKABLE virtual QString publicKeyToAddress(const QString &publicKeyStr) = 0;

    // Mnemonic operations
    Q_INVOKABLE virtual QString createRandomMnemonic(int length) = 0;
    Q_INVOKABLE virtual QString createRandomMnemonicWithDefaultLength() = 0;
    Q_INVOKABLE virtual uint32_t lengthToEntropyStrength(int length) = 0;
signals:
    void eventResponse(const QString& eventName, const QVariantList& data);
};

#define WalletModuleInterface_iid "org.logos.WalletModuleInterface"
Q_DECLARE_INTERFACE(WalletModuleInterface, WalletModuleInterface_iid)


