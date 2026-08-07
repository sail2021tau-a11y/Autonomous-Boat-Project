# Autonomous Boat – System Engineering and Integrations

Final project in Electrical Engineering, Tel Aviv University  
**Project No. 25-1-1-3272**

This repository contains the software integration and control system developed for the **Sail-IL Autonomous Surface Vehicle (ASV)**. The platform is designed to perform autonomous marine navigation tasks inspired by the international RoboBoat competition.

The system runs on an **NVIDIA Jetson Orin** and uses **ROS 2 Foxy** to connect perception, navigation, monitoring, and motor-control nodes. A **ZED 2i stereo camera** and a **YOLOv8** model detect and localize objects in 3D, navigation nodes calculate steering commands, and an **Arduino Nano** converts the commands into PWM signals for three Blue Robotics T200 thrusters.

> ## ⚠️ Branch status: `fix/timeout-stop-signal-pending-water-test` — not yet merged, not yet water-tested
>
> This branch is **not** `AB_2026_Integration`. It contains one small, isolated code fix on top of the integration branch's current state, kept separate on purpose until it has been validated on the real boat:
>
> - **What changed:** `TimerManager.timer_callback()` in `task1_navigation_2025_ros2_eff.py`, `task2_navigation_2025_ros2_eff.py`, and `task3_navigation_2025_ros2_eff.py` now calls `self.node.publish_angle("stop")` before shutting the node down. Previously it only called `rclpy.shutdown()`, so a detection-loss timeout ended the navigation node without ever telling the engine controller to stop the motors — the boat would keep the last commanded thrust. Task 1's `start_timer(TIMEOUT)` call, which was commented out, is now enabled so Task 1 is covered by this timeout like Tasks 2 and 3 already were.
> - **What did *not* change:** everything else — no soft-start ramping, no RC override, no communication-loss watchdog were added. Those remain out of scope; see [Safety Notes](#safety-notes) and [Current Limitations](#current-limitations-and-important-notes) below, which are otherwise unchanged from `AB_2026_Integration`.
> - **Why it's separate:** this changes real motor-stop behavior in the field (a detection-loss timeout will now actually kill the thrusters, where before it silently didn't). It needs a bench test (props off / boat secured) and a real water test on Tasks 1–3 before merging into `AB_2026_Integration`.
> - **Diff scope:** four changed lines total across the three navigation files — nothing else in the codebase was touched.
>
> Once this has been tested and confirmed working, merge it into `AB_2026_Integration` and this callout should be deleted.

## Main Capabilities

- **Task 1 – Gate navigation:** calculates the midpoint between red and green buoys and steers through the gate.
- **Task 2 – Obstacle avoidance:** creates a virtual gate to perform a smooth avoidance maneuver around yellow obstacles.
- **Task 3 – Precision docking:** prioritizes docking symbols and stops the boat approximately one meter from the target.
- **Task 4 – GPS waypoint navigation:** calculates distance and bearing to a predefined coordinate using GPS data.
- Real-time object detection and 3D position estimation using YOLOv8 and the ZED point cloud.
- Differential thrust control for left, right, and front thrusters.
- Flask dashboard with telemetry, processed camera output, and a live 2D map.
- Safe task shutdown, emergency stop commands, and a three-second command-immunity period after stopping.

## System Architecture

```text
ZED 2i Camera / GPS
        │
        ▼
Perception Node (YOLOv8 + ZED point cloud)
        │  object_distance_info
        ▼
Navigation Node (Task 1 / 2 / 3 / 4)
        │  steering_directions
        ▼
Engine Controller Node
        │  Serial-over-USB: l<left>,r<right>,f<front>
        ▼
Arduino Nano → ESCs → Three T200 Thrusters
```

## Repository Structure

```text
.
├── publisher_cv_2025_ros2.py          # YOLOv8 + ZED perception node
├── task1_navigation_2025_ros2_eff.py  # Gate navigation
├── task2_navigation_2025_ros2_eff.py  # Obstacle avoidance
├── task3_navigation_2025_ros2_eff.py  # Precision docking
├── task4_navigation_2025_ros2_eff.py  # GPS waypoint navigation
├── engine_controller.py               # Steering-to-thruster conversion and serial output
├── server_ros2.py                     # Flask dashboard, telemetry, video and TTS
├── map_gui_viewer.py                  # PyQt5 live 2D object map
├── get_target_gps.py                  # Utility for reading target GPS coordinates
├── serialtest.py                      # Direct Arduino serial test
├── arduino_controllers/               # Arduino firmware
├── bashes4/                           # Current launch and shutdown scripts for Tasks 1–4 (use this one)
├── bashes/                            # Older duplicate script set (Tasks 1–3 only, no Task 4) — kept for reference
├── hardware_docs/                     # Wiring/block diagrams, KiCad files, and system diagrams
├── templates/index.html               # Web dashboard
└── frequencies/                       # Performance and frequency test results
```

`bashes/` is an earlier copy of the launch scripts and predates Task 4; it still points at its own copy of `start.py` and does not know about the GPS waypoint task. Use `bashes4/` for normal operation.

## Hardware

- NVIDIA Jetson Orin
- ZED 2i stereo camera
- Arduino Nano
- u-blox GPS receiver
- Three Blue Robotics T200 thrusters
- Three Blue Robotics Basic ESCs
- 12 V, 30 Ah LiFePO4 battery
- Serial connections:
  - Arduino: `/dev/ttyUSB1`
  - GPS: `/dev/ttyACM0`

The serial device names may change between systems. Verify them before running the software.

Wiring diagrams, hardware block diagrams, the ROS 2 node graph, and KiCad source files are in `hardware_docs/`.

## Dependencies

### Required platform software

- Ubuntu on NVIDIA Jetson Orin
- ROS 2 Foxy
- Python 3
- A ROS 2 workspace, currently referenced as `~/foxy_ws`
- `colcon` build tools
- ZED SDK with the Python API (`pyzed`)
- NVIDIA Jetson-compatible PyTorch installation
- Arduino IDE or Arduino CLI for uploading the firmware
- Linux utilities used by the scripts: `bash`, `curl`, `gnome-terminal`, `pkill`, `timeout`, and `stdbuf`

### ROS 2 packages

- `rclpy`
- `std_msgs`
- Custom message package: `cv_from_zed_ros2`
  - Required message: `cv_from_zed_ros2/msg/ObjectDistanceInfo`
  - Required fields: `label`, `distance_x`, `distance_y`, and `distance_z`

> The custom ROS 2 message package is expected to be installed in the ROS workspace. It is not included in this repository export.

### Python packages

The code imports the following third-party packages:

- `numpy`
- `opencv-python` / system OpenCV (`cv2`)
- `ultralytics`
- `torch`
- `requests`
- `flask`
- `pyserial`
- `pynmea2`
- `PyQt5`
- `pyautogui`
- `TTS` (Coqui TTS)
- `simpleaudio`

Install the versions compatible with the Jetson image and ROS 2 environment. No pinned `requirements.txt` is currently included. On Jetson, install PyTorch, OpenCV, and the ZED Python API using versions compatible with the installed JetPack/ZED SDK rather than replacing them with generic desktop builds.

For the remaining Python packages, a typical installation command is:

```bash
python3 -m pip install numpy ultralytics requests flask pyserial pynmea2 PyQt5 pyautogui TTS simpleaudio
```

## Installation

### 1. Clone the repository

```bash
git clone https://github.com/sail2021tau-a11y/Autonomous-Boat-Project.git
cd Autonomous-Boat-Project
```

### 2. Prepare and source the ROS 2 workspace

Place the custom `cv_from_zed_ros2` package in the workspace source directory, then build and source it:

```bash
source /opt/ros/foxy/setup.bash
cd ~/foxy_ws
colcon build --symlink-install
source install/setup.bash
```

### 3. Add the YOLOv8 weights

The perception node expects the trained model file:

```text
FinalVersionWeights.pt
```

The weights file is not included in this repository export. Copy it from the project Drive/media archive and update the model path in `publisher_cv_2025_ros2.py`.

### 4. Upload the Arduino firmware

Upload the following sketch to the Arduino Nano:

```text
arduino_controllers/_3_engine_lrf_work_VER2/_3_engine_lrf_work_VER2.ino
```

The Arduino communicates at **9600 baud** and controls the thrusters through PWM:

- Left thruster: pin D9
- Right thruster: pin D10
- Front thruster: pin D8

> A separate `2026_AB_hardware_validation` branch contains an earlier, standalone Arduino sketch (`boat_firmware.ino`) with a steering servo on pin D3 and an RC receiver override on pin D2. That branch is an independent hardware bring-up harness with its own serial protocol (single-character commands, 115200 baud, `/dev/ttyACM1`) — it is not part of the ROS 2 pipeline this README documents, and its own soft-start function (`updateMotorsSmoothly`) is an empty stub with no ramping logic. The firmware in `arduino_controllers/` on this branch only implements differential thrust on D8/D9/D10 above; it has no servo output and no RC input pin.
>
> Motor soft-start ramping was separately validated during hardware stability/current-draw testing, but that ramping logic was not carried into this branch's shipped firmware and there are no current plans to add it here.

### 5. Update machine-specific configuration

Before the first run, update the following settings:

1. **Absolute project paths**  
   The launch scripts currently contain the original development path:

   ```text
   /home/sail/Desktop/danel_gadya_sail_2025_git
   ```

   Replace it in `bashes4/*.sh` and `bashes4/start.py` with the actual repository location.

2. **ROS 2 workspace path**  
   The scripts currently expect:

   ```text
   /home/sail/foxy_ws
   ```

3. **YOLO model path**  
   Update the path to `FinalVersionWeights.pt` in `publisher_cv_2025_ros2.py`.

4. **Serial ports**  
   Confirm the Arduino and GPS ports:

   ```bash
   ls /dev/ttyUSB* /dev/ttyACM*
   ```

5. **GPS target**  
   Set `TARGET_LAT` and `TARGET_LON` in `task4_navigation_2025_ros2_eff.py`.

6. **TTS environment**  
   `bashes4/run_environment_ros2.sh` references a dedicated TTS virtual environment. Update this path or run `server_ros2.py` from an environment containing the TTS dependencies.

### 6. Make the scripts executable

```bash
chmod +x bashes4/*.sh
```

## Basic Run Commands

### Recommended interactive launcher

Start the management interface:

```bash
bash bashes4/start_run_ros2.sh
```

The launcher uses a state machine. Enter commands in this order:

| Command | Action |
|---|---|
| `S` | Start the Flask server, perception node, and engine controller |
| `1` | Start Task 1 – gate navigation |
| `2` | Start Task 2 – obstacle avoidance |
| `3` | Start Task 3 – precision docking |
| `4` | Start Task 4 – GPS waypoint navigation |
| `1K` / `2K` / `3K` / `4K` | Stop the currently running task and send an engine-stop command |
| `SK` | Stop the full environment and close the engine controller safely |
| `exit` | Exit the launcher when no task is running |

Example session:

```text
S
2
2K
SK
```

Do not start another task before stopping the active task with its matching `K` command.

### Flask dashboard

After command `S` reports that the server is ready, open:

```text
http://<JETSON-IP>:5000
```

### Task 4 heading input

The current Task 4 implementation reads GPS coordinates but receives heading through the `/mock_heading` ROS 2 topic. Publish a test heading with:

```bash
bash bashes4/set_heading.sh 90
```

Replace `90` with the current heading in degrees. A real closed-loop GPS implementation requires a validated IMU or another live heading source.

To read the current GPS coordinates for use as a waypoint:

```bash
python3 get_target_gps.py
```

### Serial communication test

With the thrusters disconnected or the boat physically secured, test communication with the Arduino using:

```bash
python3 serialtest.py
```

## ROS 2 Topics

| Topic | Message type | Purpose |
|---|---|---|
| `object_distance_info` | `cv_from_zed_ros2/msg/ObjectDistanceInfo` | Detected object label and 3D coordinates |
| `steering_directions` | `std_msgs/msg/String` | Steering angle or `stop` command |
| `/mock_heading` | `std_msgs/msg/String` | Manual heading input for Task 4 |

## Safety Notes

- Test navigation logic with the thrusters disconnected before performing powered tests.
- Confirm the serial ports before every run.
- Always stop the active task with its matching kill command before using `SK`.
- The engine controller sends zero-power commands during startup and shutdown.
- **Verify the boat's physical kill switch / battery disconnect before every powered test or launch.** This is independent of the software below and is the primary, reliable way to cut motor power in an emergency.
- **This branch's Arduino sketch has no RC override and no soft-start ramping**, and does not stop the thrusters on its own if the Jetson stops sending commands. The sketch (`arduino_controllers/_3_engine_lrf_work_VER2.ino`) applies each commanded power value directly with no ramping and does not read an RC receiver — for example, if the ROS 2 process crashes or the USB cable is unplugged while motors are running, the last commanded power is held until a new command arrives or power is cut physically. Do not rely on a remote control or an automatic communication-loss stop unless you implement and test that logic yourself.
- **The `stop` command is sent on task completion, on operator kill commands, and — on this branch only — on a navigation task's own detection-loss timeout.** Tasks 3 and 4 publish `stop` automatically on reaching their target, and every task's `*_kill_ros2.sh` script publishes `stop` when the operator issues the matching `K` command. Each navigation node's `TimerManager` (Tasks 1–3), which ends the task if the camera stops detecting valid targets for an extended period, now also calls `self.node.publish_angle("stop")` before shutting its ROS 2 node down (Task 1's timer was previously disabled in code and has been re-enabled to match Tasks 2 and 3). **This specific change has not yet been tested on the physical boat** — see the branch-status note at the top of this file before treating it as validated.

## Current Limitations and Important Notes

- Several launch files contain absolute paths from the original Jetson installation and must be updated on a new system.
- The trained YOLOv8 weights and the custom ROS 2 message package are not included in this repository export.
- Task 4 currently uses a manually published heading because a validated IMU heading source is not integrated. The GPS module also has several meters of static positioning error, which is why the arrival radius in the code is set to 8.5 m rather than a tighter value.
- The Arduino sketch has no RC override and no soft-start ramping (soft-start was validated separately during hardware stability testing but was not carried into this branch's firmware).
- This branch fixes the navigation-timeout stop gap described in the branch-status note at the top of this file, but that fix is unmerged and untested on the physical boat — see [Safety Notes](#safety-notes).
- The `2B` option appears in both `bashes4/start.py` and `bashes/start.py`, but the corresponding `task2_run_ros2B.sh` / `task2_kill_ros2B.sh` (and `2BK` entry) are not present in this repository. Do not use `2B`/`2BK` unless those scripts are restored.
- `bashes/` is an older, Task-4-less duplicate of `bashes4/` with its own hardcoded paths; it is kept for reference but is not the launcher described in this README.
- The `2026_AB_hardware_validation` branch is a separate, non-integrated hardware bring-up harness (single-character serial protocol, different pinout, RC override, steering servo on D3) predating this branch's ROS 2 integration work. It uses its own README/OPERATIONS.md and is not compatible with the pipeline described here.
- Changes made in another copy of the repository are not synchronized automatically.
