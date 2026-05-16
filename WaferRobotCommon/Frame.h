#pragma once
#include <string>

std::string build_robot_frame(const std::string& message_type, int sequence_id, const std::string& payload = "");
bool send_robot_frame(const std::string& com_port, const std::string& frame, std::string& error_message);