#include <Arduino.h>
#include <Servo.h> 

Servo leftEngine;
Servo rightEngine;
Servo backEngine; // Matching the KiCad schematic naming (BACK ESC)

const int LEFT_ENGINE_PIN = 9;   // Verified working (Left Thruster)
const int RIGHT_ENGINE_PIN = 10; // Verified working (Right Thruster)
const int BACK_ENGINE_PIN = 8;   // FIXED: Changed from 11 to 8 based on KiCad Schematic

// Safety Cap for bench testing (0 to 100 percentage)
const int MAX_TEST_POWER = 15; 

void setup() {
  Serial.begin(9600); 
  
  leftEngine.attach(LEFT_ENGINE_PIN);
  rightEngine.attach(RIGHT_ENGINE_PIN);
  backEngine.attach(BACK_ENGINE_PIN);
  
  // ESC Arming Sequence
  // Sending neutral signal (1500us) to all ESCs to unlock them safely
  leftEngine.writeMicroseconds(1500);
  rightEngine.writeMicroseconds(1500);
  backEngine.writeMicroseconds(1500);
  
  Serial.println("ESCs Arming Sequence started... Please wait 3 seconds.");
  delay(3000); 
  
  Serial.println("Arduino ready - SCHEMATIC PINS ACTIVE (9, 10, 8) - Safe Cap 15%");
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim(); 
    
    int left_power = 0, right_power = 0, forward_power = 0;
    
    // Parsing command format: l<val>,r<val>,f<val>
    if (sscanf(command.c_str(), "l%d,r%d,f%d", &left_power, &right_power, &forward_power) == 3) {
      
      // Enforce safety cap
      left_power = constrain(left_power, 0, MAX_TEST_POWER);
      right_power = constrain(right_power, 0, MAX_TEST_POWER);
      forward_power = constrain(forward_power, 0, MAX_TEST_POWER);
      
      // Map percentage (0 to 100) to standard ESC pulse widths (1500us to 2000us)
      int left_us = map(left_power, 0, 100, 1500, 2000);      
      int right_us = map(right_power, 0, 100, 1500, 2000);    
      int back_us = map(forward_power, 0, 100, 1500, 2000); 
      
      // Write signals to the actual pins
      leftEngine.writeMicroseconds(left_us);
      rightEngine.writeMicroseconds(right_us);
      backEngine.writeMicroseconds(back_us);
      
      Serial.print("Received: " + command);
      Serial.println(" -> Signals sent to Pins 9, 10 and 8.");
    } else {
      Serial.println("Error: Invalid command format.");
    }
  }
}
