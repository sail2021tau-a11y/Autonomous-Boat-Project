source /opt/ros/foxy/setup.bash && ros2 topic pub --once /mock_heading std_msgs/msg/String "{data: '"$1"'}"
