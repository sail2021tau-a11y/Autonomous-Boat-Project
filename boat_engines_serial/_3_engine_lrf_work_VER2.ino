
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

  // Move past the label (e.g., 'l') and skip any spaces
  int startIndex = index + 1;
  while (startIndex < data.length() && data.charAt(startIndex) == ' ') {
    startIndex++;
  }

  // Find next space or end
  int spaceIndex = data.indexOf(' ', startIndex);
  if (spaceIndex == -1) spaceIndex = data.length();

  String val = data.substring(startIndex, spaceIndex);
  val.trim();
  return constrain(val.toInt(), -100, 100);
}

void setThrusters(int valL, int valR, int valF) {
  int pwmL = map(valL, -100, 100, 1430, 1570);
  int pwmR = map(valR, -100, 100, 1430, 1570);
  int pwmF = map(valF, -100, 100, 1430, 1570);

  escLeft.writeMicroseconds(pwmL);
  escRight.writeMicroseconds(pwmR);
  escForward.writeMicroseconds(pwmF);
}
