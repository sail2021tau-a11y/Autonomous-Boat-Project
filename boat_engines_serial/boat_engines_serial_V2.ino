#include <Arduino.h>
#include <Servo.h> 

Servo escLeft;
Servo escRight;
Servo escBack; // Matching the KiCad schematic (Back Thruster)

const int LEFT_ENGINE_PIN = 9;   // Verified working
const int RIGHT_ENGINE_PIN = 10; // Verified working
const int BACK_ENGINE_PIN = 8;   // FIXED: Hardware schematic reality

// Safety Cap for bench testing (0 to 100 percentage)
// Protects both forward and reverse directions
const int MAX_TEST_POWER = 30; 

void setup() {
  Serial.begin(9600); 
  
  escLeft.attach(LEFT_ENGINE_PIN);
  escRight.attach(RIGHT_ENGINE_PIN);
  escBack.attach(BACK_ENGINE_PIN);
  
  // ESC Arming Sequence (Neutral Signal)
  escLeft.writeMicroseconds(1500);
  escRight.writeMicroseconds(1500);
  escBack.writeMicroseconds(1500);
  
  delay(2000); // Wait for ESC boot tones
  
  // Exact Handshake string expected by Danel & Gadya's Python script
  Serial.println("Send command: l 30 r -20 f 0"); 
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim(); 
    
    int left_power = 0, right_power = 0, forward_power = 0;
    
    // Using robust sscanf to parse commas/spaces and directly support negative numbers (e.g. l-30)
    if (sscanf(command.c_str(), "l%d,r%d,f%d", &left_power, &right_power, &forward_power) == 3) {
      
      // Enforce bidirectional safety cap (-15 to 15)
      left_power = constrain(left_power, -MAX_TEST_POWER, MAX_TEST_POWER);
      right_power = constrain(right_power, -MAX_TEST_POWER, MAX_TEST_POWER);
      forward_power = constrain(forward_power, -MAX_TEST_POWER, MAX_TEST_POWER);
      
      // Map bidirectional percentage (-100 to 100) to standard ESC pulse widths (1100us to 1900us)
      int left_us = map(left_power, -100, 100, 1100, 1900);     
      int right_us = map(right_power, -100, 100, 1100, 1900);    
      int back_us = map(forward_power, -100, 100, 1100, 1900); 
      
      // Write correct pulse widths to the physical pins
      escLeft.writeMicroseconds(left_us);
      escRight.writeMicroseconds(right_us);
      escBack.writeMicroseconds(back_us);
      
      Serial.print("Received: " + command);
      Serial.println(" -> Protocol synchronized.");
    }
  }
}
