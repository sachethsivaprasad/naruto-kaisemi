#pragma once
#include <cstddef>
#include <string>
#include <string_view>

// Buffers incoming serial bytes until a complete CRLF (or LF) terminated line exists.
class SerialLineAccumulator {
public:
    static constexpr std::size_t kMaxBufferedLine = 8192;

    enum class PushResult { Incomplete, Ready, Overflow };

    [[nodiscard]] PushResult push_bytes(std::string_view incoming, std::string& out_completed_line);

    void clear() { buffer_.clear(); }

private:
    std::string buffer_;
};
