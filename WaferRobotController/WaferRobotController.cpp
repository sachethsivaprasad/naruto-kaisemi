#include <iostream>
#include <string>
#include "../WaferRobotCommon/RobotProtocol.h"
#include "../WaferRobotCommon/SerialLink.h"
using namespace std;

int main(int argc, char* argv[]) {
    string com_port;
    if (argc >= 2) {
        com_port = argv[1];
    }
    else {
        cout << "Usage: WaferRobotController.exe <COM port>" << endl;
        cout << "Example: WaferRobotController.exe COM5" << endl;
        cout << "Enter COM port: ";
        getline(cin, com_port);

        if (com_port.empty()) {
            cout << "[ERROR] COM port is required." << endl;
            return 1;
        }
    }

    int seq_counter = 100; // Start at 100
    string user_input;

    cout << "=== CONTROLLER TERMINAL ===" << endl;
    cout << "Sending frames on " << com_port << endl;
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

        string frame_to_send = build_robot_frame("CMD", seq_counter, user_input);
        string send_error;
        bool send_success = send_robot_frame(com_port, frame_to_send, send_error);

        // Output
        cout << "[SUCCESS] Frame Packaged!" << endl;

        string debug_print = frame_to_send.substr(1, frame_to_send.length() - 3);
        cout << "Raw Bytes: [SOH]" << debug_print << "[CRLF]" << endl;

        cout << "Calculated Checksum: " << frame_to_send.substr(frame_to_send.length() - 4, 2) << endl;

        if (send_success) {
            cout << "[SUCCESS] Frame Sent to " << com_port << endl;
        }
        else {
            cout << "[ERROR] Frame Send Failed: " << send_error << endl;
        }
    }

    return 0;
}
