/*
 * Push button (ESP32)
 * -------------------
 * Reports each press of a button once, however long it is held.
 *
 * Wiring: button between GPIO 13 and GND. No external resistor: the internal pull-up
 *         keeps the pin HIGH, and pressing the button pulls it LOW.
 */

#define BUTTON_PIN 13

const unsigned long DEBOUNCE_MS = 30;   // contacts bounce for a few milliseconds

bool wasPressed = false;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  bool pressed = digitalRead(BUTTON_PIN) == LOW;

  // act on the change from released to pressed, not on the level
  if (pressed && !wasPressed) {
    Serial.println("Button pressed!");
  }
  wasPressed = pressed;

  delay(DEBOUNCE_MS);
}
