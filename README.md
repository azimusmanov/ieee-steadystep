# SteadyStep — IEEE Project

IMU data collection system using an ESP32 + MPU6050, transmitting sensor data over BLE to a MacBook for local CSV logging. Intended to eventually feed into an AWS ML pipeline for gait/motion classification.

## Hardware

- **ESP32** (Dev Module) — microcontroller with built-in BLE
- **MPU6050** — 6-axis IMU (accelerometer + gyroscope) connected via I2C on pins SDA=21, SCL=22

## How it works

```
ESP32 + MPU6050
  └─ reads accel (m/s²) + gyro (deg/s) at 5 Hz
  └─ sends CSV over BLE UART
       └─ receive_imu.py (MacBook)
            └─ imu_data.csv
```

Each row logged:
```
mac_timestamp, esp32_ms, ax, ay, az, gx, gy, gz
```
- `mac_timestamp` — Unix time on the Mac when the packet arrived
- `esp32_ms` — milliseconds since ESP32 boot (`millis()`)
- `ax/ay/az` — linear acceleration in m/s²
- `gx/gy/gz` — angular velocity in deg/s

## Files

| File | Description |
|------|-------------|
| `imu_files/basic_test.ino` | Sanity check — verifies ESP32 is alive and blinks LED |
| `imu_files/i2c_test.ino` | I2C scanner — confirms MPU6050 is wired and detected at 0x68 |
| `imu_files/mpu6050_readval.ino` | Main sketch — reads IMU, transmits over BLE UART |
| `receive_imu.py` | Python BLE receiver — connects to ESP32 and saves data to CSV |

## Setup

### ESP32 (Arduino IDE)

1. Add board URL in Preferences:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
2. Install **esp32 by Espressif Systems** via Boards Manager
3. Select your board: Tools → Board → ESP32 Arduino → ESP32 Dev Module
4. Flash `imu_files/mpu6050_readval.ino`

### MacBook (Python receiver)

```bash
pip install bleak
python3 receive_imu.py
```

Make sure the ESP32 is powered on first. The script scans for a BLE device named `SteadyStep-IMU`, connects, and appends rows to `imu_data.csv`.

## Roadmap

- [ ] Add activity labels to CSV for supervised training
- [ ] Push CSV to AWS S3
- [ ] Train classifier in SageMaker
