#!/usr/bin/env python3

import serial
import pynmea2

# Adjust this to match your actual GPS port
SERIAL_PORT = '/dev/ttyACM0'
BAUD_RATE = 9600

def get_current_coordinates():
    print("📡 Connecting to GPS antenna... Waiting for a valid fix...")
    try:
        with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1) as ser:
            while True:
                line = ser.readline().decode('ascii', errors='replace').strip()
                
                # Check for location sentences
                if line.startswith('$GNRMC') or line.startswith('$GNGGA'):
                    try:
                        msg = pynmea2.parse(line)
                        if msg.latitude != 0.0 and msg.longitude != 0.0:
                            print("\n✅ Valid GPS Fix Found!")
                            print("=" * 40)
                            print(f"TARGET_LAT = {msg.latitude:.6f}")
                            print(f"TARGET_LON = {msg.longitude:.6f}")
                            print("=" * 40)
                            print("Copy these values into your task 4 code.")
                            break
                    except pynmea2.ParseError:
                        pass # Ignore malformed sentences
    except Exception as e:
        print(f"❌ Error reading GPS: {e}")

if __name__ == '__main__':
    get_current_coordinates()