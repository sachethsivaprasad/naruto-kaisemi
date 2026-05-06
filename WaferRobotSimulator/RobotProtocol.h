#pragma once
#include <string>

struct RobotFrame {
    std::string message_type;
    int sequence_id;
    std::string payload;
    std::string hex_checksum;
    bool is_valid;
};

std::string calculate_checksum(std::string data);
RobotFrame parse_incoming_frame(std::string raw_data);