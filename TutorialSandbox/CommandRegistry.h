#pragma once
#include <string>
#include <vector>
#include <unordered_map>

// We need our ParsedCommand struct here so the Registry knows what it's looking at
struct ParsedCommand {
    std::string action;
    std::unordered_map<std::string, std::string> parameters;
};

class CommandRegistry {
private:
    // This is our RAM database. Key = "PICK", Value = {"FROM", "ARM"}
    std::unordered_map<std::string, std::vector<std::string>> valid_commands;

public:
    // The Boot Loader
    bool load_from_csv(const std::string& filename);

    // The Live Checker
    bool validate_command(const ParsedCommand& cmd);
};