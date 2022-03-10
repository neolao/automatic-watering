#include <Encoder.h> // http://www.pjrc.com/teensy/td_libs_Encoder.html
#include <LowPower.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BUTTON_CLK_PIN 4
#define BUTTON_DT_PIN 3
#define BUTTON_SW_PIN 2
#define ENCODER_DO_NOT_USE_INTERRUPTS

//#define DELAY_POWER_SAVER 60000
#define DELAY_POWER_SAVER 10000


Adafruit_SSD1306 screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Encoder rotaryButton(BUTTON_CLK_PIN, BUTTON_DT_PIN);
long rotaryButtonValue = 0;
long counter = 0;
long lastActivityTime;

void TimerHandler() {
  static bool toggle = false;

  Serial.println("Hop: " + millis());
  toggle = !toggle;
}

void setup() {
  /*
  Serial.begin(9600);
  Serial.println(F("Initialize System"));
  Serial.println(BOARD_TYPE);
  Serial.println(TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));
  */
  setupPowerSaver();
  setupScreen();
  setupRotaryButton();
}

void loop() {
  checkPowerSaver();

  long newPosition = rotaryButton.read();
  if (newPosition != rotaryButtonValue) {
    rotaryButtonValue = newPosition;
  }


  if (digitalRead(BUTTON_SW_PIN) == LOW) {
    rotaryButtonValue = 0;
  }


  rotaryButtonValue = (rotaryButtonValue / 4) % 20;

  
  counter++;

  updateScreen();
}

void setupPowerSaver() {
  lastActivityTime = millis();
}

void checkPowerSaver() {
  if ((millis() - lastActivityTime) > DELAY_POWER_SAVER) {
    powerDown();
    checkPowerSaver();
  }
}

void setupScreen() {
  if (!screen.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    //Serial.println("Screen allocation failed");
    for (;;);
  }

  screen.clearDisplay();
  screen.setTextWrap(false);

  screen.setTextSize(2);
  screen.setTextColor(WHITE);
  screen.display();
}

void setupRotaryButton() {
  pinMode(BUTTON_SW_PIN, INPUT_PULLUP);
}

void updateScreen() {
  screen.clearDisplay();

  // Header
  long powerSaverCountdown = DELAY_POWER_SAVER - (millis() - lastActivityTime);
  screen.setCursor(0, 0);
  //*
  // Debug
  drawDigit(0, 0, 0);
  drawDigit(1, 5, 0);
  drawDigit(2, 10, 0);
  drawDigit(3, 15, 0);
  drawDigit(4, 20, 0);
  drawDigit(5, 25, 0);
  drawDigit(6, 30, 0);
  drawDigit(7, 35, 0);
  drawDigit(8, 40, 0);
  drawDigit(9, 45, 0);
  //*/

  // Separator
  screen.drawFastHLine(0, 7, SCREEN_WIDTH, WHITE);

  //
  screen.setCursor(0, 9);
  screen.println(powerSaverCountdown);
  screen.println(rotaryButtonValue);

  // Update
  screen.display();
}

void powerDown() {
  screen.clearDisplay();
  screen.display();
  
  attachInterrupt(digitalPinToInterrupt(BUTTON_SW_PIN), wakeUp, LOW);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  //LowPower.powerSave(SLEEP_FOREVER, ADC_OFF, BOD_OFF, TIMER2_ON);
  detachInterrupt(digitalPinToInterrupt(BUTTON_SW_PIN));
}

void wakeUp() {
  lastActivityTime = millis();
}

void drawDigit(uint8_t digit, uint8_t x, uint8_t y) {
  switch (digit) {
    case 0:
      screen.drawRect(x, y, 3, 5, WHITE);
      break;
    case 1:
      screen.drawFastVLine(x + 1, y, 5, WHITE);
      screen.drawFastHLine(x, y + 4, 3, WHITE);
      screen.drawPixel(x, y + 1, WHITE);
      break;
    case 2:
      screen.drawFastHLine(x, y, 3, WHITE);
      screen.drawPixel(x + 2, y + 1, WHITE);
      screen.drawFastHLine(x, y + 2, 3, WHITE);
      screen.drawPixel(x, y + 3, WHITE);
      screen.drawFastHLine(x, y + 4, 3, WHITE);
      break;
    case 3:
      screen.drawFastHLine(x, y, 3, WHITE);
      screen.drawFastHLine(x, y + 2, 3, WHITE);
      screen.drawFastHLine(x, y + 4, 3, WHITE);
      screen.drawFastVLine(x + 2, y, 5, WHITE);
      break;
  }
}
