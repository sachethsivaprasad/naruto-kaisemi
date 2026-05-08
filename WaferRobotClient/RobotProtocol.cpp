#include "RobotProtocol.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

//Checksum Calculator
string calculate_checksum(string data) {
    unsigned char checksum = 0;
    for (char c : data) {
        checksum ^= c;
    }
    stringstream hex_stream;
    hex_stream << uppercase << hex << setw(2) << setfill('0') << (int)checksum;
    return hex_stream.str();
}

//Parser
RobotFrame parse_incoming_frame(string raw_data) {
    RobotFrame frame;
    frame.is_valid = false;

    string clean_string = raw_data.substr(1, raw_data.length() - 3);
    stringstream ss(clean_string);
    string token;
    vector<string> parts;

    while (getline(ss, token, '|')) {
        parts.push_back(token);
    }

    if (parts.size() == 4) {
        frame.message_type = parts[0];
        frame.sequence_id = stoi(parts[1]);
        frame.payload = parts[2];
        frame.hex_checksum = parts[3];

        string data_to_check = parts[0] + "|" + parts[1] + "|" + parts[2];
        string calculated_hex = calculate_checksum(data_to_check);

        if (calculated_hex == frame.hex_checksum) {
            frame.is_valid = true;
        }
        else {
            cout << "[ERROR] Checksum Mismatch!" << endl;
        }
    }
    else {
        cout << "[ERROR] Malformed frame received!" << endl;
    }

    return frame;
}
