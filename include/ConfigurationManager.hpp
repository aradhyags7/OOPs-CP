#ifndef CONFIGURATION_MANAGER_HPP
#define CONFIGURATION_MANAGER_HPP

#include "RuleManager.hpp"
#include <string>
#include <vector>

class ConfigurationManager {
public:
    static bool loadConfiguration(const std::string& filePath,
                                  RuleManager& targetManager,
                                  std::string& errorMessage);

    static bool saveConfiguration(const std::string& filePath,
                                  const RuleManager& sourceManager,
                                  std::string& errorMessage);

private:
    static std::string trim(const std::string& s);
    static std::vector<std::string> splitTokens(const std::string& line);
    static std::string extractQuotedString(const std::string& line);
};

#endif // CONFIGURATION_MANAGER_HPP
