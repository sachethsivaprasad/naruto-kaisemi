#include "RobotProtocol.h"
#include "RobotSimDispatcher.h"
#include "SerialLineAccumulator.h"
#include "Utf8Argv.h"
#include "Win32SerialPort.h"

#include <array>
#include <atomic>
#include <cctype>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>

namespace {

std::atomic_bool g_stop{false};

std::string Trim(std::string_view text) {
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) {
        text.remove_prefix(1);
    }
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
        text.remove_suffix(1);
    }
    return std::string(text);
}

void Usage() {
    std::cerr << "WaferRobotSimulator.exe <COM> [--csv <path>]\n"
              << "Pair this port with the client using a virtual COM utility.\n"
              << "Quit with q + Enter.\n";
}

void ReaderMain(Win32SerialPort& port, SimulatorIo io) {
    SerialLineAccumulator accumulator;
    std::array<std::byte, 2048> scratch{};

    while (!g_stop.load(std::memory_order_acquire)) {
        const size_t rx =
            port.read_some(reinterpret_cast<void*>(scratch.data()), scratch.size());

        if (g_stop.load(std::memory_order_acquire)) {
            break;
        }
        if (rx == 0) {
            std::this_thread::yield();
            continue;
        }

        std::string line;
        const std::string_view chunk(reinterpret_cast<const char*>(scratch.data()), rx);
        SerialLineAccumulator::PushResult result = accumulator.push_bytes(chunk, line);

        bool keep_going = true;
        while (keep_going && !g_stop.load(std::memory_order_acquire)) {
            switch (result) {
                case SerialLineAccumulator::PushResult::Overflow:
                    accumulator.clear();
                    std::cerr << "[SERVER] Line length exceeded limit; buffer cleared.\n";
                    keep_going = false;
                    break;
                case SerialLineAccumulator::PushResult::Incomplete:
                    keep_going = false;
                    break;
                case SerialLineAccumulator::PushResult::Ready: {
                    const RobotFrame frame = parse_incoming_frame(line);
                    dispatch_server_frame(io, frame);
                    result = accumulator.push_bytes({}, line);
                    break;
                }
            }
        }
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        Usage();
        return 1;
    }

    if (argc >= 4) {
        std::string flag(argv[2]);
        for (char& ch : flag) {
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }
        if (flag != "--csv") {
            Usage();
            return 2;
        }
        std::cerr << "[SERVER] CSV scripting not implemented yet; argument ignored.\n";
    }

    Win32SerialPort port;
    const std::wstring device =
        NormalizeComPortArg(NarrowConsoleArgToWide(std::string_view(argv[1])));
    if (device.empty() || !port.open(device)) {
        Usage();
        return 3;
    }

    SimulatorIo bridge{};
    bridge.send_raw = [&](std::string_view bytes) noexcept -> bool {
        return port.write_all(bytes);
    };

    std::cout << "[SERVER] UART robot simulator on ";
    std::wcout << device << L"\n";

    std::thread reader([&]() { ReaderMain(port, bridge); });

    std::cerr << "> q + Enter stops the simulator.\n";

    std::string command;
    while (std::getline(std::cin, command)) {
        command = Trim(command);
        if (command.empty()) {
            continue;
        }
        for (char& ch : command) {
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        }
        if (command == "q" || command == "quit") {
            break;
        }
    }

    g_stop.store(true, std::memory_order_release);
    port.close();
    reader.join();
    std::cerr << "[SERVER] Shutdown complete.\n";
    return 0;
}
