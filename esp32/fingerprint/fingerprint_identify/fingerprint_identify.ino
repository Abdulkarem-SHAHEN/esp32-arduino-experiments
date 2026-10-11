/*
 * Fingerprint sensor, step 3: recognise enrolled fingers (ESP32)
 * --------------------------------------------------------------
 * Waits for a finger, searches the prints stored in the sensor and prints the ID of the
 * match with its confidence score. This is the building block of an access control:
 * replace the Serial message with the action to perform (open a lock, log the entry...).
 *
 * Adapted to the ESP32 (hardware UART2) from the "fingerprint" example of the Adafruit
 * Fingerprint Sensor Library.
 *
 * Sensor:  AS608 / R307 optical fingerprint module
 * Wiring:  sensor TX -> GPIO 16 (RX2), sensor RX -> GPIO 17 (TX2), VCC -> 3.3 V, GND -> GND
 *          (some modules need 5 V: check the marking on yours)
 * Library: Adafruit Fingerprint Sensor Library
 * First:   enroll at least one finger with fingerprint_enroll
 */

#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

#define RXD2 16   // receives from the sensor TX
#define TXD2 17   // sends to the sensor RX

HardwareSerial sensorSerial(2);
Adafruit_Fingerprint finger(&sensorSerial);

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\nFingerprint identification");

  sensorSerial.begin(57600, SERIAL_8N1, RXD2, TXD2);
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor found.");
  } else {
    Serial.println("Fingerprint sensor not found: check the wiring.");
    while (true) {
      delay(10);
    }
  }

  finger.getTemplateCount();
  if (finger.templateCount == 0) {
    Serial.println("The sensor holds no fingerprint yet: run fingerprint_enroll first.");
  } else {
    Serial.print("The sensor holds ");
    Serial.print(finger.templateCount);
    Serial.println(" fingerprint(s). Waiting for a finger...");
  }
}

// Scans the finger on the sensor and looks it up.
// Returns the ID of the matching print, 0 when the finger is unknown, and -1 when there
// is no finger on the sensor or the scan failed.
int identifyFinger() {
  uint8_t result = finger.getImage();
  if (result == FINGERPRINT_NOFINGER) {
    return -1;
  }
  if (result != FINGERPRINT_OK) {
    Serial.println("The image could not be taken.");
    return -1;
  }

  // extract the features of the image, then search the stored prints for them
  if (finger.image2Tz() != FINGERPRINT_OK) {
    Serial.println("The image is not clear enough, try again.");
    return -1;
  }

  result = finger.fingerFastSearch();
  if (result == FINGERPRINT_NOTFOUND) {
    return 0;
  }
  if (result != FINGERPRINT_OK) {
    Serial.println("Communication error during the search.");
    return -1;
  }
  return finger.fingerID;
}

void loop() {
  int id = identifyFinger();

  if (id > 0) {
    Serial.print("Match: ID #");
    Serial.print(id);
    Serial.print(", confidence ");
    Serial.println(finger.confidence);
    delay(1000);   // do not report the same finger several times
  } else if (id == 0) {
    Serial.println("No match: this finger is not enrolled.");
    delay(1000);
  }

  delay(200);
}
