#include "SerialLink.h"
#include "RobotProtocol.h"
#include <Windows.h>

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

    bool configure_serial_port(HANDLE serial_handle, const string& normalized_port, string& error_message) {
        DCB serial_config = {};
        serial_config.DCBlength = sizeof(serial_config);

        if (!GetCommState(serial_handle, &serial_config)) {
            error_message = "Failed to read serial settings for " + normalized_port + ": " + format_windows_error(GetLastError());
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
            return false;
        }

        COMMTIMEOUTS timeouts = {};
        timeouts.WriteTotalTimeoutConstant = 1000;
        timeouts.WriteTotalTimeoutMultiplier = 10;

        if (!SetCommTimeouts(serial_handle, &timeouts)) {
            error_message = "Failed to configure serial timeouts for " + normalized_port + ": " + format_windows_error(GetLastError());
            return false;
        }

        return true;
    }
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

//Frame Sender
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

    if (!configure_serial_port(serial_handle, normalized_port, error_message)) {
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

//Frame Receiver
bool receive_robot_frame(const string& com_port, string& frame, string& error_message) {
    frame.clear();
    error_message.clear();

    string normalized_port = normalize_com_port_name(com_port);
    HANDLE serial_handle = CreateFileA(
        normalized_port.c_str(),
        GENERIC_READ,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (serial_handle == INVALID_HANDLE_VALUE) {
        error_message = "Failed to open " + normalized_port + ": " + format_windows_error(GetLastError());
        return false;
    }

    if (!configure_serial_port(serial_handle, normalized_port, error_message)) {
        CloseHandle(serial_handle);
        return false;
    }

    bool frame_started = false;
    char previous_byte = '\0';

    while (true) {
        char current_byte = '\0';
        DWORD bytes_read = 0;
        BOOL read_result = ReadFile(
            serial_handle,
            &current_byte,
            1,
            &bytes_read,
            nullptr);

        if (!read_result) {
            error_message = "Failed to read frame from " + normalized_port + ": " + format_windows_error(GetLastError());
            CloseHandle(serial_handle);
            return false;
        }

        if (bytes_read == 0) {
            continue;
        }

        if (!frame_started) {
            if (current_byte != '\x01') {
                continue;
            }

            frame_started = true;
        }

        frame.push_back(current_byte);

        if (previous_byte == '\r' && current_byte == '\n') {
            CloseHandle(serial_handle);
            return true;
        }

        previous_byte = current_byte;
    }
}
