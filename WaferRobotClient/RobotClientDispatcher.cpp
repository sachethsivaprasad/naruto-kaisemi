#include "RobotClientDispatcher.h"

#include <iostream>

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
    } else if (frame.message_type == "STAT") {
        cout << "[CLIENT] <<< Received STAT Report. Sequence ID: " << frame.sequence_id << endl;
        cout << "         DWORD (decimal): " << frame.payload << endl;

        // TODO: Decode busy/error bits directly from dword string.
    } else if (frame.message_type == "EVT") {

        cout << "\n========================================" << endl;
        cout << "[CLIENT EVENT] <<< Asynchronous EVT received." << endl;
        cout << "Sequence ID: " << frame.sequence_id << endl;
        cout << "DWORD payload: " << frame.payload << endl;
        cout << "========================================\n" << endl;

        // TODO: Trigger emergency logic or queue the next command in the sequence.
    } else {
        cout << "\n[CLIENT WARNING] Unknown message type: " << frame.message_type << endl;
    }
}
