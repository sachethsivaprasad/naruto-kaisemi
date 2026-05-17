#include <iostream>
#include <string>
#include "../WaferRobotCommon/RobotProtocol.h"

using namespace std;

void dispatch_client_frame(RobotFrame frame) {

    if (!frame.is_valid) {
        cout << "[CLIENT ERROR] Corrupted frame received from Server. Dropping." << endl;
        return;
    }

    // Router
    if (frame.message_type == "ACK") {
        cout << "[CLIENT] <<< Received ACK for Sequence ID: " << frame.sequence_id << endl;

        // TODO: Tell the Client's internal tracking system that the command is pending execution.
    }
    else if (frame.message_type == "STAT") {
        cout << "[CLIENT] <<< Received STAT Report. Sequence ID: " << frame.sequence_id << endl;
        cout << "         Status Data: " << frame.payload << endl;

        // TODO: Decode the 32-bit DWORD payload
    }
    else if (frame.message_type == "EVT") {
        // This is an ASYNCHRONOUS message. The Server sends this unprompted!
        // Examples: "MOVE COMPLETE", "HARDWARE COLLISION DETECTED", "ESTOP PRESSED".
        cout << "\n========================================" << endl;
        cout << "[CLIENT ALARM] <<< ASYNCHRONOUS EVENT RECEIVED!" << endl;
        cout << "Event Payload: " << frame.payload << endl;
        cout << "========================================\n" << endl;

        // TODO: Trigger emergency logic or queue the next command in the sequence.
    }
    else {
        cout << "\n[CLIENT WARNING] Unknown message type: " << frame.message_type << endl;
    }
}