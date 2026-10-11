/*
 * Telegram alert to several people, with retries (ESP32)
 * ------------------------------------------------------
 * Same idea as telegram_alert_basic, made more dependable:
 *   - the alert goes to a list of recipients instead of one chat;
 *   - a message that fails is sent again, waiting a little longer each time;
 *   - the start-up message carries the network name and the IP address, which is
 *     handy to find the board on the network;
 *   - a dropped Wi-Fi connection is restored before the next alert.
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
const unsigned long READ_INTERVAL_MS = 1000;
const unsigned long ALERT_COOLDOWN_MS = 60000;   // at most one alert per minute
const int MAX_RETRIES = 2;                       // extra attempts after a failed send
const int NUM_RECIPIENTS = sizeof(RECIPIENTS) / sizeof(RECIPIENTS[0]);

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

unsigned long lastAlert = 0;
bool alertSent = false;

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected");
}

// Sends one message; on failure tries again after 1 s, then 2 s.
bool sendWithRetry(const String &chatId, const String &text) {
  for (int attempt = 0; attempt <= MAX_RETRIES; attempt++) {
    if (bot.sendMessage(chatId, text, "")) {
      return true;
    }
    Serial.printf("Sending to %s failed (attempt %d)\n", chatId.c_str(), attempt + 1);
    delay(1000 * (attempt + 1));
  }
  return false;
}

void sendToAll(const String &text) {
  for (int i = 0; i < NUM_RECIPIENTS; i++) {
    if (!sendWithRetry(RECIPIENTS[i], text)) {
      Serial.printf("Could not deliver to %s\n", RECIPIENTS[i]);
    }
    delay(800);   // stay well under the rate limit of the Bot API
  }
}

void setup() {
  Serial.begin(115200);
  connectWiFi();

  // Root certificate of api.telegram.org: the connection is encrypted and the server verified
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  String bootMessage = "✅ Device started\n";
  bootMessage += "📶 Network: " + String(WIFI_SSID) + "\n";
  bootMessage += "📡 IP: " + WiFi.localIP().toString();
  sendToAll(bootMessage);
}

void loop() {
  int lightValue = analogRead(LDR_PIN);
  Serial.print("Light value: ");
  Serial.println(lightValue);

  // The cooldown keeps a reading that stays high from flooding the chats
  bool cooldownOver = !alertSent || millis() - lastAlert >= ALERT_COOLDOWN_MS;
  if (lightValue > LIGHT_THRESHOLD && cooldownOver) {
    if (WiFi.status() != WL_CONNECTED) {
      connectWiFi();
    }
    String alert = "⚠️ Alert: light above " + String(LIGHT_THRESHOLD) + "\n";
    alert += "📊 Current value: " + String(lightValue) + "\n";
    alert += "📡 IP: " + WiFi.localIP().toString();
    sendToAll(alert);
    lastAlert = millis();
    alertSent = true;
  }

  delay(READ_INTERVAL_MS);
}
