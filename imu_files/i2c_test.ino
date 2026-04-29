// Script to test ESP32 + IMU Wiring
#include <Wire.h>


// Expected output:

// 0x68 → normal
// 0x69 → if ADO tied to VCC

// If nothing shows up → wiring issue.

void setup() {
Serial.begin(115200);
delay(1000);
Serial.println("\nI2C Scanner");

Wire.begin(21, 22); // SDA, SCL
}

void loop() {
byte error, address;
int count = 0;

Serial.println("Scanning...");

for (address = 1; address < 127; address++) {
Wire.beginTransmission(address);
error = Wire.endTransmission();

if (error == 0) {
  Serial.print("Device found at 0x");
  if (address < 16) Serial.print("0");
  Serial.println(address, HEX);
  count++;
}
}

if (count == 0) {
Serial.println("No I2C devices found\n");
} else {
Serial.println("Done\n");
}

delay(3000);
}
 