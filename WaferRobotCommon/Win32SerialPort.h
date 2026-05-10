#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <mutex>
#include <string>
#include <string_view>

// Minimal blocking Win32 UART for lab / virtual COM pairs (defaults: 115200 8N1).
class Win32SerialPort {
public:
    Win32SerialPort();
    Win32SerialPort(const Win32SerialPort&) = delete;
    Win32SerialPort& operator=(const Win32SerialPort&) = delete;
    Win32SerialPort(Win32SerialPort&&) noexcept;
    Win32SerialPort& operator=(Win32SerialPort&&) noexcept;
    ~Win32SerialPort();

    [[nodiscard]] bool open(const std::wstring& device_path, DWORD baud_rate = 115200);
    void close();

    [[nodiscard]] bool is_open() const { return handle_ != INVALID_HANDLE_VALUE; }

    // Blocks until bytes are returned or timeouts elapse per COMMTIMEOUTS.
    // Returns bytes read into buffer prefix, or 0 on timeout / graceful end.
    [[nodiscard]] size_t read_some(void* buf, size_t capacity);

    [[nodiscard]] bool write_all(std::string_view data);

private:
    HANDLE handle_{INVALID_HANDLE_VALUE};
    mutable std::mutex write_mutex_;
};

[[nodiscard]] std::wstring NormalizeComPortArg(std::wstring_view arg);
