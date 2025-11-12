#include <gtest/gtest.h>

#include <yvc_core/ApiKeyVault.h>

namespace yvc::test {
namespace {

TEST(ApiKeyVaultTests, StoresAndRetrievesKeyInMemory) {
    ApiKeyVault vault;
    EXPECT_FALSE(vault.hasKey());

    vault.store("secret-key");
    EXPECT_TRUE(vault.hasKey());
    EXPECT_EQ(vault.retrieve(), "secret-key");

    vault.store("");
    EXPECT_FALSE(vault.hasKey());
    EXPECT_EQ(vault.retrieve(), "");
}

} // namespace
} // namespace yvc::test
