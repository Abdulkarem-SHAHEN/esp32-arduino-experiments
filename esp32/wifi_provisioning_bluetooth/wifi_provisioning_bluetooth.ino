/*
 * Wi-Fi provisioning over Bluetooth (ESP32)
 * -----------------------------------------
 * Normally the Wi-Fi name and password are written in the code, so when the router or
 * its password changes, the board has to be connected to a computer and flashed again.
 *
 * Here the credentials are entered from a phone instead: the ESP32 exposes a Bluetooth
 * serial port, asks for the new network name and password, tries them, and stores them
 * in flash memory (NVS) once they work. They survive resets and power cuts, and the
 * firmware never has to be touched again.
 *
 * Once on the network, the board serves a small web page that switches a pin on and
 * off, to show that it is reachable at its new address.
 *
 * Phone side: any "Serial Bluetooth Terminal" app, paired with ESP32_BT_Config.
 *   1  change the Wi-Fi network (asks for the setup PIN, the SSID, then the password)
 *   2  show the current connection
 *
 * Board:  ESP32 with classic Bluetooth (ESP32-WROOM / DevKit v1; not S2, S3 or C3)
 * Wiring: LED or relay input on GPIO 2 (the on-board LED of most dev kits)
 * Note:   Wi-Fi and Bluetooth together make a large program. If the IDE reports that
 *         the sketch is too big, select Tools > Partition Scheme > Huge APP.
 */

#include <BluetoothSerial.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>

// ---------------------------------------------------------------- settings
const char *BT_DEVICE_NAME = "ESP32_BT_Config";
const char *SETUP_PIN = "1234";               // change it: it protects the Wi-Fi settings
const int CONTROL_PIN = 2;
const unsigned long WIFI_TIMEOUT_MS = 10000;  // how long one connection attempt may take

// ---------------------------------------------------------------- state
BluetoothSerial SerialBT;
WebServer server(80);
Preferences preferences;

String ssid;       // network in use, loaded from flash at start-up
String password;

// The Bluetooth dialogue is a small state machine: each line received from the phone
// is interpreted according to the question that was asked last.
enum Step { IDLE, WAIT_PIN, WAIT_SSID, WAIT_PASSWORD };
Step step = IDLE;
String candidateSsid;   // typed by the user, not yet tested

// ---------------------------------------------------------------- stored credentials
void loadCredentials() {
  preferences.begin("wifi", true);            // read-only
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  preferences.end();
}

void saveCredentials() {
  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.end();
}

// ---------------------------------------------------------------- Wi-Fi
// Tries to join a network and reports whether it worked within the timeout.
bool connectWiFi(const String &name, const String &pass) {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(name.c_str(), pass.c_str());
  Serial.printf("Connecting to Wi-Fi \"%s\"", name.c_str());

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT_MS) {
    delay(500);
    Serial.print(".");
  }

  bool connected = WiFi.status() == WL_CONNECTED;
  if (connected) {
    Serial.printf("\nConnected, IP address: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\nConnection failed.");
  }
  return connected;
}

void printStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    SerialBT.printf("Connected to \"%s\", IP address %s\n", ssid.c_str(), WiFi.localIP().toString().c_str());
  } else if (ssid.length() > 0) {
    SerialBT.printf("Not connected. Saved network: \"%s\"\n", ssid.c_str());
  } else {
    SerialBT.println("Not connected. No network saved yet.");
  }
}

void printMenu() {
  SerialBT.println("1: change the Wi-Fi network");
  SerialBT.println("2: show the connection");
}

// ---------------------------------------------------------------- Bluetooth dialogue
void handleBluetoothLine(String line) {
  line.trim();

  switch (step) {
    case IDLE:
      if (line == "1") {
        SerialBT.println("Setup PIN:");
        step = WAIT_PIN;
      } else if (line == "2") {
        printStatus();
      } else {
        printMenu();
      }
      break;

    case WAIT_PIN:
      if (line == SETUP_PIN) {
        SerialBT.println("New network name (SSID):");
        step = WAIT_SSID;
      } else {
        SerialBT.println("Wrong PIN.");
        step = IDLE;
      }
      break;

    case WAIT_SSID:
      if (line.length() == 0) {
        SerialBT.println("The name cannot be empty. New network name (SSID):");
      } else {
        candidateSsid = line;
        SerialBT.println("Password:");
        step = WAIT_PASSWORD;
      }
      break;

    case WAIT_PASSWORD:
      step = IDLE;
      SerialBT.printf("Trying \"%s\" ...\n", candidateSsid.c_str());
      if (connectWiFi(candidateSsid, line)) {
        // only credentials that work are kept
        ssid = candidateSsid;
        password = line;
        saveCredentials();
        SerialBT.println("Saved.");
        printStatus();
      } else {
        SerialBT.println("Could not connect: check the name and the password.");
        if (ssid.length() > 0) {
          // a typing mistake must not take the board off its working network
          SerialBT.printf("Going back to \"%s\" ...\n", ssid.c_str());
          connectWiFi(ssid, password);
          printStatus();
        }
      }
      break;
  }
}

// ---------------------------------------------------------------- web page
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'><title>ESP32 control</title></head><body>";
  html += "<h1>ESP32 control</h1>";
  html += String("<p>The output is ") + (digitalRead(CONTROL_PIN) == HIGH ? "ON" : "OFF") + "</p>";
  html += "<form action='/on' method='GET'><button type='submit'>ON</button></form>";
  html += "<form action='/off' method='GET'><button type='submit'>OFF</button></form>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// Switch the output, then send the browser back to the main page.
void handleOn() {
  digitalWrite(CONTROL_PIN, HIGH);
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleOff() {
  digitalWrite(CONTROL_PIN, LOW);
  server.sendHeader("Location", "/");
  server.send(303);
}

// ---------------------------------------------------------------- Arduino entry points
void setup() {
  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, LOW);

  Serial.begin(115200);
  SerialBT.begin(BT_DEVICE_NAME);
  Serial.printf("Bluetooth started as \"%s\".\n", BT_DEVICE_NAME);

  loadCredentials();
  if (ssid.length() > 0) {
    connectWiFi(ssid, password);
  } else {
    Serial.println("No Wi-Fi network saved yet: configure one over Bluetooth.");
  }

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.begin();
}

void loop() {
  if (SerialBT.available()) {
    handleBluetoothLine(SerialBT.readStringUntil('\n'));
  }

  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
  }
}
