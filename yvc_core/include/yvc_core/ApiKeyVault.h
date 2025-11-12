#pragma once

#include <string>
#include <vector>

namespace yvc {

class ApiKeyVault {
public:
    void store(const std::string& key);
    std::string retrieve() const;
    bool hasKey() const;

private:
#ifdef _WIN32
    std::vector<unsigned char> encrypted_;
#else
    std::string in_memory_;
#endif
};

} // namespace yvc
