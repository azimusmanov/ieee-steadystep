#include <Wire.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// How to verify it's "accurate enough"

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
#define ACCEL_SCALE 16384.0  // LSB/g for ±2g range
#define G_TO_MS2   9.81      // m/s² per g
#define GYRO_SCALE 131.0     // LSB/(°/s) for ±250°/s range

// Nordic UART Service UUIDs (matches the Python script)
#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// Detect connect/disconnect events
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("BLE client connected");
  }
  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("BLE client disconnected — restarting advertising");
    pServer->getAdvertising()->start();
  }
};

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

  // Init BLE
  BLEDevice::init("SteadyStep-IMU");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  pServer->getAdvertising()->start();
  Serial.println("BLE advertising as 'SteadyStep-IMU'");
}

void loop() {
  // Read IMU registers
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

  float ax = (accelX / ACCEL_SCALE) * G_TO_MS2;
  float ay = (accelY / ACCEL_SCALE) * G_TO_MS2;
  float az = (accelZ / ACCEL_SCALE) * G_TO_MS2;

  float gx = gyroX / GYRO_SCALE;
  float gy = gyroY / GYRO_SCALE;
  float gz = gyroZ / GYRO_SCALE;

  // millis() timestamp (ms since boot) — Python side adds wall-clock time
  unsigned long t = millis();

  // CSV format: timestamp_ms,ax,ay,az,gx,gy,gz
  char buf[80];
  snprintf(buf, sizeof(buf), "%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f", t, ax, ay, az, gx, gy, gz);

  // Always print to Serial for local debugging
  Serial.println(buf);

  // Send over BLE if a client is connected
  if (deviceConnected) {
    pCharacteristic->setValue(buf);
    pCharacteristic->notify();
  }

  delay(200);
}
