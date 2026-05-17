#include "CommandRegistry.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

ParameterRule CommandRegistry::parse_rule(const string& raw_param) {
    ParameterRule rule;
    size_t colon_pos = raw_param.find(':');

    if (colon_pos != string::npos) {
        rule.name = raw_param.substr(0, colon_pos);
        rule.hardware_link = raw_param.substr(colon_pos + 1);
    }
    else {
        rule.name = raw_param;
        rule.hardware_link = "";
    }
    return rule;
}

bool CommandRegistry::load_from_cfg(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "[REGISTRY FATAL ERROR] Could not open " << filename << endl;
        return false;
    }

    string line;
    string current_section = "";

    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;

        if (line.front() == '[' && line.back() == ']') {
            current_section = line.substr(1, line.size() - 2);
            continue;
        }

        stringstream line_stream(line);
        string key, value;

        if (getline(line_stream, key, '=') && getline(line_stream, value)) {

            if (current_section == "COMMANDS") {
                CommandSignature sig;
                string req_str, opt_str;

                size_t pipe_pos = value.find('|');
                if (pipe_pos != string::npos) {
                    req_str = value.substr(0, pipe_pos);
                    opt_str = value.substr(pipe_pos + 1);
                }
                else {
                    req_str = value;
                }

                if (!req_str.empty()) {
                    stringstream req_stream(req_str);
                    string param;
                    while (getline(req_stream, param, ';')) sig.required.push_back(parse_rule(param));
                }

                if (!opt_str.empty()) {
                    stringstream opt_stream(opt_str);
                    string param;
                    while (getline(opt_stream, param, ';')) sig.optional.push_back(parse_rule(param));
                }

                valid_commands[key] = sig;
            }
            else if (current_section == "HARDWARE") {
                vector<string> limits;
                stringstream val_stream(value);
                string limit;

                while (getline(val_stream, limit, ',')) {
                    limits.push_back(limit);
                }
                hardware_limits[key] = limits;
            }
        }
    }
    file.close();
    cout << "[REGISTRY] Loaded " << valid_commands.size() << " commands and "
        << hardware_limits.size() << " hardware constraints." << endl;
    return true;
}

bool CommandRegistry::validate_syntax(const ParsedCommand& cmd) {
    if (valid_commands.find(cmd.action) == valid_commands.end()) {
        cout << "[SYNTAX FAILED] Unknown Action: " << cmd.action << endl;
        return false;
    }

    const CommandSignature& rules = valid_commands[cmd.action];

    for (const ParameterRule& rule : rules.required) {
        if (cmd.parameters.find(rule.name) == cmd.parameters.end()) {
            cout << "[SYNTAX FAILED] Missing parameter: " << rule.name << endl;
            return false;
        }
    }

    for (auto const& [user_key, user_value] : cmd.parameters) {
        bool found = false;
        for (const auto& rule : rules.required) if (rule.name == user_key) found = true;
        for (const auto& rule : rules.optional) if (rule.name == user_key) found = true;

        if (!found) {
            cout << "[SYNTAX FAILED] Illegal parameter: " << user_key << endl;
            return false;
        }
    }
    return true;
}

bool CommandRegistry::validate_constraints(const ParsedCommand& cmd) {
    if (valid_commands.find(cmd.action) == valid_commands.end()) return false;

    const CommandSignature& rules = valid_commands[cmd.action];

    for (auto const& [user_key, user_value] : cmd.parameters) {
        string hw_link = "";

        for (const auto& rule : rules.required) if (rule.name == user_key) hw_link = rule.hardware_link;
        for (const auto& rule : rules.optional) if (rule.name == user_key) hw_link = rule.hardware_link;

        if (!hw_link.empty()) {

            if (hardware_limits.find(hw_link) == hardware_limits.end()) {
                cout << "[HARDWARE FATAL] Config file is missing [" << hw_link << "] list!" << endl;
                return false;
            }

            const vector<string>& allowed = hardware_limits[hw_link];
            if (find(allowed.begin(), allowed.end(), user_value) == allowed.end()) {
                cout << "[CONSTRAINT FAILED] '" << user_value << "' is an invalid value for " << user_key << endl;
                return false;
            }
        }
    }
    return true;
}