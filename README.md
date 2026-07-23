# Autonomous Boat – System Engineering and Integrations

Final project in Electrical Engineering, Tel Aviv University  
**Project No. 25-1-1-3272**

This repository contains the software integration and control system developed for the **Sail-IL Autonomous Surface Vehicle (ASV)**. The platform is designed to perform autonomous marine navigation tasks inspired by the international RoboBoat competition.

The system runs on an **NVIDIA Jetson Orin** and uses **ROS 2 Foxy** to connect perception, navigation, monitoring, and motor-control nodes. A **ZED 2i stereo camera** and a **YOLOv8** model detect and localize objects in 3D, navigation nodes calculate steering commands, and an **Arduino Nano** converts the commands into PWM signals for three Blue Robotics T200 thrusters.

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
├── bashes4/                           # Main launch and shutdown scripts for Tasks 1–4
├── templates/index.html               # Web dashboard
└── frequencies/                       # Performance and frequency test results
```

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
- Servo output prepared for future use: pin D3

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
- Verify the physical kill switch and manual override before operating in water.
- Confirm the serial ports before every run.
- Always stop the active task with its matching kill command before using `SK`.
- The engine controller sends zero-power commands during startup and shutdown, but hardware safety mechanisms must remain available.

## Current Limitations and Important Notes

- Several launch files contain absolute paths from the original Jetson installation and must be updated on a new system.
- The trained YOLOv8 weights and the custom ROS 2 message package are not included in this repository export.
- Task 4 currently uses a manually published heading because a validated IMU heading source is not integrated.
- The `2B` option appears in `bashes4/start.py`, but the corresponding `task2_run_ros2B.sh` and `task2_kill_ros2B.sh` files are not present in this repository. Do not use `2B` unless those scripts are restored.
- Changes made in another copy of the repository are not synchronized automatically.

## Project Team

**Students**

- Tal Gilboa
- Tal Franco

**Supervisors**

- Simcha Leibovitz
- Roi Reich

Electrical Engineering Project Laboratory, Tel Aviv University.
