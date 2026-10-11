/*
 * PIR motion sensor (ESP32)
 * -------------------------
 * Lights an LED while the sensor sees movement and reports when motion starts and stops.
 *
 * Wiring: HC-SR501 OUT -> GPIO 2, VCC -> 5 V, GND -> GND
 *         LED on GPIO 4 through a 220 Ohm resistor
 * Note:   the sensor needs up to a minute after power-up before its readings are reliable.
 */

const int PIR_PIN = 2;
const int LED_PIN = 4;

bool motion = false;

void setup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  bool detected = digitalRead(PIR_PIN) == HIGH;
  digitalWrite(LED_PIN, detected ? HIGH : LOW);

  // print only when the state changes, instead of on every reading
  if (detected != motion) {
    motion = detected;
    Serial.println(motion ? "Motion detected!" : "No more motion");
  }

  delay(50);
}
