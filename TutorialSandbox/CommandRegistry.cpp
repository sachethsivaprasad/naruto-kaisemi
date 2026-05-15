#include "CommandRegistry.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

// --- STAGE A: THE BOOT LOADER ---
bool CommandRegistry::load_from_csv(const string& filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "[REGISTRY FATAL ERROR] Could not open " << filename << endl;
        return false;
    }

    string line;
    // 1. Read and throw away the Header row
    getline(file, line);

    // 2. Loop through the actual data
    while (getline(file, line)) {
        stringstream line_stream(line);
        string action, params_str;

        // Grab the action (up to the first comma)
        getline(line_stream, action, ',');

        // Grab the parameters (the rest of the line)
        getline(line_stream, params_str);

        vector<string> required_params;

        // If there are parameters, split them by the semicolon ';'
        if (!params_str.empty()) {
            stringstream param_stream(params_str);
            string single_param;
            while (getline(param_stream, single_param, ';')) {
                required_params.push_back(single_param);
            }
        }

        // Save it to our RAM database
        valid_commands[action] = required_params;
    }

    file.close();
    cout << "[REGISTRY] Loaded " << valid_commands.size() << " valid commands into RAM." << endl;
    return true;
}

// --- STAGE B: THE LIVE CHECKER ---
bool CommandRegistry::validate_command(const ParsedCommand& cmd) {

    // 1. Does the Action even exist?
    if (valid_commands.find(cmd.action) == valid_commands.end()) {
        cout << "[VALIDATION FAILED] Unknown Action: " << cmd.action << endl;
        return false;
    }

    // 2. Do we have all the required parameters?
    const vector<string>& required_params = valid_commands[cmd.action];

    for (const string& req_param : required_params) {
        // If we look in the command's parameters and can't find the required one...
        if (cmd.parameters.find(req_param) == cmd.parameters.end()) {
            cout << "[VALIDATION FAILED] Command '" << cmd.action
                << "' is missing required parameter: " << req_param << endl;
            return false;
        }
    }

    // If it survives both checks, it is a perfect command.
    return true;
}