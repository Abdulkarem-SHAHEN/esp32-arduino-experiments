/*
 * IR remote multi-tool (Arduino UNO)
 * ----------------------------------
 * One infrared remote control drives three small applications. The three keys of the
 * top row choose the application:
 *
 *   CH-  LED ring     one of four LEDs is lit; NEXT / PREV move it around, the keys
 *                     1 to 4 jump to a LED, PLAY switches the ring on and off
 *   CH   Calculator   whole numbers typed on the remote, VOL+ adds, VOL- subtracts,
 *                     EQ shows the result; everything is displayed on the Serial Monitor
 *   CH+  Blinker      an LED blinks; VOL+ makes the blink slower and VOL- faster,
 *                     in steps of 100 ms (0 = always on, 800 ms at most)
 *
 * Project made with Naim Ayroud.
 *
 * Remote:   21-key "Car MP3" remote (NEC protocol). Another remote works too: print
 *           the code of each key on the Serial Monitor and replace the KEY_ values.
 * Wiring:   IR receiver (VS1838B or similar) OUT -> D3, VCC -> 5 V, GND -> GND
 *           ring LEDs on D8, D9, D10 and D11, blink LED on D12, each through a 220 Ohm resistor
 * Library:  IRremote (version 3 or newer)
 * Monitor:  9600 baud
 */

#include <IRremote.h>

// ---------------------------------------------------------------- pins
const uint8_t IR_RECEIVER_PIN = 3;
const uint8_t RING_PINS[] = {8, 9, 10, 11};
const uint8_t RING_SIZE = sizeof(RING_PINS) / sizeof(RING_PINS[0]);
const uint8_t BLINK_PIN = 12;

// ---------------------------------------------------------------- remote key codes
const uint32_t KEY_CH_MINUS = 0xBA45FF00;   // application 1: LED ring
const uint32_t KEY_CH       = 0xB946FF00;   // application 2: calculator
const uint32_t KEY_CH_PLUS  = 0xB847FF00;   // application 3: blinker
const uint32_t KEY_PREV     = 0xBB44FF00;
const uint32_t KEY_NEXT     = 0xBF40FF00;
const uint32_t KEY_PLAY     = 0xBC43FF00;
const uint32_t KEY_MINUS    = 0xF807FF00;   // VOL-
const uint32_t KEY_PLUS     = 0xEA15FF00;   // VOL+
const uint32_t KEY_EQUALS   = 0xF609FF00;   // EQ
const uint32_t KEY_DIGITS[10] = {
  0xE916FF00, 0xF30CFF00, 0xE718FF00, 0xA15EFF00, 0xF708FF00,   // 0 1 2 3 4
  0xE31CFF00, 0xA55AFF00, 0xBD42FF00, 0xAD52FF00, 0xB54AFF00    // 5 6 7 8 9
};

// ---------------------------------------------------------------- state
enum Mode { MODE_RING, MODE_CALCULATOR, MODE_BLINK };
Mode mode = MODE_RING;

IRrecv IR(IR_RECEIVER_PIN);

// LED ring
bool ringOn = false;
uint8_t ringPosition = 0;            // index of the lit LED, 0 to RING_SIZE - 1

// Calculator. A long holds 9 digits on the UNO, where an int would stop at 32767.
const uint8_t MAX_DIGITS = 9;
long firstNumber = 0;
long currentNumber = 0;              // the number being typed
uint8_t digitCount = 0;              // digits typed so far for currentNumber
char pendingOperator = 0;            // '+', '-', or 0 while the first number is typed

// Blinker
const int BLINK_STEP_MS = 100;
const int BLINK_MAX_MS = 800;
int blinkDelay = 0;                  // 0 = LED always on
bool blinkState = false;
unsigned long lastToggle = 0;

// ---------------------------------------------------------------- helpers
// The digit printed on a key, or -1 if the code is not a digit key.
int digitOf(uint32_t code) {
  for (int d = 0; d < 10; d++) {
    if (KEY_DIGITS[d] == code) {
      return d;
    }
  }
  return -1;
}

void ringOff() {
  for (uint8_t i = 0; i < RING_SIZE; i++) {
    digitalWrite(RING_PINS[i], LOW);
  }
}

// Lights the LED at ringPosition and no other one, or none while the ring is off.
void showRing() {
  for (uint8_t i = 0; i < RING_SIZE; i++) {
    digitalWrite(RING_PINS[i], (ringOn && i == ringPosition) ? HIGH : LOW);
  }
}

void resetCalculator() {
  firstNumber = 0;
  currentNumber = 0;
  digitCount = 0;
  pendingOperator = 0;
}

long applyOperator(long a, char op, long b) {
  return op == '+' ? a + b : a - b;
}

// ---------------------------------------------------------------- application 1: LED ring
void handleRingKey(uint32_t code) {
  if (code == KEY_PLAY) {
    ringOn = !ringOn;
  } else if (ringOn) {
    int digit = digitOf(code);
    if (code == KEY_NEXT) {
      ringPosition = (ringPosition + 1) % RING_SIZE;
    } else if (code == KEY_PREV) {
      ringPosition = (ringPosition + RING_SIZE - 1) % RING_SIZE;
    } else if (digit >= 1 && digit <= RING_SIZE) {
      ringPosition = digit - 1;
    }
  }
  showRing();
}

// ---------------------------------------------------------------- application 2: calculator
void handleCalculatorKey(uint32_t code) {
  int digit = digitOf(code);

  if (digit >= 0) {
    if (digitCount < MAX_DIGITS) {
      currentNumber = currentNumber * 10 + digit;
      digitCount++;
      Serial.print(digit);
    }

  } else if (code == KEY_PLUS || code == KEY_MINUS) {
    if (digitCount == 0) {
      Serial.println(F("\nEnter a number first"));
      return;
    }
    // "12 + 5 -" continues the calculation with 17 as the first number
    firstNumber = pendingOperator ? applyOperator(firstNumber, pendingOperator, currentNumber) : currentNumber;
    pendingOperator = (code == KEY_PLUS) ? '+' : '-';
    currentNumber = 0;
    digitCount = 0;
    Serial.print(pendingOperator == '+' ? F(" + ") : F(" - "));

  } else if (code == KEY_EQUALS) {
    if (!pendingOperator || digitCount == 0) {
      Serial.println(F("\nEnter two numbers and an operation first"));
      return;
    }
    Serial.print(F(" = "));
    Serial.println(applyOperator(firstNumber, pendingOperator, currentNumber));
    resetCalculator();
  }
}

// ---------------------------------------------------------------- application 3: blinker
void handleBlinkKey(uint32_t code) {
  if (code == KEY_PLUS) {
    if (blinkDelay < BLINK_MAX_MS) {
      blinkDelay += BLINK_STEP_MS;
    } else {
      Serial.println(F("The delay cannot go above 800 ms"));
      return;
    }
  } else if (code == KEY_MINUS) {
    if (blinkDelay > 0) {
      blinkDelay -= BLINK_STEP_MS;
    } else {
      Serial.println(F("The delay cannot go below 0"));
      return;
    }
  } else {
    return;
  }
  Serial.print(F("The delay is now "));
  Serial.print(blinkDelay);
  Serial.println(F(" ms"));
}

// Called on every pass of loop(): toggles the LED when its delay has elapsed. Using
// millis() instead of delay() keeps the remote responsive while the LED blinks.
void updateBlink() {
  if (mode != MODE_BLINK) {
    return;
  }
  if (blinkDelay == 0) {
    digitalWrite(BLINK_PIN, HIGH);
  } else if (millis() - lastToggle >= (unsigned long)blinkDelay) {
    blinkState = !blinkState;
    digitalWrite(BLINK_PIN, blinkState ? HIGH : LOW);
    lastToggle = millis();
  }
}

// ---------------------------------------------------------------- choosing the application
// Switches application and leaves the outputs of the previous one off.
void enterMode(Mode newMode) {
  mode = newMode;
  ringOff();
  digitalWrite(BLINK_PIN, LOW);

  if (mode == MODE_RING) {
    Serial.println(F("\nMode 1: LED ring"));
    showRing();
  } else if (mode == MODE_CALCULATOR) {
    Serial.println(F("\nMode 2: calculator"));
    resetCalculator();
  } else {
    Serial.println(F("\nMode 3: blinker"));
    blinkState = true;
    lastToggle = millis();
    digitalWrite(BLINK_PIN, HIGH);
  }
}

void handleKey(uint32_t code) {
  if (code == KEY_CH_MINUS) {
    enterMode(MODE_RING);
  } else if (code == KEY_CH) {
    enterMode(MODE_CALCULATOR);
  } else if (code == KEY_CH_PLUS) {
    enterMode(MODE_BLINK);
  } else if (mode == MODE_RING) {
    handleRingKey(code);
  } else if (mode == MODE_CALCULATOR) {
    handleCalculatorKey(code);
  } else {
    handleBlinkKey(code);
  }
}

// ---------------------------------------------------------------- Arduino entry points
void setup() {
  Serial.begin(9600);
  IR.enableIRIn();

  for (uint8_t i = 0; i < RING_SIZE; i++) {
    pinMode(RING_PINS[i], OUTPUT);
  }
  pinMode(BLINK_PIN, OUTPUT);

  Serial.println(F("IR remote multi-tool, by Naim Ayroud and Abdulkarem Shahen"));
  Serial.println(F("Choose a mode with the three top keys: CH- LED ring, CH calculator, CH+ blinker"));
}

void loop() {
  updateBlink();

  if (IR.decode()) {
    uint32_t code = IR.decodedIRData.decodedRawData;
    // a key that is held down sends "repeat" frames, which are not new key presses
    bool isRepeat = (IR.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) || code == 0;
    if (!isRepeat) {
      handleKey(code);
    }
    IR.resume();
  }
}
