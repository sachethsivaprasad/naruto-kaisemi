#pragma once

#include <Windows.h>

#include <string>
#include <string_view>

inline std::wstring NarrowConsoleArgToWide(std::string_view ascii) {
    if (ascii.empty()) {
        return L"";
    }
    auto size_needed = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                           ascii.data(),
                                           static_cast<int>(ascii.size()), nullptr,
                                           0);
    if (size_needed <= 0) {
        size_needed =
            MultiByteToWideChar(CP_ACP, 0, ascii.data(), static_cast<int>(ascii.size()),
                                nullptr, 0);
    }
    if (size_needed <= 0) {
        return {};
    }

    std::wstring out(static_cast<size_t>(size_needed), L'\0');
    int ok =
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, ascii.data(),
                            static_cast<int>(ascii.size()), out.data(),
                            static_cast<int>(out.size()));
    if (ok == 0) {
        ok =
            MultiByteToWideChar(CP_ACP, 0, ascii.data(), static_cast<int>(ascii.size()),
                                out.data(), static_cast<int>(out.size()));
    }
    if (ok <= 0) {
        return {};
    }
    out.resize(static_cast<size_t>(ok));
    return out;
}
