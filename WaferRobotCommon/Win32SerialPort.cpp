#define NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "Win32SerialPort.h"
#include <algorithm>
#include <cwctype>
#include <limits>

Win32SerialPort::Win32SerialPort() = default;

Win32SerialPort::~Win32SerialPort() {
    close();
}

Win32SerialPort::Win32SerialPort(Win32SerialPort&& other) noexcept {
    *this = std::move(other);
}

Win32SerialPort& Win32SerialPort::operator=(Win32SerialPort&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    close();
    handle_ = other.handle_;
    other.handle_ = INVALID_HANDLE_VALUE;
    return *this;
}

void Win32SerialPort::close() {
    if (handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
    }
}

std::wstring NormalizeComPortArg(std::wstring_view arg) {
    if (arg.empty()) {
        return L"";
    }
    std::size_t begin = 0;
    std::size_t finish = arg.size();
    while (begin < finish && std::isspace(static_cast<unsigned char>(arg[begin]))) {
        ++begin;
    }
    while (finish > begin && std::isspace(static_cast<unsigned char>(arg[finish - 1]))) {
        --finish;
    }

    constexpr wchar_t kPrefix[] = L"\\\\.\\";
    if (finish > begin &&
        finish - begin >= 4 &&
        arg[begin + 0] == L'\\' && arg[begin + 1] == L'\\' &&
        arg[begin + 2] == L'.' && arg[begin + 3] == L'\\') {
        std::wstring full(arg.substr(begin, finish - begin));
        for (wchar_t& c : full) {
            c = static_cast<wchar_t>(std::towupper(static_cast<wint_t>(c)));
        }
        return full;
    }

    std::wstring name(arg.substr(begin, finish - begin));
    for (wchar_t& ch : name) {
        ch = static_cast<wchar_t>(std::towupper(static_cast<wint_t>(ch)));
    }

    bool all_digits = true;
    for (wchar_t c : name) {
        if (std::iswdigit(c) == 0) {
            all_digits = false;
            break;
        }
    }
    if (all_digits) {
        std::wstring out;
        out.reserve(std::wcslen(kPrefix) + 3 + name.size());
        out.append(kPrefix);
        out.append(L"COM");
        out.append(name);
        return out;
    }

    if (name.size() >= 3 && name[0] == L'C' && name[1] == L'O' && name[2] == L'M') {
        std::wstring out;
        out.reserve(std::wcslen(kPrefix) + name.size());
        out.append(kPrefix);
        out.append(name);
        return out;
    }

    std::wstring out;
    out.reserve(std::wcslen(kPrefix) + name.size());
    out.append(kPrefix);
    out.append(name);
    return out;
}

bool Win32SerialPort::open(const std::wstring& device_path, DWORD baud_rate) {
    close();

    HANDLE raw =
        CreateFileW(device_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (raw == INVALID_HANDLE_VALUE) {
        return false;
    }

    COMMTIMEOUTS timeouts{};
    timeouts.ReadIntervalTimeout = 40;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 2000;

    if (!SetupComm(raw, static_cast<DWORD>(64 * 1024), static_cast<DWORD>(64 * 1024))) {
        CloseHandle(raw);
        return false;
    }
    if (!SetCommTimeouts(raw, &timeouts)) {
        CloseHandle(raw);
        return false;
    }

    DCB dcb{};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(raw, &dcb)) {
        CloseHandle(raw);
        return false;
    }

    dcb.BaudRate = baud_rate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary = TRUE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    if (!SetCommState(raw, &dcb)) {
        CloseHandle(raw);
        return false;
    }

    handle_ = raw;
    return true;
}

size_t Win32SerialPort::read_some(void* buf, size_t capacity) {
    if (handle_ == INVALID_HANDLE_VALUE || capacity == 0 || buf == nullptr) {
        return 0;
    }
    DWORD received = 0;
    constexpr DWORD kMaxDWORD = (std::numeric_limits<DWORD>::max)();
    const DWORD chunk =
        capacity > static_cast<size_t>(kMaxDWORD) ? kMaxDWORD : static_cast<DWORD>(capacity);
    BOOL ok = ReadFile(handle_, buf, chunk, &received, nullptr);
    return ok ? static_cast<size_t>(received) : 0;
}

bool Win32SerialPort::write_all(std::string_view data) {
    if (handle_ == INVALID_HANDLE_VALUE || data.empty()) {
        return false;
    }
    std::lock_guard lk(write_mutex_);

    std::size_t offset = 0;
    while (offset < data.size()) {
        DWORD chunk = static_cast<DWORD>(
            std::min<std::size_t>(data.size() - offset,
                                  static_cast<std::size_t>((std::numeric_limits<DWORD>::max)())));
        DWORD wrote = 0;
        BOOL ok = WriteFile(handle_, data.data() + offset, chunk, &wrote, nullptr);
        if (!ok || wrote == 0) {
            return false;
        }
        offset += static_cast<std::size_t>(wrote);
    }

    FlushFileBuffers(handle_);
    return true;
}
