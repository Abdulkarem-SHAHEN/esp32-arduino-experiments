/*
 * Fingerprint sensor, step 2: enroll a finger (ESP32)
 * ---------------------------------------------------
 * Stores a new fingerprint in the memory of the sensor under an ID chosen on the Serial
 * Monitor. The finger is scanned twice; the sensor merges the two scans into one model
 * and saves it, so the print stays in the sensor even when the ESP32 is reprogrammed.
 *
 * Adapted to the ESP32 (hardware UART2) from the "enroll" example of the Adafruit
 * Fingerprint Sensor Library.
 *
 * Sensor:  AS608 / R307 optical fingerprint module
 * Wiring:  sensor TX -> GPIO 16 (RX2), sensor RX -> GPIO 17 (TX2), VCC -> 3.3 V, GND -> GND
 *          (some modules need 5 V: check the marking on yours)
 * Library: Adafruit Fingerprint Sensor Library
 * Monitor: 115200 baud; type an ID from 1 to 127 and press Enter
 */

#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

#define RXD2 16   // receives from the sensor TX
#define TXD2 17   // sends to the sensor RX

HardwareSerial sensorSerial(2);
Adafruit_Fingerprint finger(&sensorSerial);

uint8_t id;   // slot in which the finger is being enrolled

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\nFingerprint enrollment");

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

  finger.getParameters();
  Serial.print("Capacity: ");
  Serial.println(finger.capacity);
  Serial.print("Security level: ");
  Serial.println(finger.security_level);
}

// Waits for a number typed on the Serial Monitor (0 is not a valid ID).
uint8_t readNumber() {
  uint8_t number = 0;
  while (number == 0) {
    while (!Serial.available()) {
      delay(10);
    }
    number = Serial.parseInt();
  }
  return number;
}

// Blocks until the sensor has taken an image of a finger.
void waitForImage() {
  int result = -1;
  while (result != FINGERPRINT_OK) {
    result = finger.getImage();
    switch (result) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
    delay(100);
  }
}

// Runs the whole enrollment for `id`. Returns true when the print was stored.
bool enrollFingerprint() {
  Serial.print("Place the finger to enroll as #");
  Serial.println(id);

  // first scan -> feature buffer 1
  waitForImage();
  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    Serial.println("The image could not be converted, try again.");
    return false;
  }

  Serial.println("Remove the finger");
  delay(2000);
  while (finger.getImage() != FINGERPRINT_NOFINGER) {
    delay(50);
  }

  // second scan of the same finger -> feature buffer 2
  Serial.println("Place the same finger again");
  waitForImage();
  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    Serial.println("The second image could not be converted, try again.");
    return false;
  }

  // the two scans must agree before a model is built from them
  if (finger.createModel() != FINGERPRINT_OK) {
    Serial.println("The two scans did not match, try again.");
    return false;
  }

  if (finger.storeModel(id) != FINGERPRINT_OK) {
    Serial.println("The fingerprint could not be stored.");
    return false;
  }

  Serial.print("Stored as #");
  Serial.println(id);
  return true;
}

void loop() {
  Serial.println("\nReady to enroll a fingerprint.");
  Serial.println("Type the ID (1 to 127) to save this finger as:");
  id = readNumber();
  if (id > 127) {
    Serial.println("The ID must be between 1 and 127.");
    return;
  }

  while (!enrollFingerprint()) {
    // keep asking until the finger has been enrolled
  }
}
