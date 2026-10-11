// Copy this file to "secrets.h" (same folder) and fill in your own values.
// secrets.h is listed in .gitignore, so your credentials are never committed.

#define WIFI_SSID "your-wifi-name"
#define WIFI_PASSWORD "your-wifi-password"

// Token given by @BotFather when the bot is created
#define BOT_TOKEN "123456789:replace-with-your-bot-token"

// Chat ids of everyone who receives the alerts (each one is given by @myidbot).
// Every recipient has to send /start to the bot once, or the bot cannot write to them.
const char *RECIPIENTS[] = {
  "000000000",
  "111111111",
};

// Picture sent with every alert: a direct, public link to a .jpg or .png file.
// Telegram downloads it itself, so the ESP32 only sends this address.
#define PHOTO_URL "https://example.com/picture.jpg"
