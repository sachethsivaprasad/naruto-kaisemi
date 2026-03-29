# Wafer Handler Robot - C++20 Proof of Concept

## Project Overview
This project is a Proof of Concept (PoC) for a semiconductor WIP (Wafer In Process) robotic handling system, specifically targeting the kinematics of an Ion Implanter robot. It features a decoupled Client/Server architecture where a Controller communicates with a Robot Simulator over a Virtual COM Port using a custom ASCII protocol.

## Key Features
* **Modern C++ Architecture:** Built using C++20, utilizing modern concurrency (`std::jthread`) for non-blocking asynchronous hardware polling.
* **RTZW Kinematics Simulation:** Simulates Radial, Theta, Z (elevation/micro-moves), and Wrist movements for precision wafer transfers.
* **CSV-Driven Digital Twin:** Robot timing, physical constraints, and command durations are dynamically loaded from a CSV, allowing for deterministic hardware simulation without physical robots.
* **Multi-Mode End Effector:** Supports state management for both single-wafer handling and 5-wafer batch transfers.
* **Canonical Comms Protocol:** Implements an industry-standard `CMD / ACK / STAT` handshake protocol with continuous status polling and support for optional event-driven interrupt notifications.

## Deliverable Sequence
Demonstrates an end-to-end automated transfer sequence moving a wafer from Load Port A, through an Aligner station, into the Process Chamber, and back to storage.
