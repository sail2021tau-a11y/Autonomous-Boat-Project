### This hardware validation branch was developed to execute tests and verify the hardware team's deliverables at December 2025.
### Written by the Tal & Tal software team.

# Autonomous Boat Control System

This project implements a hybrid control and navigation system for an autonomous surface vessel. It uses a **Master-Slave architecture** combined with a manual **RC Override**:
* **Master (Jetson Orin):** Handles GPS navigation, path planning, and sends logic commands.
* **Slave (Arduino Nano 33 IoT):** Actuates motors and servos based on either RC input or Jetson commands.
* **RC Controller:** Provides manual control and a physical safety switch for mode selection.

## System Architecture

### Hardware Flow
1.  **Manual Mode:** `[RC Transmitter]` -> `[RC Receiver]` -> **Arduino** -> `[ESCs & Servos]`
2.  **Auto Mode:** `[GPS]` -> **Jetson Orin** -> `[USB Serial]` -> **Arduino** -> `[ESCs & Servos]`

### Logic & Safety
The system state is determined by **Channel 5** on the RC Transmitter:
* **Switch LOW (Manual):** The Arduino ignores the Jetson and follows the pilot's stick inputs directly.
* **Switch HIGH (Autonomous):** The Arduino listens to serial commands from the Jetson (Python script).

## Hardware Connection (Pinout)

### Actuators (Outputs)
* **Left Motor (ESC):** Pin `D9`
* **Right Motor (ESC):** Pin `D10`
* **Middle/Back Motor (ESC):** Pin `D8`
* **Steering Servo:** Pin `D3`

### RC Receiver Inputs (RadioLink R8EF - Red LED/PWM Mode)
Connect the signal pins from the receiver to the Arduino Analog pins:
* **CH 4 (Left Stick Horizontal):** Pin `A0` (Steering)
* **CH 2 (Left Stick Vertical):** Pin `A1` (Main Throttle)
* **CH 1 (Right Stick Horizontal):** Pin `A2` (Middle Motor)
* **CH 5 (2-Way Switch):** Pin `A3` (Mode Select)

### Sensors & Compute
* **GPS Module:** U-blox USB GPS connected to Jetson.
* **Communication:** Arduino connected to Jetson via USB (`/dev/ttyACM0`).

## Software Prerequisites

### 1. Jetson Orin (Python Environment)
* Python 3.x
* Required libraries:
    ```bash
    pip install pyserial pynmea2
    ```

### 2. Arduino Nano (Firmware)
* **Board:** Arduino Nano 33 IoT
* **Firmware Version:** V22 (Full Speed Autonomous)
* **Libraries:** `Servo.h` (Built-in)

## Installation & Setup

1.  **Flash the Firmware:**
    * Open `boat_firmware.ino` in Arduino IDE.
    * Connect the Arduino Nano via USB.
    * Upload the V22 code.
    * **Calibration:** Verify servo angles and motor directions in `runManualLoop`.

2.  **Configure the Jetson:**
    * Clone this repository.
    * Verify device paths (Arduino is usually `/dev/ttyACM0`).

## Usage

### Mode A: Manual Control (RC)
This is the default safety mode.
1.  Turn on the RC Transmitter.
2.  **Toggle Switch (CH5)** to the **LOW** position.
3.  Use the **Left Stick** for Throttle and Steering.
4.  Use the **Right Stick** for the Middle Motor (Strafe/Rotate).

### Mode B: Autonomous Navigation (GPS)
1.  **Set Target:** Edit `simple_gps_nav.py`:
    ```python
    TARGET_LAT = 32.085300
    TARGET_LON = 34.781800
    ```
2.  **Launch:**
    * Place boat in water.
    * **Toggle Switch (CH5)** to the **HIGH** position (Auto Mode).
    * Run the script:
    ```bash
    sudo python3 simple_gps_nav.py
    ```

### Serial Control Protocol
When in Autonomous Mode, the Arduino accepts these characters:
* `F` - Forward (Max Speed)
* `B` - Backward (Max Speed)
* `L` - Turn Left (Servo Max Angle)
* `R` - Turn Right (Servo Min Angle)
* `C` - Center Steering & Stop Middle Motor
* `Z` - Rotate Left (Middle Motor only)
* `X` - Rotate Right (Middle Motor only)
* `S` - Stop All Motors

## Safety
* **Emergency Stop:** Immediately toggle the RC Switch (CH5) to **LOW** to regain manual control.
* **Prop Safety:** Always test with propellers removed or on a stand.
