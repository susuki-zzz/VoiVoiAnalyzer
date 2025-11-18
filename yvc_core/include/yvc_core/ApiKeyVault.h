#pragma once

#include <string>
#include <vector>

namespace yvc {

    /// <summary>
    /// Provides secure storage for API keys using platform-specific encryption.
    /// On Windows, uses DPAPI; on other platforms, uses in-memory storage.
    /// </summary>
    class ApiKeyVault {
    public:
        /// <summary>
        /// Stores an API key securely.
        /// </summary>
        /// <param name="key">The API key to store</param>
        void store(const std::string& key);

        /// <summary>
        /// Retrieves the stored API key.
        /// </summary>
        /// <returns>The decrypted API key</returns>
        std::string retrieve() const;

        /// <summary>
        /// Checks if an API key is currently stored.
        /// </summary>
        /// <returns>True if a key is stored, false otherwise</returns>
        bool hasKey() const;

    private:
#ifdef _WIN32
        std::vector<unsigned char> encrypted_;
#else
        std::string in_memory_;
#endif
    };

} // namespace yvc
