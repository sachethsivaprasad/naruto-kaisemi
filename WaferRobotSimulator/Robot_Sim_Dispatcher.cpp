#include "RobotSimDispatcher.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

using namespace std;

namespace {

mutex g_busy_mutex{};
atomic<bool> g_busy{false};
atomic<int> g_last_cmd_seq{-1};

[[nodiscard]] uint32_t dword_from_bits(bool error, bool busy, uint32_t code14) noexcept {
    const uint32_t masked = code14 & 0x3FFFu;
    uint32_t dword = masked << 2u;
    if (busy) {
        dword |= (1u << 1);
    }
    if (error) {
        dword |= 1u;
    }
    return dword;
}

[[nodiscard]] string dword_string(bool error, bool busy, uint32_t code14) {
    const uint32_t d = dword_from_bits(error, busy, code14);
    return std::to_string(static_cast<unsigned long>(d));
}

void schedule_evt_after_delay(SimulatorIo io, int cmd_seq, uint32_t idle_dword) {
    thread(
        [](SimulatorIo copied_io, const int seq, const uint32_t finished_dword,
           const std::chrono::milliseconds delay) mutable {
            this_thread::sleep_for(delay);

            bool send_ok =
                copied_io.send_raw(build_frame(
                    "EVT", seq,
                    std::to_string(static_cast<unsigned long>(finished_dword))));

            if (!send_ok) {
                cerr << "[SERVER] EVT send failed (port closed?)." << endl;
            }

            {
                lock_guard lk(g_busy_mutex);
                g_busy.store(false);
            }
            g_last_cmd_seq.store(seq);

        },
        std::move(io), cmd_seq, idle_dword, chrono::milliseconds(350))
        .detach();
}

} // namespace

void dispatch_server_frame(SimulatorIo& io, RobotFrame frame) {

    if (!frame.is_valid) {
        cout << "[SERVER WARNING] Frame rejected by Protocol Parser (Checksum/Format Error)."
             << endl;
        return;
    }

    // Router
    if (frame.message_type == "CMD") {
        cout << "\n>>> [ROUTED TO: COMMAND EXECUTION] <<<" << endl;
        cout << "Sequence ID : " << frame.sequence_id << endl;
        cout << "Raw Payload : " << frame.payload << endl;

        lock_guard lk(g_busy_mutex);

        bool expected = false;
        if (!g_busy.compare_exchange_strong(expected, true)) {
            cerr << "[SERVER] Prior command still simulated as busy." << endl;
        }

        g_last_cmd_seq.store(frame.sequence_id);

        bool ack_ok =
            io.send_raw(build_frame("ACK", frame.sequence_id, ""));
        if (!ack_ok) {
            cerr << "[SERVER] ACK send failed." << endl;
            g_busy.store(false);
            return;
        }

        SimulatorIo copied = io;

        schedule_evt_after_delay(
            std::move(copied),
            frame.sequence_id,
            dword_from_bits(false, false, 0));

    } else if (frame.message_type == "STAT") {
        cout << "\n>>> [ROUTED TO: STATUS REPORTING] <<<" << endl;
        cout << "Sequence ID : " << frame.sequence_id << endl;

        const uint32_t code14 =
            g_busy.load() ? 1u : 0u; // simple lab placeholder for status code bucket
        const string mid_dword_string =
            dword_string(false, g_busy.load(), code14);

        bool stat_ok =
            io.send_raw(build_frame("STAT", frame.sequence_id, mid_dword_string));
        if (!stat_ok) {
            cerr << "[SERVER] STAT response send failed." << endl;
        }

    } else {
        cout << "\n[SERVER ERROR] Received illegal message type for a Server: "
             << frame.message_type << endl;
    }
}
