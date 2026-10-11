/*
 * Telegram alert, basic version (ESP32)
 * -------------------------------------
 * Reads a light sensor (LDR) and sends a Telegram message to one chat when the reading
 * goes above a threshold. The message arrives as a normal phone notification, with no
 * app to write and no server to run: the ESP32 talks to the Telegram Bot API directly
 * over HTTPS.
 *
 * Wiring:    LDR between 3.3 V and GPIO 34, 10 kOhm resistor between GPIO 34 and GND.
 *            GPIO 34 belongs to ADC1; the pins of ADC2 cannot be read while Wi-Fi is on.
 * Libraries: UniversalTelegramBot (Brian Lough), ArduinoJson
 * Setup:     copy secrets.example.h to secrets.h and fill it in.
 */

#include <ArduinoJson.h>
#include <UniversalTelegramBot.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "secrets.h"

const int LDR_PIN = 34;
const int LIGHT_THRESHOLD = 3000;                // ADC reading, 0 to 4095
const unsigned long READ_INTERVAL_MS = 500;
const unsigned long ALERT_COOLDOWN_MS = 60000;   // at most one alert per minute

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

unsigned long lastAlert = 0;
bool alertSent = false;

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected");

  // Root certificate of api.telegram.org: the connection is encrypted and the server verified
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  bot.sendMessage(CHAT_ID, "ESP32 started ✅", "");
}

void loop() {
  int lightValue = analogRead(LDR_PIN);
  Serial.print("Light value: ");
  Serial.println(lightValue);

  // The cooldown keeps a reading that stays high from flooding the chat
  bool cooldownOver = !alertSent || millis() - lastAlert >= ALERT_COOLDOWN_MS;
  if (lightValue > LIGHT_THRESHOLD && cooldownOver) {
    bot.sendMessage(CHAT_ID, "⚠️ Light above " + String(LIGHT_THRESHOLD) + ". Current value: " + String(lightValue), "");
    lastAlert = millis();
    alertSent = true;
  }

  delay(READ_INTERVAL_MS);
}
