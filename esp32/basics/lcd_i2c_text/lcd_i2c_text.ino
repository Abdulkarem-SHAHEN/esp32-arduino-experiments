/*
 * 16x2 LCD over I2C: fixed text (ESP32)
 * -------------------------------------
 * Writes one line of text on each row of the display.
 *
 * Wiring:  SDA -> GPIO 21, SCL -> GPIO 22, VCC -> 5 V, GND -> GND
 * Library: LiquidCrystal I2C
 * Note:    0x27 is the usual address of the I2C backpack; some modules use 0x3F.
 */

#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);   // address, columns, rows

void setup() {
  Wire.begin(21, 22);   // SDA, SCL
  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);   // column, row
  lcd.print("AuraHome");
  lcd.setCursor(0, 1);
  lcd.print("123 . . . ");
}

void loop() {
  // nothing to do: the text stays on the display
}
