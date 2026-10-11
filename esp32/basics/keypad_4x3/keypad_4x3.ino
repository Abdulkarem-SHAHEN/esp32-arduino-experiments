/*
 * 4x3 matrix keypad (ESP32)
 * -------------------------
 * Prints every key pressed on a 12-key keypad.
 *
 * The 12 keys need only 7 pins: the library drives one row at a time and reads the
 * columns to find which key connects them.
 *
 * Wiring:  rows    R1..R4 -> GPIO 32, 33, 25, 26
 *          columns C1..C3 -> GPIO 27, 14, 12
 * Library: Keypad (Mark Stanley, Alexander Brevig)
 */

#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {32, 33, 25, 26};
byte colPins[COLS] = {27, 14, 12};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(115200);
}

void loop() {
  char key = keypad.getKey();   // 0 when no key is pressed
  if (key) {
    Serial.println(key);
  }
}
