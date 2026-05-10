#pragma once
#include <string>
#include <string_view>

struct RobotFrame {
    std::string message_type;
    int sequence_id{};
    std::string payload;
    std::string hex_checksum;
    bool is_valid{};
};

[[nodiscard]] std::string calculate_checksum(std::string_view data);

[[nodiscard]] std::string build_frame(std::string_view message_type, int sequence_id, std::string_view payload);

[[nodiscard]] RobotFrame parse_incoming_frame(std::string_view raw_line_with_soh_optional_crlf);
