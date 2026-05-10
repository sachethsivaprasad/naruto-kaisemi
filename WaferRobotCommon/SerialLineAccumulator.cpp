#include "SerialLineAccumulator.h"

SerialLineAccumulator::PushResult SerialLineAccumulator::push_bytes(
    std::string_view incoming, std::string& out_completed_line) {
    buffer_.append(incoming);

    while (!buffer_.empty()) {
        if (buffer_.size() > kMaxBufferedLine) {
            buffer_.clear();
            return PushResult::Overflow;
        }

        const std::size_t nl = buffer_.find('\n');
        if (nl == std::string::npos) {
            return PushResult::Incomplete;
        }

        std::size_t chop = nl + 1;
        std::size_t logical_end = nl;
        if (logical_end > 0 && buffer_[logical_end - 1] == '\r') {
            --logical_end;
        }

        out_completed_line.assign(buffer_.data(), logical_end);
        buffer_.erase(0, chop);

        if (!out_completed_line.empty()) {
            return PushResult::Ready;
        }
    }

    return PushResult::Incomplete;
}
