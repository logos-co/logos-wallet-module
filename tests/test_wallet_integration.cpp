// Integration tests for WalletModuleImpl — uses the REAL libgowalletsdk static library.
// No RPC calls: offline transaction helpers only (CI-friendly).

#include <logos_test.h>
#include "wallet_module_impl.h"

#include <string>

// Minimal legacy EIP-155 transaction JSON (chainId 1) suitable for hash / RLP helpers.
static const char* kSampleLegacyTx = R"({
  "type": "0x0",
  "nonce": "0x0",
  "gasPrice": "0x3b9aca00",
  "gas": "0x5208",
  "to": "0x70997970c51812dc3a010c7d01b50e0d17dc79c8",
  "value": "0x0",
  "input": "0x",
  "chainId": "0x1",
  "v": "0x26",
  "r": "0x1",
  "s": "0x1"
})";

LOGOS_TEST(integration_transaction_get_hash) {
    WalletModuleImpl impl;
    std::string hash = impl.transactionGetHash(kSampleLegacyTx);
    LOGOS_ASSERT_FALSE(hash.empty());
    LOGOS_ASSERT_TRUE(hash.size() > 2);
    LOGOS_ASSERT_TRUE(hash[0] == '0' && (hash[1] == 'x' || hash[1] == 'X'));
}

LOGOS_TEST(integration_transaction_json_to_rlp_roundtrip) {
    WalletModuleImpl impl;
    std::string rlp = impl.transactionJsonToRlp(kSampleLegacyTx);
    LOGOS_ASSERT_FALSE(rlp.empty());

    std::string back = impl.transactionRlpToJson(rlp);
    LOGOS_ASSERT_FALSE(back.empty());
}
