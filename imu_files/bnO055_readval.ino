#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Libraries needed (Arduino Library Manager):
//   Adafruit BNO055
//   Adafruit Unified Sensor

#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// Try 0x29 if 0x28 fails (depends on ADO pin)
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29);

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer)    { deviceConnected = true; }
  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    pServer->getAdvertising()->start();
  }
};

void setup() {
  Serial.begin(115200);
  delay(500);  // let serial settle before printing anything
  Wire.begin(21, 22);

  if (!bno.begin()) {
    Serial.println("BNO055 not found. Check wiring or try I2C addr 0x29.");
    // feed watchdog while halted so ESP32 doesn't crash-loop
    while (1) { 
      Serial.println("BNO055 NOT OK");
      delay(1000); }
  }
  bno.setExtCrystalUse(true);
  Serial.println("BNO055 OK");

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
  Serial.println("BLE ready — SteadyStep-IMU");
}

void loop() {
  imu::Vector<3> euler    = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  imu::Vector<3> linAccel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  imu::Vector<3> gyro     = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);

  // CSV: esp32_ms, heading, roll, pitch, lin_ax, lin_ay, lin_az, gx, gy, gz
  char buf[128];
  snprintf(buf, sizeof(buf),
    "%lu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
    millis(),
    euler.x(),    euler.y(),    euler.z(),
    linAccel.x(), linAccel.y(), linAccel.z(),
    gyro.x(),     gyro.y(),     gyro.z()
  );

  Serial.println(buf);

  if (deviceConnected) {
    pCharacteristic->setValue(buf);
    pCharacteristic->notify();
  }

  delay(100);  // 10 Hz — stable for BLE, sufficient for gait
}
