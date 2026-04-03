#include <Wire.h>

// How to verify it’s “accurate enough”

// Do these quick checks:

// 1. Flat on table
// Accel Z ≈ +16384 (gravity)
// X, Y ≈ 0

// 2. Rotate board
// Values should shift between axes smoothly

// 3. Gyro
// Still → near 0
// Move → spikes

#define MPU_ADDR 0x68

int16_t accelX, accelY, accelZ;
int16_t gyroX, gyroY, gyroZ;

void setup() {
Serial.begin(115200);
Wire.begin(21, 22);

// Wake up MPU6050
Wire.beginTransmission(MPU_ADDR);
Wire.write(0x6B); // PWR_MGMT_1
Wire.write(0);    // wake up
Wire.endTransmission(true);

Serial.println("MPU6050 Initialized");
}

void loop() {
// Start reading at accel register
Wire.beginTransmission(MPU_ADDR);
Wire.write(0x3B);
Wire.endTransmission(false);
Wire.requestFrom(MPU_ADDR, 14, true);

accelX = Wire.read() << 8 | Wire.read();
accelY = Wire.read() << 8 | Wire.read();
accelZ = Wire.read() << 8 | Wire.read();

Wire.read(); Wire.read(); // skip temp

gyroX = Wire.read() << 8 | Wire.read();
gyroY = Wire.read() << 8 | Wire.read();
gyroZ = Wire.read() << 8 | Wire.read();

Serial.print("Accel: ");
Serial.print(accelX); Serial.print(", ");
Serial.print(accelY); Serial.print(", ");
Serial.print(accelZ);

Serial.print(" | Gyro: ");
Serial.print(gyroX); Serial.print(", ");
Serial.print(gyroY); Serial.print(", ");
Serial.println(gyroZ);

delay(200);
}