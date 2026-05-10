#pragma once

#include "RobotProtocol.h"
#include <functional>
#include <string_view>

struct SimulatorIo {
    std::function<bool(std::string_view)> send_raw{};
};

void dispatch_server_frame(SimulatorIo& io, RobotFrame frame);
