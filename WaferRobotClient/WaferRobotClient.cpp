#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include "../WaferRobotCommon/RobotProtocol.h"
using namespace std;

// Frame Builder
string build_robot_frame(int sequence_id, string payload) {
   
    string core_data = "CMD|" + to_string(sequence_id) + "|" + payload;
    string checksum = calculate_checksum(core_data);
    string final_frame = "\x01" + core_data + "|" + checksum + "\r\n";

    return final_frame;
}

int main() {
    int seq_counter = 100; // Start at 100
    string user_input;

    cout << "=== CLIENT TERMINAL SIMULATOR ===" << endl;
    cout << "Type a payload to package (or type 'exit' to quit)." << endl;
    cout << "Example: PICK FROM=LPA1 ARM=LOWER" << endl;

    while (true) {
        cout << "\n> ";
        getline(cin, user_input);

        if (user_input == "exit") {
            break;
        }

        if (user_input.empty()) {
            continue;
        }

        seq_counter++; 

        string frame_to_send = build_robot_frame(seq_counter, user_input);

        // Output
        cout << "[SUCCESS] Frame Packaged!" << endl;

        string debug_print = frame_to_send.substr(1, frame_to_send.length() - 3);
        cout << "Raw Bytes: [SOH]" << debug_print << "[CRLF]" << endl;

        cout << "Calculated Checksum: " << frame_to_send.substr(frame_to_send.length() - 4, 2) << endl;

       
    }

    return 0;
}