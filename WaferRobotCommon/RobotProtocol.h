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
std::string build_robot_frame(const std::string& message_type, int sequence_id, const std::string& payload = "");
bool send_robot_frame(const std::string& com_port, const std::string& frame, std::string& error_message);
RobotFrame parse_incoming_frame(std::string raw_data);
