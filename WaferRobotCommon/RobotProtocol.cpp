#include "RobotProtocol.h"
#include <Windows.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

namespace {
    string normalize_com_port_name(const string& com_port) {
        if (com_port.rfind("\\\\.\\", 0) == 0) {
            return com_port;
        }

        return "\\\\.\\" + com_port;
    }

    string format_windows_error(DWORD error_code) {
        LPSTR message_buffer = nullptr;
        DWORD size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            error_code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPSTR>(&message_buffer),
            0,
            nullptr);

        if (size == 0 || message_buffer == nullptr) {
            return "Windows error code " + to_string(error_code);
        }

        string message(message_buffer, size);
        LocalFree(message_buffer);

        while (!message.empty() && (message.back() == '\r' || message.back() == '\n' || message.back() == ' ')) {
            message.pop_back();
        }

        return message;
    }
}

//Checksum Calculator
string calculate_checksum(string data) {
    unsigned char checksum = 0;
    for (char c : data) {
        checksum ^= c;
    }
    stringstream hex_stream;
    hex_stream << uppercase << hex << setw(2) << setfill('0') << (int)checksum;
    return hex_stream.str();
}

//Frame Builder
string build_robot_frame(const string& message_type, int sequence_id, const string& payload) {
    string core_data = message_type + "|" + to_string(sequence_id);

    if (!payload.empty()) {
        core_data += "|" + payload;
    }

    string checksum = calculate_checksum(core_data);
    return "\x01" + core_data + "|" + checksum + "\r\n";
}

bool send_robot_frame(const string& com_port, const string& frame, string& error_message) {
    error_message.clear();

    string normalized_port = normalize_com_port_name(com_port);
    HANDLE serial_handle = CreateFileA(
        normalized_port.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (serial_handle == INVALID_HANDLE_VALUE) {
        error_message = "Failed to open " + normalized_port + ": " + format_windows_error(GetLastError());
        return false;
    }

    DCB serial_config = {};
    serial_config.DCBlength = sizeof(serial_config);

    if (!GetCommState(serial_handle, &serial_config)) {
        error_message = "Failed to read serial settings for " + normalized_port + ": " + format_windows_error(GetLastError());
        CloseHandle(serial_handle);
        return false;
    }

    serial_config.BaudRate = CBR_9600;
    serial_config.ByteSize = 8;
    serial_config.Parity = NOPARITY;
    serial_config.StopBits = ONESTOPBIT;
    serial_config.fOutxCtsFlow = FALSE;
    serial_config.fOutxDsrFlow = FALSE;
    serial_config.fDtrControl = DTR_CONTROL_DISABLE;
    serial_config.fRtsControl = RTS_CONTROL_DISABLE;
    serial_config.fOutX = FALSE;
    serial_config.fInX = FALSE;

    if (!SetCommState(serial_handle, &serial_config)) {
        error_message = "Failed to configure " + normalized_port + " as 9600 8N1: " + format_windows_error(GetLastError());
        CloseHandle(serial_handle);
        return false;
    }

    COMMTIMEOUTS timeouts = {};
    timeouts.WriteTotalTimeoutConstant = 1000;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(serial_handle, &timeouts)) {
        error_message = "Failed to configure write timeout for " + normalized_port + ": " + format_windows_error(GetLastError());
        CloseHandle(serial_handle);
        return false;
    }

    DWORD bytes_written = 0;
    BOOL write_result = WriteFile(
        serial_handle,
        frame.data(),
        static_cast<DWORD>(frame.size()),
        &bytes_written,
        nullptr);

    if (!write_result) {
        error_message = "Failed to write frame to " + normalized_port + ": " + format_windows_error(GetLastError());
        CloseHandle(serial_handle);
        return false;
    }

    if (bytes_written != frame.size()) {
        error_message = "Only wrote " + to_string(bytes_written) + " of " + to_string(frame.size()) + " frame bytes to " + normalized_port;
        CloseHandle(serial_handle);
        return false;
    }

    CloseHandle(serial_handle);
    return true;
}

//Parser
RobotFrame parse_incoming_frame(string raw_data) {
    RobotFrame frame;
    frame.is_valid = false;

    string clean_string = raw_data.substr(1, raw_data.length() - 3);
    stringstream ss(clean_string);
    string token;
    vector<string> parts;

    while (getline(ss, token, '|')) {
        parts.push_back(token);
    }

    if (parts.size() == 3 || parts.size() == 4) {
        frame.message_type = parts[0];
        frame.sequence_id = stoi(parts[1]);
        frame.payload = parts.size() == 4 ? parts[2] : "";
        frame.hex_checksum = parts.size() == 4 ? parts[3] : parts[2];

        string data_to_check = parts[0] + "|" + parts[1];
        if (!frame.payload.empty()) {
            data_to_check += "|" + frame.payload;
        }
        string calculated_hex = calculate_checksum(data_to_check);

        if (calculated_hex == frame.hex_checksum) {
            frame.is_valid = true;
        }
        else {
            cout << "[ERROR] Checksum Mismatch!" << endl;
        }
    }
    else {
        cout << "[ERROR] Malformed frame received!" << endl;
    }

    return frame;
}
