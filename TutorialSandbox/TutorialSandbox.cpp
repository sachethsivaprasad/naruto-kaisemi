#include <iostream>
#include <string>
#include "RobotProtocol.h" // Import your custom module!

using namespace std;

int main()
{
    cout << "=== WAFER ROBOT SIMULATOR BOOTING ===" << endl;

    // In the future, this string will come from the COM Port or the Terminal Input function.
    // Simulate bytes arriving from UART / virtual COM; exact checksum stays in sync automatically.
    const std::string incoming_data_from_outside =
        build_frame("CMD", 101, "PICK FROM=LPA1 ARM=LOWER");

    cout << "Waiting for command..." << endl;

    const RobotFrame received = parse_incoming_frame(incoming_data_from_outside);

    // Process the result
    if (received.is_valid) {
        cout << "[SYSTEM] Valid command received. Sequence ID: " << received.sequence_id << endl;
        cout << "[SYSTEM] Executing Payload: " << received.payload << endl;

        // Next up: We will pass 'received.payload' to your CSV engine!

    }
    else {
        cout << "[SYSTEM] Dropping invalid frame." << endl;
    }

    return 0;
}