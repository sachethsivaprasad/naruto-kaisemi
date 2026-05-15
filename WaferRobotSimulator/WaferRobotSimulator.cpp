#include <iostream>
#include <string>
#include "../WaferRobotCommon/RobotProtocol.h"
#include "RobotSimDispatcher.h"

using namespace std;

int main()
{
    cout << "=== WAFER ROBOT SIMULATOR BOOTING  ===" << endl;

  
    string incoming_data_from_outside = "\x01" "CMD|101|PICK FROM=LPA1 ARM=LOWER|0C\r\n";

    cout << "Waiting for command..." << endl;

    
    RobotFrame received = parse_incoming_frame(incoming_data_from_outside);

    if (received.is_valid) {
        cout << "Valid command received. Sequence ID: " << received.sequence_id << endl;
        cout << "Executing Payload: " << received.payload << endl;


    }
    else {
        cout << "Dropping invalid frame." << endl;
    }

    dispatch_server_frame(received);

    return 0;
}