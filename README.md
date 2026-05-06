# Arduino Firmware: Low-Level Propulsion Controller

This subdirectory contains the official Arduino Nano firmware (`boat_engines_serial.ino`) responsible for translating high-level directional directives from the NVIDIA Jetson (ROS 2 environment) into physical motor movement.

## Hardware Architecture & Pin Mapping
Based on meticulous validation against the final KiCad electrical schematic, the electronic speed controllers (ESCs) are routed to the following hardware PWM pins on the Arduino Nano:
* **Left Thruster (ESC 1):** `Pin 9` (PWM)
* **Right Thruster (ESC 2):** `Pin 10` (PWM)
* **Back Thruster (ESC 3):** `Pin 8` (PWM) — *Fixed from previous software misconfigurations routing to Pin 11.*

## Key Functional Features

### 1. ESC Arming Sequence (Safety First)
Electronic Speed Controllers feature hardware-level safety initialization routines to avoid accidental bench-testing injuries. Upon boot/reset, the firmware triggers a **3-second arming delay**, continuous feeding a neutral signal of `1500 microseconds` to all channels. This safely commands zero-throttle, allowing the ESCs to boot, self-calibrate, and unlock propulsion lines.

### 2. Low-Level Control Protocol (Servo / RC Signaling)
Standard brushless ESCs reject conventional microcontroller `analogWrite` PWM signals due to high-frequency incompatibilities. This firmware leverages the native `<Servo.h>` library, outputting standard RC servo pulses running at a 50Hz refresh cycle:
* `1500 us` = Dead Neutral / Engines Stopped
* `2000 us` = Full Throttle Forward

### 3. Integrated Bench-Testing Safety Cap
To protect researchers, lab tools, and mechanical linkages from unexpected autonomous software spikes, a **Hard Safety Cap** is hardcoded directly into the microcontroller logic:
* `MAX_TEST_POWER = 15` (15% maximum power scale).
* Even if high-level ROS 2 navigation scripts malfunction and request 100% full throttle, the Arduino acts as a hardware firewall, strictly clamping incoming inputs to safe, low-torque laboratory limits.

### 4. Serial Packet Parser
The firmware opens a hardware serial connection running at a baud rate of `9600`. It processes incoming asynchronous string buffers terminated by a newline (`\n`) character.
* **Expected Format:** `l<left_power>,r<right_power>,f<forward_power>`
* **Example Payload:** `l50,r50,f0` (Commands left/right motors to safe-capped 15% power, keeping the front/back thruster neutral).
