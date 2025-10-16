# Real-Time Location System (RTLS) with UWB and ESP-NOW
This project implements a Real-Time Location System (RTLS) using ESP32 microcontrollers, the Decawave DW1000 UWB module for precise ranging, and ESP-NOW for robust, short-range, peer-to-peer wireless communication.

## The system is designed with two distinct components:

The Anchor: This device is fixed in a known location. Its primary job is to listen for ranging requests from the Tag, calculate the precise distance, and remain open to receiving simple data messages.
The Tag: This is the mobile component that actively initiates the ranging process. It determines its distance from the Anchor and can also send quick, low-latency messages to other devices using ESP-NOW.
