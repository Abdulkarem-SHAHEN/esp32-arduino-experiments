/*
 * Ultrasonic distance sensor HC-SR04 (ESP32)
 * ------------------------------------------
 * Measures a distance twice a second and prints it in centimetres.
 *
 * The sensor sends a burst of ultrasound and raises ECHO for as long as the sound takes
 * to come back. Sound travels at about 343 m/s, that is 0.0343 cm per microsecond, and
 * the burst covers the distance twice (there and back), hence the division by 2.
 *
 * Wiring: TRIG -> GPIO 5, VCC -> 5 V, GND -> GND
 *         ECHO -> GPIO 18 through a voltage divider (the ECHO signal is 5 V and the
 *         ESP32 pins accept 3.3 V)
 */

#define TRIG_PIN 5
#define ECHO_PIN 18

const unsigned long ECHO_TIMEOUT_US = 30000;   // about 5 m: beyond the range of the sensor

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  // a 10 microsecond pulse on TRIG starts a measurement
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // length of the ECHO pulse, 0 if nothing came back in time
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);

  if (duration == 0) {
    Serial.println("Out of range");
  } else {
    float distance = duration * 0.0343 / 2;
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  }

  delay(500);
}
