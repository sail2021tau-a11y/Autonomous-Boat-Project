#include <Arduino.h>

// Define PWM pins for the three engines
const int LEFT_ENGINE_PIN = 9;   
const int RIGHT_ENGINE_PIN = 10; 
const int CENTER_ENGINE_PIN = 11;

void setup() {
  // Initialize serial communication to match the Jetson's BAUD_RATE
  Serial.begin(9600); 
  
  // Set engine pins as outputs
  pinMode(LEFT_ENGINE_PIN, OUTPUT);
  pinMode(RIGHT_ENGINE_PIN, OUTPUT);
  pinMode(CENTER_ENGINE_PIN, OUTPUT);
  
  // Start with engines powered off (PWM = 0)
  analogWrite(LEFT_ENGINE_PIN, 0);
  analogWrite(RIGHT_ENGINE_PIN, 0);
  analogWrite(CENTER_ENGINE_PIN, 0);
  
  // Print ready message to the Serial Monitor
  Serial.println("Arduino ready - Send command in format l<val>,r<val>,f<val>");
}

void loop() {
  // Check if data is available to read from the Jetson
  if (Serial.available() > 0) {
    // Read the incoming string until a newline character is received
    String command = Serial.readStringUntil('\n');
    command.trim(); // Remove any leading/trailing whitespace
    
    int left_power = 0, right_power = 0, forward_power = 0;
    
    // Parse the string formatted as "l50,r30,f10"
    if (sscanf(command.c_str(), "l%d,r%d,f%d", &left_power, &right_power, &forward_power) == 3) {
      
      // Map the percentage power (-100 to 100) to Arduino PWM output (0 to 255)
      // Constrain is used to prevent values exceeding limits
      int left_pwm = map(constrain(left_power, -100, 100), 0, 100, 0, 255);
      int right_pwm = map(constrain(right_power, -100, 100), 0, 100, 0, 255);
      int forward_pwm = map(constrain(forward_power, -100, 100), 0, 100, 0, 255);
      
      // Apply the calculated PWM signals to the corresponding engine pins
      // The abs() function ensures the PWM value is positive
      analogWrite(LEFT_ENGINE_PIN, abs(left_pwm));
      analogWrite(RIGHT_ENGINE_PIN, abs(right_pwm));
      analogWrite(CENTER_ENGINE_PIN, abs(forward_pwm));
      
      // Send a confirmation back to the Jetson
      Serial.println("Received: " + command + " -> Motors updated.");
    } else {
      // Handle incorrectly formatted commands
      Serial.println("Error: Invalid command format.");
    }
  }
}
