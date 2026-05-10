#include "RobotClientDispatcher.h"
#include "RobotProtocol.h"
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

/*
  Run this on the complementary half of your virtual COM pair.
  Typical flow (after splitter maps COM10 <-> COM11):
      WaferRobotSimulator.exe COM11
      WaferRobotClient.exe COM10
*/

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

void Help() {
    std::cerr
        << "WaferRobotClient.exe <COM>\n"
        << "\nInteractive commands:\n"
        << "  Type natural language payloads -> packaged as CMD|seq|<payload>|chk\n"
        << "    Example: pick from=lptest arm=lower\n"
        << "  poll                 -> emits STAT polling frame with its own rolling seq#\n"
        << "  help                 -> recap these hints\n"
        << "  exit | quit          -> cleanly stop reader thread\n"
        << "\n";
}

void ReaderProcess(Win32SerialPort& port) {
    SerialLineAccumulator accumulator;
    std::array<std::byte, 2048> scratch{};

    while (!g_stop.load(std::memory_order_acquire)) {
        const size_t bytes =
            port.read_some(reinterpret_cast<void*>(scratch.data()), scratch.size());
        if (g_stop.load(std::memory_order_acquire)) {
            break;
        }
        if (bytes == 0) {
            std::this_thread::yield();
            continue;
        }

        std::string line;
        const std::string_view chunk(reinterpret_cast<const char*>(scratch.data()), bytes);
        SerialLineAccumulator::PushResult status = accumulator.push_bytes(chunk, line);

        bool keep_going = true;
        while (keep_going && !g_stop.load(std::memory_order_acquire)) {
            switch (status) {
                case SerialLineAccumulator::PushResult::Overflow:
                    accumulator.clear();
                    std::cerr << "[CLIENT] Oversized inbound line; resetting buffer.\n";
                    keep_going = false;
                    break;
                case SerialLineAccumulator::PushResult::Incomplete:
                    keep_going = false;
                    break;
                case SerialLineAccumulator::PushResult::Ready: {

                    dispatch_client_frame(parse_incoming_frame(line));


                    status = accumulator.push_bytes({}, line);
                    break;
                }
            }
        }
    }
}

} // namespace

int main(int argc, char** argv) {


    if (argc < 2) {
        Help();
        return 1;
    }

    Win32SerialPort uart;


    const std::wstring device_path =
        NormalizeComPortArg(NarrowConsoleArgToWide(std::string_view(argv[1])));

        

    if (device_path.empty() || !uart.open(device_path)) {
        Help();




        std::cerr << "[CLIENT] Unable to open serial port argument.\n";
        return 2;
    }



    std::cerr << "[CLIENT] Connected:";
    std::wcout << L' ' << device_path << L'\n';


    std::cerr << "(type help for shortcuts)\n";

    std::thread listener([&]() { ReaderProcess(uart); });



    

    std::cerr << "> ";

    

    std::string user_line;

    long long cmd_seq = 100;

    long long stat_seq = 500;

    

    while (std::getline(std::cin, user_line)) {

        std::string cleaned = Trim(user_line);
        if (cleaned.empty()) {
            std::cerr << "> ";
            continue;



        }



        std::string lowered = cleaned;



        for (char& letter : lowered) {
            letter = static_cast<char>(std::tolower(static_cast<unsigned char>(letter)));





        }



        if (lowered == "exit" || lowered == "quit") {
            break;
        }


        if (lowered == "help") {


            Help();
            std::cerr << "> ";

            continue;
        }


        std::string frame;

        if (lowered == "poll") {


            ++stat_seq;
            frame = build_frame("STAT",


                                static_cast<int>(stat_seq),


                                "");

        }



        else {
            ++cmd_seq;




            frame = build_frame("CMD", static_cast<int>(cmd_seq),

                                cleaned);



        }



        if (!uart.write_all(frame)) {
            std::cerr << "[CLIENT] Write failed.\n";


            break;
        }


        std::cerr << "> ";



    

    }



    g_stop.store(true, std::memory_order_release);



    uart.close();

    listener.join();



    

    std::cerr << "[CLIENT] Session ended cleanly.\n";



    

    return 0;


}
