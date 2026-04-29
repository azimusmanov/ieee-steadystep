import asyncio
import csv
import time
from bleak import BleakScanner, BleakClient

CHARACTERISTIC_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
OUTPUT_FILE = "bno055_data.csv"
HEADER = ["mac_timestamp", "esp32_ms",
          "heading", "roll", "pitch",
          "lin_ax", "lin_ay", "lin_az",
          "gx", "gy", "gz"]

def handle_data(sender, data):
    row = data.decode("utf-8").strip().split(",")
    if len(row) != 10:
        print(f"Malformed packet, skipping: {row}")
        return
    mac_ts = time.time()
    print(f"t={mac_ts:.3f}  esp32_ms={row[0]}  "
          f"euler=({row[1]},{row[2]},{row[3]})  "
          f"linAccel=({row[4]},{row[5]},{row[6]})  "
          f"gyro=({row[7]},{row[8]},{row[9]})")
    with open(OUTPUT_FILE, "a", newline="") as f:
        csv.writer(f).writerow([mac_ts] + row)

async def main():
    print("Scanning for SteadyStep-IMU...")
    device = await BleakScanner.find_device_by_name("SteadyStep-IMU", timeout=10)
    if not device:
        print("Device not found. Is the ESP32 powered on?")
        return

    print(f"Found: {device.address} — connecting...")
    async with BleakClient(device) as client:
        print("Connected. Receiving data (Ctrl+C to stop)...\n")

        with open(OUTPUT_FILE, "a", newline="") as f:
            if f.tell() == 0:
                csv.writer(f).writerow(HEADER)

        await client.start_notify(CHARACTERISTIC_UUID, handle_data)
        await asyncio.sleep(3600)

asyncio.run(main())
