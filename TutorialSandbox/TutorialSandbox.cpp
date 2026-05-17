#include <iostream>
#include "CommandRegistry.h"

using namespace std;

int main() {
    CommandRegistry registry;

    cout << "=== BOOT SEQUENCE ===" << endl;
    // 1. Load the database into RAM
    if (!registry.load_from_cfg("robot.cfg")) {
        cout << "Test aborted: CFG file not found!" << endl;
        return -1;
    }

    // --- SCENARIO 1: The Perfect Command ---
    // User types: PICK FROM=LPA1
    ParsedCommand cmd1;
    cmd1.action = "PICK";
    cmd1.parameters["FROM"] = "LPA2";
    cmd1.parameters["ARM"] = "LOWER";


    cout << "\n--- Test 1: Perfect Command ---" << endl;
    if (registry.validate_syntax(cmd1))
    {
        if (registry.validate_constraints(cmd1)) cout << "RESULT: ACCEPTED (Passed)\n";
	    else cout << "RESULT: REJECTED Hardware Constraint Failed (Failed)\n";
    }
    else cout << "RESULT: REJECTED Command Error (Failed)\n";

	


    // --- SCENARIO 2: Missing Required Parameter ---
    // User types: PLACE ARM=LOWER  (They forgot the 'TO=' parameter!)
    ParsedCommand cmd2;
    cmd2.action = "PLACE";
    cmd2.parameters["ARM"] = "LOWER";

    cout << "\n--- Test 2: Missing Parameter ---" << endl;
    if (registry.validate_syntax(cmd2))
    {
        if (registry.validate_constraints(cmd2)) cout << "RESULT: ACCEPTED (Passed)\n";
        else cout << "RESULT: REJECTED Hardware Constraint Failed (Failed)\n";
    }
    else cout << "RESULT: REJECTED Command Error (Failed)\n";


    // --- SCENARIO 3: Unknown Action ---
    // User types: DANCE SPEED=FAST
    ParsedCommand cmd3;
    cmd3.action = "DANCE";
    cmd3.parameters["SPEED"] = "FAST";

    cout << "\n--- Test 3: Unknown Action ---" << endl;
    if (registry.validate_syntax(cmd3))
    {
        if (registry.validate_constraints(cmd3)) cout << "RESULT: ACCEPTED (Passed)\n";
        else cout << "RESULT: REJECTED Hardware Constraint Failed (Failed)\n";
    }
    else cout << "RESULT: REJECTED Command Error (Failed)\n";


    // --- SCENARIO 4: Extra Optional Parameters ---
    // User types: MOVE ARM=LOWER R=10 T=20 Z=30 W=40 SPEED=FAST
    // (SPEED is an extra optional parameter that isn't required)
    ParsedCommand cmd4;
    cmd4.action = "MOVE";
    cmd4.parameters["ARM"] = "LOWER";
    cmd4.parameters["R"] = "10";
    cmd4.parameters["T"] = "20";
    cmd4.parameters["Z"] = "30";
    cmd4.parameters["W"] = "40";
    cmd4.parameters["SPEED"] = "FAST";

    cout << "\n--- Test 4: Extra Optional Params ---" << endl;
    if (registry.validate_syntax(cmd4))
    {
        if (registry.validate_constraints(cmd4)) cout << "RESULT: ACCEPTED (Passed)\n";
        else cout << "RESULT: REJECTED Hardware Constraint Failed (Failed)\n";
    }
    else cout << "RESULT: REJECTED Command Error (Failed)\n";

    return 0;
}