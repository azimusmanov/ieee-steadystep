// Base test to test wired conneciton to esp 32 (without IMU)

void setup() {
Serial.begin(115200);
delay(1000);

Serial.println("ESP32 is alive");
pinMode(2, OUTPUT); // built-in LED on most ESP32 boards
}

void loop() {
Serial.println("Running...");

digitalWrite(2, HIGH);
delay(500);

digitalWrite(2, LOW);
delay(500);
}


// What you should see:
// ESP32 is alive
// Running...
// Running...
// ...