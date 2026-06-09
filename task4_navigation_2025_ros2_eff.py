#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from std_msgs.msg import String
import serial
import pynmea2
import math
import os
import subprocess
import requests
import threading
import time

# ===========================================
# GPS Navigation Configuration
# ===========================================
ARRIVED_THRESHOLD_METERS = 8.5  # Arrival radius in meters
SERIAL_PORT = '/dev/ttyACM0'    # GPS serial port as identified in tests
BAUD_RATE = 9600                # Default baud rate for u-blox

# Target coordinates (Waypoint) - currently set near your tested location
TARGET_LAT = 32.108509
TARGET_LON = 34.805753

class Task4Navigator(Node):
    def __init__(self):
        super().__init__('navigation_task4_node')
        
        # ROS2 Publishers & Subscribers
        self.publisher = self.create_publisher(String, 'steering_directions', 10)
        self.create_subscription(String, '/mock_heading', self.mock_heading_callback, 10)
        
        # System variables
        self.current_lat = None
        self.current_lon = None
        self.current_heading = 0.0  # Simulated heading (Mock Heading)
        self.conf_set = False
        self.is_active = True
        
        # Start a separate thread for reading GPS to prevent blocking ROS2 execution
        threading.Thread(target=self.read_gps, daemon=True).start()

        # Navigation loop (1Hz)
        self.create_timer(1.0, self.navigate)

        self.launch_gui()
        self.say(". Starting Task Four. Navigation to G P S waypoint.")
        self.get_logger().info("Task 4 started. Waiting for GPS fix...")

    def launch_gui(self):
        """Launches the PyQt-based GUI for real-time visualization."""
        gui_path = os.path.join(os.path.dirname(__file__), 'map_gui_viewer.py')
        subprocess.Popen(['python3', gui_path])

    def say(self, text):
        """Sends a text message to the local TTS server for audio feedback."""
        try:
            requests.post("http://localhost:5000/say", json={"text": text})
        except Exception as e:
            self.get_logger().warn(f"Cannot connect to TTS server: {e}")

    def mock_heading_callback(self, msg):
        """Callback to receive and update the simulated heading from the ROS2 topic."""
        try:
            self.current_heading = float(msg.data)
            self.get_logger().info(f"🔄 Mock Heading manually updated to: {self.current_heading}°")
        except ValueError:
            self.get_logger().error(f"Invalid mock heading received: {msg.data}")

    def read_gps(self):
        """Continuously reads GPS data with retry logic for busy ports."""
        while self.is_active:
            try:
                with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
                    self.get_logger().info("✅ GPS Port opened successfully.")
                    while self.is_active:
                        line = ser.readline().decode('ascii', errors='replace').strip()
                        if line.startswith('$GNRMC') or line.startswith('$GNGGA'):
                            try:
                                msg = pynmea2.parse(line)
                                if msg.latitude != 0.0 and msg.longitude != 0.0:
                                    self.current_lat = msg.latitude
                                    self.current_lon = msg.longitude
                            except pynmea2.ParseError:
                                pass
            except Exception as e:
                self.get_logger().warn(f"GPS Port busy or error: {e}. Retrying in 2 seconds...")
                time.sleep(2) 

    def haversine_distance(self, lat1, lon1, lat2, lon2):
        """Calculates the Euclidean distance in meters between two coordinates."""
        R = 6371000.0  # Earth radius in meters
        phi1, phi2 = math.radians(lat1), math.radians(lat2)
        dphi = math.radians(lat2 - lat1)
        dlambda = math.radians(lon2 - lon1)
        a = math.sin(dphi/2)**2 + math.cos(phi1) * math.cos(phi2) * math.sin(dlambda/2)**2
        return R * 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))

    def calculate_bearing(self, lat1, lon1, lat2, lon2):
        """Calculates the target bearing (azimuth) relative to true north."""
        phi1, phi2 = math.radians(lat1), math.radians(lat2)
        dlambda = math.radians(lon2 - lon1)
        y = math.sin(dlambda) * math.cos(phi2)
        x = math.cos(phi1) * math.sin(phi2) - math.sin(phi1) * math.cos(phi2) * math.cos(dlambda)
        return (math.degrees(math.atan2(y, x)) + 360) % 360

    def navigate(self):
        """Main navigation loop executed every second."""
        if not self.conf_set:
            with open("/tmp/current_task.txt", "w") as f:
                f.write("task4")
            self.conf_set = True

        # Wait until a valid GPS fix is acquired
        if self.current_lat is None or self.current_lon is None:
            return 

        # 1. Calculate distance and bearing to target
        distance = self.haversine_distance(self.current_lat, self.current_lon, TARGET_LAT, TARGET_LON)
        target_bearing = self.calculate_bearing(self.current_lat, self.current_lon, TARGET_LAT, TARGET_LON)

        # 2. Check if the boat has reached the destination
        if distance <= ARRIVED_THRESHOLD_METERS:
            self.get_logger().info("🎯 Waypoint Reached! Stopping engines.")
            self.say(". Destination reached. Stopping engines.")
            msg = String()
            msg.data = "stop"
            self.publisher.publish(msg)
            
            # Gracefully shut down the node
            self.is_active = False
            
            # Report successful stop to dashboard
            try:
                requests.post("http://localhost:5000/api/telemetry", json={"task_status": "Task 4 Stopped (Arrived)"}, timeout=0.1)
            except Exception:
                pass
            
            rclpy.shutdown()
            return

        # 3. Calculate steering error relative to our current heading
        error_angle = target_bearing - self.current_heading
        
        # Normalize the angle to be within [-180, 180] range
        if error_angle > 180:
            error_angle -= 360
        elif error_angle < -180:
            error_angle += 360

        # 4. Publish steering command for engine controller
        msg = String()
        msg.data = f"{round(error_angle)}"
        self.publisher.publish(msg)
        
        # Log telemetry data including real-time raw GPS coordinates
        self.get_logger().info(
            f"🌐 GPS Pos: ({self.current_lat:.6f}, {self.current_lon:.6f}) | "
            f"Dist: {distance:.1f}m | Tgt Bearing: {target_bearing:.1f}° | "
            f"Mock Hdg: {self.current_heading}° | Error: {msg.data}°"
        )
        
        # ==========================================
        # Telemetry Update to Flask Dashboard
        # ==========================================
        def send_task_telemetry():
            try:
                requests.post("http://localhost:5000/api/telemetry", json={
                    "task_status": "Task 4 (GPS) Active",
                    "distance": f"{distance:.1f}",
                    "gps": {"lat": round(self.current_lat, 6), "lon": round(self.current_lon, 6)}
                }, timeout=0.1)
            except Exception:
                pass
                
        threading.Thread(target=send_task_telemetry, daemon=True).start()
        # ==========================================

def main(args=None):
    rclpy.init(args=args)
    node = Task4Navigator()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.is_active = False
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()