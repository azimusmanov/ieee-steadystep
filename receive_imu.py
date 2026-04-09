import asyncio
import csv
import time
from bleak import BleakScanner, BleakClient

# Same UUID as on the ESP32
CHARACTERISTIC_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
OUTPUT_FILE = "imu_data.csv"

def handle_data(sender, data):
    row = data.decode("utf-8").strip().split(",")
    timestamp = time.time()
    print(f"{timestamp}  {row}")
    with open(OUTPUT_FILE, "a", newline="") as f:
        writer = csv.writer(f)
        writer.writerow([timestamp] + row)

async def main():
    print("Scanning for SteadyStep-IMU...")
    device = await BleakScanner.find_device_by_name("SteadyStep-IMU", timeout=10)
    if not device:
        print("Device not found. Is the ESP32 powered on?")
        return

    print(f"Found: {device.address} — connecting...")
    async with BleakClient(device) as client:
        print("Connected. Receiving data (Ctrl+C to stop)...\n")

        # Write CSV header if file is new
        with open(OUTPUT_FILE, "a", newline="") as f:
            if f.tell() == 0:
                csv.writer(f).writerow(["mac_timestamp","esp32_ms","ax","ay","az","gx","gy","gz"])

        await client.start_notify(CHARACTERISTIC_UUID, handle_data)
        await asyncio.sleep(3600)  # run for 1 hour max

asyncio.run(main())