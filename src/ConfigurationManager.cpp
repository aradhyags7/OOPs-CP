#include "ConfigurationManager.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

std::string ConfigurationManager::trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::vector<std::string> ConfigurationManager::splitTokens(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string ConfigurationManager::extractQuotedString(const std::string& line) {
    auto firstQuote = line.find('\"');
    if (firstQuote == std::string::npos) return "";
    auto secondQuote = line.find('\"', firstQuote + 1);
    if (secondQuote == std::string::npos) return "";
    return line.substr(firstQuote + 1, secondQuote - firstQuote - 1);
}

bool ConfigurationManager::loadConfiguration(const std::string& filePath,
                                             RuleManager& targetManager,
                                             std::string& errorMessage) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        errorMessage = "Unable to open configuration file: '" + filePath + "'";
        return false;
    }

    std::vector<FirewallRule> tempRules;
    std::string line;
    int lineNum = 0;

    bool insideRule = false;
    int currentId = 0;
    int currentPriority = 100;
    bool currentEnabled = true;
    ActionType currentAction = ActionType::BLOCK;
    std::string currentName;
    std::vector<std::unique_ptr<RuleCondition>> currentConditions;

    while (std::getline(file, line)) {
        lineNum++;
        std::string trimmed = trim(line);

        if (trimmed.empty() || trimmed[0] == '#') {
            continue; // Skip comments and empty lines
        }

        std::vector<std::string> tokens = splitTokens(trimmed);

        if (tokens[0] == "RULE") {
            if (insideRule) {
                errorMessage = "Syntax Error at line " + std::to_string(lineNum) + ": Nested RULE block without END_RULE";
                return false;
            }

            if (tokens.size() < 5) {
                errorMessage = "Syntax Error at line " + std::to_string(lineNum) + ": Insufficient RULE header tokens";
                return false;
            }

            try {
                currentId = std::stoi(tokens[1]);
                currentPriority = std::stoi(tokens[2]);
                currentEnabled = (tokens[3] == "1" || tokens[3] == "true");

                std::string act = tokens[4];
                if (act == "ALLOW") currentAction = ActionType::ALLOW;
                else if (act == "BLOCK") currentAction = ActionType::BLOCK;
                else if (act == "REJECT") currentAction = ActionType::REJECT;
                else if (act == "LOG") currentAction = ActionType::LOG;
                else {
                    errorMessage = "Syntax Error at line " + std::to_string(lineNum) + ": Unknown Action '" + act + "'";
                    return false;
                }

                currentName = extractQuotedString(trimmed);
                if (currentName.empty() && tokens.size() > 5) {
                    currentName = tokens[5];
                }

                // Check duplicate ID in temp batch
                for (const auto& r : tempRules) {
                    if (r.getId() == currentId) {
                        errorMessage = "Duplicate Rule ID Error at line " + std::to_string(lineNum) + ": ID " + std::to_string(currentId) + " already exists";
                        return false;
                    }
                }

                currentConditions.clear();
                insideRule = true;
            } catch (const std::exception& e) {
                errorMessage = "Data Conversion Error at line " + std::to_string(lineNum) + ": " + e.what();
                return false;
            }
        } else if (tokens[0] == "COND") {
            if (!insideRule) {
                errorMessage = "Syntax Error at line " + std::to_string(lineNum) + ": COND placed outside RULE block";
                return false;
            }

            if (tokens.size() < 3) {
                errorMessage = "Syntax Error at line " + std::to_string(lineNum) + ": Insufficient COND arguments";
                return false;
            }

            std::string condType = tokens[1];
            try {
                if (condType == "IP") {
                    if (tokens.size() < 4) {
                        errorMessage = "Syntax Error at line " + std::to_string(lineNum) + ": Expected COND IP <SRC|DST> <ip_or_cidr>";
                        return false;
                    }
                    IPField f = (tokens[2] == "SRC") ? IPField::SOURCE : IPField::DESTINATION;
                    currentConditions.push_back(std::make_unique<IPCondition>(tokens[3], f));
                } else if (condType == "PORT") {
                    if (tokens.size() < 4) {
                        errorMessage = "Syntax Error at line " + std::to_string(lineNum) + ": Expected COND PORT <SRC|DST> <port_or_range>";
                        return false;
                    }
                    PortField f = (tokens[2] == "SRC") ? PortField::SOURCE : PortField::DESTINATION;
                    auto dash = tokens[3].find('-');
                    if (dash != std::string::npos) {
                        uint16_t minP = static_cast<uint16_t>(std::stoi(tokens[3].substr(0, dash)));
                        uint16_t maxP = static_cast<uint16_t>(std::stoi(tokens[3].substr(dash + 1)));
                        currentConditions.push_back(std::make_unique<PortCondition>(minP, maxP, f));
                    } else {
                        uint16_t p = static_cast<uint16_t>(std::stoi(tokens[3]));
                        currentConditions.push_back(std::make_unique<PortCondition>(p, f));
                    }
                } else if (condType == "PROTO") {
                    std::string pStr = tokens[2];
                    ProtocolMatch pm = ProtocolMatch::ANY;
                    if (pStr == "TCP") pm = ProtocolMatch::TCP;
                    else if (pStr == "UDP") pm = ProtocolMatch::UDP;
                    else if (pStr == "ICMP") pm = ProtocolMatch::ICMP;
                    else if (pStr == "ANY") pm = ProtocolMatch::ANY;
                    else {
                        errorMessage = "Syntax Error at line " + std::to_string(lineNum) + ": Unknown Protocol '" + pStr + "'";
                        return false;
                    }
                    currentConditions.push_back(std::make_unique<ProtocolCondition>(pm));
                } else if (condType == "DIR") {
                    std::string dStr = tokens[2];
                    DirectionMatch dm = DirectionMatch::ANY;
                    if (dStr == "INBOUND") dm = DirectionMatch::INBOUND;
                    else if (dStr == "OUTBOUND") dm = DirectionMatch::OUTBOUND;
                    else if (dStr == "ANY") dm = DirectionMatch::ANY;
                    else {
                        errorMessage = "Syntax Error at line " + std::to_string(lineNum) + ": Unknown Direction '" + dStr + "'";
                        return false;
                    }
                    currentConditions.push_back(std::make_unique<DirectionCondition>(dm));
                } else {
                    errorMessage = "Syntax Error at line " + std::to_string(lineNum) + ": Unknown Condition Type '" + condType + "'";
                    return false;
                }
            } catch (const std::exception& e) {
                errorMessage = "Condition Validation Error at line " + std::to_string(lineNum) + ": " + e.what();
                return false;
            }
        } else if (tokens[0] == "END_RULE") {
            if (!insideRule) {
                errorMessage = "Syntax Error at line " + std::to_string(lineNum) + ": Unexpected END_RULE without matching RULE";
                return false;
            }

            if (currentConditions.empty()) {
                errorMessage = "Validation Error at line " + std::to_string(lineNum) + ": Rule #" + std::to_string(currentId) + " has zero conditions (Empty rules prohibited)";
                return false;
            }

            FirewallRule rule(currentId, currentPriority, currentEnabled, currentName, currentAction);
            for (auto& c : currentConditions) {
                rule.addCondition(std::move(c));
            }
            tempRules.push_back(std::move(rule));
            insideRule = false;
        } else {
            errorMessage = "Syntax Error at line " + std::to_string(lineNum) + ": Unknown directive '" + tokens[0] + "'";
            return false;
        }
    }

    if (insideRule) {
        errorMessage = "Syntax Error: Incomplete RULE block at end-of-file (missing END_RULE)";
        return false;
    }

    // Atomic replacement upon complete validation success
    targetManager.replaceAll(std::move(tempRules));
    return true;
}

bool ConfigurationManager::saveConfiguration(const std::string& filePath,
                                             const RuleManager& sourceManager,
                                             std::string& errorMessage) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        errorMessage = "Unable to open file for writing: '" + filePath + "'";
        return false;
    }

    file << "# ========================================================\n";
    file << "# Firewall Rule Policy Configuration Export\n";
    file << "# Generated by Firewall Rule Engine (C++17)\n";
    file << "# ========================================================\n\n";

    for (const auto& r : sourceManager.getRules()) {
        file << "RULE " << r.getId() << " " << r.getPriority() << " "
             << (r.isEnabled() ? "1" : "0") << " " << r.getActionString() << " \""
             << r.getName() << "\"\n";

        for (const auto& cond : r.getConditions()) {
            if (auto ipC = dynamic_cast<const IPCondition*>(cond.get())) {
                file << "  COND IP " << (ipC->getTargetField() == IPField::SOURCE ? "SRC " : "DST ")
                     << Packet::ipToString(ipC->getNetworkIP()) << "/" << ipC->getPrefix() << "\n";
            } else if (auto portC = dynamic_cast<const PortCondition*>(cond.get())) {
                file << "  COND PORT " << (portC->getTargetField() == PortField::SOURCE ? "SRC " : "DST ");
                if (portC->getMinPort() == portC->getMaxPort()) {
                    file << portC->getMinPort() << "\n";
                } else {
                    file << portC->getMinPort() << "-" << portC->getMaxPort() << "\n";
                }
            } else if (auto protoC = dynamic_cast<const ProtocolCondition*>(cond.get())) {
                file << "  COND PROTO ";
                switch (protoC->getProtocolMatch()) {
                    case ProtocolMatch::TCP:  file << "TCP\n"; break;
                    case ProtocolMatch::UDP:  file << "UDP\n"; break;
                    case ProtocolMatch::ICMP: file << "ICMP\n"; break;
                    case ProtocolMatch::ANY:  file << "ANY\n"; break;
                }
            } else if (auto dirC = dynamic_cast<const DirectionCondition*>(cond.get())) {
                file << "  COND DIR ";
                switch (dirC->getDirectionMatch()) {
                    case DirectionMatch::INBOUND:  file << "INBOUND\n"; break;
                    case DirectionMatch::OUTBOUND: file << "OUTBOUND\n"; break;
                    case DirectionMatch::ANY:      file << "ANY\n"; break;
                }
            }
        }

        file << "END_RULE\n\n";
    }

    file.close();
    return true;
}
