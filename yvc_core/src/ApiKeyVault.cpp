#include "yvc_core/ApiKeyVault.h"

#ifdef _WIN32
#include <Windows.h>
#include <wincrypt.h>
#endif

#include <stdexcept>

namespace yvc {

    /// <summary>
    /// Stores the API key securely using platform facilities (DPAPI on Windows).
    /// </summary>
    void ApiKeyVault::store(const std::string& key) {
#ifdef _WIN32
        if(key.empty()) {
            encrypted_.clear();
            return;
        }
        DATA_BLOB in{};
        in.pbData = reinterpret_cast<BYTE*>(const_cast<char*>(key.data()));
        in.cbData = static_cast<DWORD>(key.size());
        DATA_BLOB out{};
        if(!CryptProtectData(&in, L"yvc_core", nullptr, nullptr, nullptr, 0, &out)) {
            throw std::runtime_error("Failed to protect API key using DPAPI");
        }
        encrypted_.assign(out.pbData, out.pbData + out.cbData);
        LocalFree(out.pbData);
#else
        in_memory_ = key;
#endif
    }

    /// <summary>
    /// Retrieves the stored API key (decrypts when necessary).
    /// </summary>
    std::string ApiKeyVault::retrieve() const {
#ifdef _WIN32
        if(encrypted_.empty()) {
            return {};
        }
        DATA_BLOB in{};
        in.pbData = const_cast<BYTE*>(encrypted_.data());
        in.cbData = static_cast<DWORD>(encrypted_.size());
        DATA_BLOB out{};
        if(!CryptUnprotectData(&in, nullptr, nullptr, nullptr, nullptr, 0, &out)) {
            throw std::runtime_error("Failed to unprotect API key using DPAPI");
        }
        std::string result(reinterpret_cast<char*>(out.pbData), out.cbData);
        LocalFree(out.pbData);
        return result;
#else
        return in_memory_;
#endif
    }

    /// <summary>
    /// Returns true if a key is currently stored.
    /// </summary>
    bool ApiKeyVault::hasKey() const {
#ifdef _WIN32
        return !encrypted_.empty();
#else
        return !in_memory_.empty();
#endif
    }

} // namespace yvc
