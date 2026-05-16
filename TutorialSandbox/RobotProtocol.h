#pragma once
#include <string>

// #pragma once stops the compiler from accidentally loading this file twice.

// 1. Define the struct so any file that includes this header knows what a RobotFrame is.
struct RobotFrame {
    std::string message_type;
    int sequence_id;
    std::string payload;
    std::string hex_checksum;
    bool is_valid;
};

// 2. Declare the functions (Notice there is no actual logic here, just the names and inputs!)
std::string calculate_checksum(std::string data);
RobotFrame parse_incoming_frame(std::string raw_data);