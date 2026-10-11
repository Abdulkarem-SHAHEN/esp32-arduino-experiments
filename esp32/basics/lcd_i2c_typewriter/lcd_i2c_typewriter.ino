/*
 * 16x2 LCD over I2C: typewriter effect (ESP32)
 * --------------------------------------------
 * Writes text one character at a time, like a typewriter.
 *
 * Wiring:  SDA -> GPIO 21, SCL -> GPIO 22, VCC -> 5 V, GND -> GND
 * Library: LiquidCrystal I2C
 * Note:    0x27 is the usual address of the I2C backpack; some modules use 0x3F.
 */

#include <LiquidCrystal_I2C.h>
#include <Wire.h>

const int TYPING_DELAY_MS = 150;   // time between two characters: lower is faster

LiquidCrystal_I2C lcd(0x27, 16, 2);   // address, columns, rows

// Prints `text` character by character, starting at the given column and row.
void typeText(int col, int row, const String &text) {
  lcd.setCursor(col, row);
  for (unsigned int i = 0; i < text.length(); i++) {
    lcd.print(text[i]);
    delay(TYPING_DELAY_MS);
  }
}

void setup() {
  Wire.begin(21, 22);   // SDA, SCL
  lcd.init();
  lcd.backlight();

  lcd.clear();
  typeText(0, 0, "Hello from ESP32");   // a row holds 16 characters
  delay(1000);
  typeText(0, 1, "Typing effect...");
}

void loop() {
  // nothing to do: the text stays on the display
}
