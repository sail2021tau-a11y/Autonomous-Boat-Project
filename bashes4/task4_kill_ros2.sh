#!/bin/bash
echo "Stopping Task 4..."

# Closing task 4 processes
pkill -f task4_navigation_2025_ros2_eff.py

# Sending POST request to the server to read the wanted message
curl -s -o /dev/null -X POST http://localhost:5000/say \
     -H "Content-Type: application/json" \
     -d '{"text":". Task 4 Finished."}' \
     --max-time 1 &

echo "Task 4 stopped."
