#include <iostream>
#include <string>
#include "RobotProtocol.h" // Import your custom module!

using namespace std;

int main()
{
    cout << "=== WAFER ROBOT SIMULATOR BOOTING ===" << endl;

    // In the future, this string will come from the COM Port or the Terminal Input function.
    // For now, we simulate receiving a string from the "outside".
    string incoming_data_from_outside = "\x01CMD|101|PICK FROM=LPA1 ARM=LOWER|4F\r\n";

    cout << "Waiting for command..." << endl;

    // Call the parser from your new module
    RobotFrame received = parse_incoming_frame(incoming_data_from_outside);

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