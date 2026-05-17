#pragma once
#include "../WaferRobotController/RobotClientDispatcher.h"
#include "CommandRegistry.h"

void dispatch_server_frame(RobotFrame frame, CommandRegistry& registry);