#include "RobotSimDispatcher.h"
#include <iostream>

using namespace std;

void dispatch_server_frame(RobotFrame frame) {

    if (!frame.is_valid) {
        cout << "[SERVER WARNING] Frame rejected by Protocol Parser (Checksum/Format Error)." << endl;
        return;
    }

    //Router
    if (frame.message_type == "CMD") {
        cout << "\n>>> [ROUTED TO: COMMAND EXECUTION] <<<" << endl;
        cout << "Sequence ID : " << frame.sequence_id << endl;
        cout << "Raw Payload : " << frame.payload << endl;

        // TODO: Pass frame.payload to the Payload Extractor
        // TODO: Pass the extracted action to the CSV Engine for timing
        // TODO: Send ACK back to Client
        // TODO: Wait the duration, then send EVT (Completion) or ACK (Done)

    }
    else if (frame.message_type == "STAT") {
        cout << "\n>>> [ROUTED TO: STATUS REPORTING] <<<" << endl;
        cout << "Sequence ID : " << frame.sequence_id << endl;

        // TODO: Generate the 32-bit DWORD status
        // TODO: Send STAT reply back to Client

    }
    else {
        // A Server should never receive an ACK or EVT
        cout << "\n[SERVER ERROR] Received illegal message type for a Server: " << frame.message_type << endl;
    }
}