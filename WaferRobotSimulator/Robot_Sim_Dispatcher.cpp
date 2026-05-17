#include "RobotSimDispatcher.h"
#include <iostream>
#include "../WaferRobotCommon/RobotProtocol.h"
#include "CommandRegistry.h"
#include "../WaferRobotCommon/SerialLink.h"
#include <sstream>
using namespace std;

ParsedCommand extract_payload(const string& payload) {
    ParsedCommand cmd;
    stringstream ss(payload);
    string token;
    if (getline(ss, token, ' ')) {
        cmd.action = token;
    }
    while (getline(ss, token, ' ')) {
        size_t eq_pos = token.find('=');
        if (eq_pos != string::npos) {
            string key = token.substr(0, eq_pos);
            string value = token.substr(eq_pos + 1);
            cmd.parameters[key] = value;
        }
    }
    return cmd;
}

void dispatch_server_frame(RobotFrame frame, CommandRegistry& registry, const string& com_port) {


    if (!frame.is_valid) {
        cout << "[SERVER WARNING] Frame rejected by Protocol Parser (Checksum/Format Error)." << endl;
        return;
    }

    //Router
    if (frame.message_type == "CMD") {
        cout << "\n>>> [ROUTED TO: COMMAND EXECUTION] <<<" << endl;
        cout << "Sequence ID : " << frame.sequence_id << endl;
        cout << "Raw Payload : " << frame.payload << endl;

        ParsedCommand cmd = extract_payload(frame.payload);
        if (!registry.validate_syntax(cmd) || !registry.validate_constraints(cmd)) {
            cout << "[SERVER REJECTED] Command failed validation." << endl;
            string nak_frame = build_robot_frame("NAK", frame.sequence_id, "");
            string err;
            send_robot_frame(com_port, nak_frame, err);
            return;
            return;
        }
        cout << "[SERVER ACCEPTED] Command is physically safe to execute." << endl;
        string ack_frame = build_robot_frame("ACK", frame.sequence_id, "");
        string err;
        send_robot_frame(com_port, ack_frame, err);
        // TODO: Pass the extracted action to the CSV Engine for timing
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