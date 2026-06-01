
#include <Servo.h>

Servo escLeft;
Servo escRight;
Servo escForward;

String input = "";

void setup() {
  Serial.begin(9600);
  escLeft.attach(9);    // Left thruster
  escRight.attach(10);  // Right thruster
  escForward.attach(8);// Forward thruster

  // Arm ESCs (neutral signal)
  escLeft.writeMicroseconds(1500);
  escRight.writeMicroseconds(1500);
  escForward.writeMicroseconds(1500);

  delay(2000);
  Serial.println("Send command: l 30 r -20 f 0");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (input.length() > 0) {
        parseInput(input);


        input = "";  // reset after processing
      }
    } else {
      input += c;
    }
  }
}

void parseInput(String cmd) {
  cmd.trim();

  int valL = extractValue(cmd, "l");
  int valR = extractValue(cmd, "r");
  int valF = extractValue(cmd, "f");

  Serial.print("Parsed -> L: "); Serial.print(valL);
  Serial.print("  R: "); Serial.print(valR);
  Serial.print("  F: "); Serial.println(valF);

  setThrusters(valL, valR, valF);
}

int extractValue(String data, String label) {
  int index = data.indexOf(label);
  if (index == -1) return 0;

  // Move past the label character (e.g., 'l', 'r', or 'f')
  int startIndex = index + label.length();

  // Find the next comma to determine where the number ends
  int endIndex = data.indexOf(',', startIndex);
  
  // If no comma is found (like in the last value 'f'), look for a space or the end of the string
  if (endIndex == -1) {
    endIndex = data.indexOf(' ', startIndex);
    if (endIndex == -1) {
      endIndex = data.length();
    }
  }

  // Extract only the number and convert it to an integer
  String val = data.substring(startIndex, endIndex);
  return val.toInt();
}

void setThrusters(int valL, int valR, int valF) {
  int pwmL = map(valL, -100, 100, 1430, 1570);
  int pwmR = map(valR, -100, 100, 1430, 1570);
  int pwmF = map(valF, -100, 100, 1430, 1570);

  escLeft.writeMicroseconds(pwmL);
  escRight.writeMicroseconds(pwmR);
  escForward.writeMicroseconds(pwmF);
}
