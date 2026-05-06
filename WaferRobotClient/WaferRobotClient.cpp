#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

// 1. The Checksum Calculator (We need this on the Client side to generate it!)
string calculate_checksum(string data) {
    unsigned char checksum = 0;
    for (char c : data) {
        checksum ^= c;
    }
    stringstream hex_stream;
    hex_stream << uppercase << hex << setw(2) << setfill('0') << (int)checksum;
    return hex_stream.str();
}

// 2. The Frame Builder
string build_robot_frame(int sequence_id, string payload) {
    // Step A: Build the core data string
    string core_data = "CMD|" + to_string(sequence_id) + "|" + payload;

    // Step B: Calculate the checksum of that core data
    string checksum = calculate_checksum(core_data);

    // Step C: Wrap it in the SOH (\x01) and CRLF (\r\n) framing bytes
    string final_frame = "\x01" + core_data + "|" + checksum + "\r\n";

    return final_frame;
}

int main() {
    int seq_counter = 100; // Let's start our sequence IDs at 100
    string user_input;

    cout << "=== CLIENT TERMINAL SIMULATOR ===" << endl;
    cout << "Type a payload to package (or type 'exit' to quit)." << endl;
    cout << "Example: PICK FROM=LPA1 ARM=LOWER" << endl;

    while (true) {
        cout << "\n> ";

        // We use getline() instead of cin >> because cin stops reading at the first space!
        getline(cin, user_input);

        if (user_input == "exit") {
            break;
        }

        // Ignore accidental empty "Enters"
        if (user_input.empty()) {
            continue;
        }

        seq_counter++; // Automatically increment the ID for every new command

        // Generate the raw frame
        string frame_to_send = build_robot_frame(seq_counter, user_input);

        // --- DEBUG OUTPUT ---
        cout << "[SUCCESS] Frame Packaged!" << endl;

        // Because \x01 and \r\n are invisible to the terminal, printing 'frame_to_send' looks messy.
        // So, let's print a safe "Debug" version where we replace the invisible bytes with text:
        string debug_print = frame_to_send.substr(1, frame_to_send.length() - 3);
        cout << "Raw Bytes: [SOH]" << debug_print << "[CRLF]" << endl;

        // We also extract the checksum just to prove the math is working dynamically
        cout << "Calculated Checksum: " << frame_to_send.substr(frame_to_send.length() - 4, 2) << endl;
    }

    return 0;
}