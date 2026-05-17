#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

struct ParsedCommand {
    std::string action;
    std::unordered_map<std::string, std::string> parameters;
};

struct ParameterRule {
    std::string name;
    std::string hardware_link;
};

struct CommandSignature {
    std::vector<ParameterRule> required;
    std::vector<ParameterRule> optional;
};

class CommandRegistry {
private:
    std::unordered_map<std::string, CommandSignature> valid_commands;
    std::unordered_map<std::string, std::vector<std::string>> hardware_limits;

    ParameterRule parse_rule(const std::string& raw_param);

public:
    bool load_from_cfg(const std::string& filename);
    bool validate_syntax(const ParsedCommand& cmd);
    bool validate_constraints(const ParsedCommand& cmd);
};