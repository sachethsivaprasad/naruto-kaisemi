#include <iostream>
#include <string>
#include "../WaferRobotCommon/RobotProtocol.h"
#include "../WaferRobotCommon/SerialLink.h"
#include "CommandRegistry.h"
#include "RobotSimDispatcher.h"

using namespace std;

namespace {
    bool is_fatal_receive_error(const string& error_message) {
        return error_message.rfind("Failed to open ", 0) == 0
            || error_message.rfind("Failed to read serial settings ", 0) == 0
            || error_message.rfind("Failed to configure ", 0) == 0;
    }
}

int main(int argc, char* argv[])
{
    string com_port;
    if (argc >= 2) {
        com_port = argv[1];
    }
    else {
        cout << "Usage: WaferRobotSimulator.exe <COM port>" << endl;
        cout << "Example: WaferRobotSimulator.exe COM6" << endl;
        cout << "Enter COM port: ";
        getline(cin, com_port);

        if (com_port.empty()) {
            cout << "[ERROR] COM port is required." << endl;
            return 1;
        }
    }


    cout << "=== WAFER ROBOT SIMULATOR BOOTING  ===" << endl;

    CommandRegistry master_registry;

    if (!master_registry.load_from_cfg("robot.cfg")) {
        cout << "CRITICAL FATAL ERROR: Could not load hardware configuration file!" << endl;
        return -1; 
    }
    cout << "Listening for controller frames on " << com_port << endl;

    while (true) {
        string incoming_data_from_outside;
        string receive_error;

        cout << "\nWaiting for command..." << endl;

        if (!receive_robot_frame(com_port, incoming_data_from_outside, receive_error)) {
            cout << "[ERROR] Frame Receive Failed: " << receive_error << endl;

            if (is_fatal_receive_error(receive_error)) {
                return 1;
            }

            continue;
        }

        string debug_print = incoming_data_from_outside.substr(1, incoming_data_from_outside.length() - 3);
        cout << "Raw Bytes: [SOH]" << debug_print << "[CRLF]" << endl;

        RobotFrame received = parse_incoming_frame(incoming_data_from_outside);

        if (received.is_valid) {
            cout << "Valid command received. Sequence ID: " << received.sequence_id << endl;
            cout << "Executing Payload: " << received.payload << endl;
        }
        else {
            cout << "Dropping invalid frame." << endl;
        }

        dispatch_server_frame(received, master_registry);
    }
}
