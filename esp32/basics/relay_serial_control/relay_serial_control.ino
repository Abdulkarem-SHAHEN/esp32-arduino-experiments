/*
 * Relay controlled from the Serial Monitor (ESP32)
 * ------------------------------------------------
 * Type "on" or "off" in the Serial Monitor to switch a relay.
 *
 * Most relay modules are active-low: the relay closes when its input is LOW. The pin is
 * therefore set HIGH at start-up so that the load stays off when the board boots.
 *
 * Wiring:  relay module IN -> GPIO 26, VCC -> 5 V, GND -> GND
 * Monitor: 115200 baud, line ending "Newline"
 */

const int RELAY_PIN = 26;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);   // off
  Serial.begin(115200);
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command == "on") {
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("Relay ON");
    } else if (command == "off") {
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Relay OFF");
    } else {
      Serial.println("Unknown command. Type 'on' or 'off'.");
    }
  }
}
