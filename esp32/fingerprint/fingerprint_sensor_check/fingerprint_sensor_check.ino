/*
 * Fingerprint sensor, step 1: is it wired correctly? (ESP32)
 * ----------------------------------------------------------
 * Talks to the sensor once and reports whether it answers. Run this before the two
 * other fingerprint sketches: most problems come from TX and RX being swapped.
 *
 * Sensor:  AS608 / R307 optical fingerprint module
 * Wiring:  sensor TX -> GPIO 16 (RX2), sensor RX -> GPIO 17 (TX2), VCC -> 3.3 V, GND -> GND
 *          (some modules need 5 V: check the marking on yours)
 * Library: Adafruit Fingerprint Sensor Library
 */

#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

#define RXD2 16   // receives from the sensor TX
#define TXD2 17   // sends to the sensor RX

HardwareSerial sensorSerial(2);   // UART2: UART0 stays free for the USB console
Adafruit_Fingerprint finger(&sensorSerial);

void setup() {
  Serial.begin(115200);
  sensorSerial.begin(57600, SERIAL_8N1, RXD2, TXD2);
  finger.begin(57600);

  Serial.println("Looking for the sensor...");
  if (finger.verifyPassword()) {
    Serial.println("Sensor found!");
  } else {
    Serial.println("Sensor not found: check the wiring (TX/RX swapped?) and the power supply.");
  }
}

void loop() {}
