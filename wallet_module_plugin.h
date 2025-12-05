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

    Q_INVOKABLE bool initWallet(const QString &configJson) override;
    Q_INVOKABLE QString rpcCall(const QString &rpcUrl, const QString &method, const QString &params) override;
    Q_INVOKABLE QString chainId(const QString &rpcUrl) override;
    Q_INVOKABLE QString getEthBalance(const QString &rpcUrl, const QString &address) override;
    Q_INVOKABLE QString getErc20Balances(const QString &rpcUrl, const QString &address, const QStringList &tokenAddresses) override;

    // Keystore operations
    Q_INVOKABLE bool initKeystore(const QString &dir) override;
    Q_INVOKABLE bool closeKeystore(const QString &privateKey) override;
    Q_INVOKABLE QString keystoreAccounts() override;
    Q_INVOKABLE QString keystoreNewAccount(const QString &passphrase) override;
    Q_INVOKABLE QString keystoreImport(const QString &keyJSON, const QString &passphrase, const QString &newPassphrase) override;
    Q_INVOKABLE QString keystoreExport(const QString &address, const QString &passphrase, const QString &newPassphrase) override;
    Q_INVOKABLE bool keystoreDelete(const QString &address, const QString &passphrase) override;
    Q_INVOKABLE bool keystoreHasAddress(const QString &address) override;
    Q_INVOKABLE bool keystoreUnlock(const QString &address, const QString &passphrase) override;
    Q_INVOKABLE bool keystoreLock(const QString &address) override;
    Q_INVOKABLE bool keystoreTimedUnlock(const QString &address, const QString &passphrase, unsigned long timeoutSeconds) override;
    Q_INVOKABLE bool keystoreUpdate(const QString &address, const QString &passphrase, const QString &newPassphrase) override;
    Q_INVOKABLE QString keystoreSignHash(const QString &address, const QString &hashHex) override;
    Q_INVOKABLE QString keystoreSignHashWithPassphrase(const QString &address, const QString &passphrase, const QString &hashHex) override;
    Q_INVOKABLE QString keystoreImportECDSA(const QString &privateKeyHex, const QString &passphrase) override;
    Q_INVOKABLE QString keystoreSignTx(const QString &address, const QString &txJSON, const QString &chainIDHex) override;
    Q_INVOKABLE QString keystoreSignTxWithPassphrase(const QString &address, const QString &passphrase, const QString &txJSON, const QString &chainIDHex) override;
    Q_INVOKABLE QString keystoreFind(const QString &address, const QString &url) override;

    // Extended keystore operations
    Q_INVOKABLE bool initExtKeystore(const QString &dir) override;
    Q_INVOKABLE bool closeExtKeystore() override;
    Q_INVOKABLE QString extKeystoreAccounts() override;
    Q_INVOKABLE QString extKeystoreNewAccount(const QString &passphrase) override;
    Q_INVOKABLE QString extKeystoreImport(const QString &keyJSON, const QString &passphrase, const QString &newPassphrase) override;
    Q_INVOKABLE QString extKeystoreImportExtendedKey(const QString &extKeyStr, const QString &passphrase) override;
    Q_INVOKABLE QString extKeystoreExportExt(const QString &address, const QString &passphrase, const QString &newPassphrase) override;
    Q_INVOKABLE QString extKeystoreExportPriv(const QString &address, const QString &passphrase, const QString &newPassphrase) override;
    Q_INVOKABLE bool extKeystoreDelete(const QString &address, const QString &passphrase) override;
    Q_INVOKABLE bool extKeystoreHasAddress(const QString &address) override;
    Q_INVOKABLE bool extKeystoreUnlock(const QString &address, const QString &passphrase) override;
    Q_INVOKABLE bool extKeystoreLock(const QString &address) override;
    Q_INVOKABLE bool extKeystoreTimedUnlock(const QString &address, const QString &passphrase, unsigned long timeoutSeconds) override;
    Q_INVOKABLE bool extKeystoreUpdate(const QString &address, const QString &passphrase, const QString &newPassphrase) override;
    Q_INVOKABLE QString extKeystoreSignHash(const QString &address, const QString &hashHex) override;
    Q_INVOKABLE QString extKeystoreSignHashWithPassphrase(const QString &address, const QString &passphrase, const QString &hashHex) override;
    Q_INVOKABLE QString extKeystoreSignTx(const QString &address, const QString &txJSON, const QString &chainIDHex) override;
    Q_INVOKABLE QString extKeystoreSignTxWithPassphrase(const QString &address, const QString &passphrase, const QString &txJSON, const QString &chainIDHex) override;
    Q_INVOKABLE QString extKeystoreDerive(const QString &address, const QString &derivationPath) override;
    Q_INVOKABLE QString extKeystoreDeriveWithPassphrase(const QString &address, const QString &derivationPath, const QString &passphrase, const QString &newPassphrase) override;
    Q_INVOKABLE QString extKeystoreFind(const QString &address, const QString &url) override;

    // Key operations
    Q_INVOKABLE QString createExtKeyFromMnemonic(const QString &phrase, const QString &passphrase) override;
    Q_INVOKABLE QString deriveExtKey(const QString &extKeyStr, const QString &pathStr) override;
    Q_INVOKABLE QString extKeyToECDSA(const QString &extKeyStr) override;
    Q_INVOKABLE QString ecdsaToPublicKey(const QString &privateKeyECDSAStr) override;
    Q_INVOKABLE QString publicKeyToAddress(const QString &publicKeyStr) override;

    // Mnemonic operations
    Q_INVOKABLE QString createRandomMnemonic(int length) override;
    Q_INVOKABLE QString createRandomMnemonicWithDefaultLength() override;
    Q_INVOKABLE uint32_t lengthToEntropyStrength(int length) override;
    
    
    QString name() const override { return "wallet_module"; }
    QString version() const override { return "1.0.0"; }

    Q_INVOKABLE void initLogos(LogosAPI* logosAPIInstance);

signals:
    void eventResponse(const QString& eventName, const QVariantList& data);

private:
    unsigned long long walletHandle;
    unsigned long long keystoreHandle;
    unsigned long long extkeystoreHandle;

    static void simple_callback(int callerRet, const char* msg, size_t len, void* userData);
};


