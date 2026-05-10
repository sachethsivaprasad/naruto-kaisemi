#include "RobotProtocol.h"
#include <cctype>
#include <iomanip>
#include <sstream>
#include <vector>

namespace {

[[nodiscard]] bool is_two_hex_digits(std::string_view s) {
    if (s.size() != 2) {
        return false;
    }
    for (char c : s) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] std::string to_upper_hex(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        out.push_back(static_cast<char>(
            std::toupper(static_cast<unsigned char>(c))));
    }
    return out;
}

} // namespace

std::string calculate_checksum(std::string_view data) {
    unsigned char checksum = 0;
    for (unsigned char c : data) {
        checksum ^= static_cast<unsigned char>(c);
    }
    std::ostringstream hex_stream;
    hex_stream << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
               << static_cast<int>(checksum);
    return hex_stream.str();
}

std::string build_frame(std::string_view message_type, int sequence_id,
                        std::string_view payload) {
    std::string core;
    core.reserve(message_type.size() + 32 + payload.size());
    core.append(message_type);
    core.push_back('|');
    core.append(std::to_string(sequence_id));
    if (!payload.empty()) {
        core.push_back('|');
        core.append(payload);
    }
    const std::string chk = calculate_checksum(core);
    std::string frame;
    frame.reserve(3 + core.size() + chk.size());
    frame.push_back('\x01');
    frame.append(core);
    frame.push_back('|');
    frame.append(chk);
    frame.append("\r\n");
    return frame;
}

RobotFrame parse_incoming_frame(std::string_view raw) {
    RobotFrame frame;
    frame.is_valid = false;

    if (raw.empty()) {
        return frame;
    }
    std::size_t offset = 0;
    if (raw.front() == '\x01') {
        ++offset;
    }
    // Allow caller to pass lines with or without trailing CRLF
    std::size_t end = raw.size();
    if (end >= 2 && raw[end - 2] == '\r' && raw[end - 1] == '\n') {
        end -= 2;
    } else if (end >= 1 && raw[end - 1] == '\n') {
        --end;
        if (end >= 1 && raw[end - 1] == '\r') {
            --end;
        }
    }

    if (offset >= end) {
        return frame;
    }

    const std::string_view body(raw.data() + offset, end - offset);
    std::vector<std::string_view> parts;
    parts.reserve(8);

    std::size_t prev = 0;
    while (prev <= body.size()) {
        const std::size_t bar = body.find('|', prev);
        if (bar == std::string_view::npos) {
            parts.emplace_back(body.substr(prev));
            break;
        }
        parts.emplace_back(body.substr(prev, bar - prev));
        prev = bar + 1;
    }

    if (parts.size() < 3) {
        return frame;
    }

    const std::string checksum_token(parts.back());
    if (!is_two_hex_digits(checksum_token)) {
        return frame;
    }

    parts.pop_back();
    if (parts.size() < 2) {
        return frame;
    }

    frame.message_type = std::string(parts[0]);
    if (parts[0].empty()) {
        return frame;
    }

    try {
        frame.sequence_id = std::stoi(std::string(parts[1]));
    } catch (...) {
        return frame;
    }

    std::string core;
    core.reserve(body.size());
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i != 0) {
            core.push_back('|');
        }
        core.append(parts[i]);
    }

    frame.payload.clear();
    for (std::size_t i = 2; i < parts.size(); ++i) {
        if (i != 2) {
            frame.payload.push_back('|');
        }
        frame.payload.append(parts[i]);
    }

    const std::string expected = to_upper_hex(calculate_checksum(core));
    if (expected != to_upper_hex(checksum_token)) {
        return frame;
    }

    frame.hex_checksum = checksum_token;
    frame.is_valid = true;
    return frame;
}
