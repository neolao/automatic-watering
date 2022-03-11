#include <Encoder.h> // http://www.pjrc.com/teensy/td_libs_Encoder.html
#include <LowPower.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BUTTON_CLK_PIN 4
#define BUTTON_DT_PIN 3
#define BUTTON_SW_PIN 2
//#define ENCODER_DO_NOT_USE_INTERRUPTS

#define MAX_VOLT 4.2
#define MIN_VOLT 3.2
#define BATTERY_GAUGE_WIDTH 10

#define DELAY_POWER_SAVER 20000


Adafruit_SSD1306 screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Encoder rotaryButton(BUTTON_CLK_PIN, BUTTON_DT_PIN);
long rotaryButtonValue = 0;
long counter = 0;
long lastActivityTime;

enum Page
{
  PAGE_HOME,
  PAGE_PLANT
};
Page currentPage = PAGE_HOME;
uint8_t homePageMenuIndex = 0;
uint8_t plantPageMenuIndex = 0;

void setup() {
  /*
  Serial.begin(2000000);
  Serial.println(F("Initialize System"));
  //*/
  
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
    wakeUp();
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

  // Header: Power Save Countdown
  long powerSaverCountdown = (DELAY_POWER_SAVER - (millis() - lastActivityTime)) / 1000;
  drawNumber(powerSaverCountdown, 0, 0);

  // Header: Battery
  double currentVolt = readVcc() / 1000.0;
  double batteryLevel = currentVolt - MIN_VOLT;
  uint8_t batteryPercent = (batteryLevel / (MAX_VOLT - MIN_VOLT)) * 100;
  drawBattery(batteryPercent, SCREEN_WIDTH - BATTERY_GAUGE_WIDTH, 0);
  

  // Separator
  screen.drawFastHLine(0, 6, SCREEN_WIDTH, WHITE);

  // Page
  switch (currentPage) {
    case PAGE_HOME:
      displayHomePage();
      break;
  }
  /*
  screen.setCursor(0, 9);
  screen.println(rotaryButtonValue);
  screen.println(millis());
  */

  // Update
  screen.display();
}

void displayHomePage() {
  // Water gauge
  uint8_t startX = 0;
  uint8_t startY = 10;
  screen.drawFastVLine(startX, startY, 40, WHITE);
  screen.drawFastVLine(startX + 10, startY, 40, WHITE);
  screen.drawFastHLine(startX, startY + 39, 11, WHITE);

  // Water icon
  uint8_t iconX = 0;
  uint8_t iconY = 54;
  screen.drawFastHLine(iconX + 4, iconY, 2, WHITE);
  screen.drawFastHLine(iconX + 5, iconY + 1, 2, WHITE);
  screen.drawFastHLine(iconX + 5, iconY + 2, 3, WHITE);
  screen.drawFastHLine(iconX + 4, iconY + 3, 4, WHITE);
  screen.drawFastHLine(iconX + 3, iconY + 4, 6, WHITE);
  screen.drawFastHLine(iconX + 2, iconY + 5, 7, WHITE);
  screen.drawFastHLine(iconX + 2, iconY + 6, 7, WHITE);
  screen.drawFastHLine(iconX + 2, iconY + 7, 7, WHITE);
  screen.drawFastHLine(iconX + 3, iconY + 8, 5, WHITE);
  screen.drawFastHLine(iconX + 4, iconY + 9, 3, WHITE);
  screen.drawFastVLine(iconX + 7, iconY + 5, 2, BLACK);
  screen.drawFastHLine(iconX + 6, iconY + 7, 2, BLACK);
  screen.drawFastHLine(iconX + 5, iconY + 8, 2, BLACK);
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
    case 4:
      screen.drawFastVLine(x, y, 2, WHITE);
      screen.drawFastHLine(x, y + 2, 3, WHITE);
      screen.drawFastVLine(x + 2, y, 5, WHITE);
      break;
    case 5:
      screen.drawFastHLine(x, y, 3, WHITE);
      screen.drawPixel(x, y + 1, WHITE);
      screen.drawFastHLine(x, y + 2, 3, WHITE);
      screen.drawPixel(x + 2, y + 3, WHITE);
      screen.drawFastHLine(x, y + 4, 3, WHITE);
      break;
    case 6:
      screen.drawFastHLine(x, y, 3, WHITE);
      screen.drawPixel(x, y + 1, WHITE);
      screen.drawRect(x, y + 2, 3, 3, WHITE);
      break;
    case 7:
      screen.drawFastHLine(x, y, 3, WHITE);
      screen.drawFastVLine(x + 2, y, 5, WHITE);
      break;
    case 8:
      screen.drawRect(x, y, 3, 3, WHITE);
      screen.drawRect(x, y + 2, 3, 3, WHITE);
      break;
    case 9:
      screen.drawFastHLine(x, y + 4, 3, WHITE);
      screen.drawPixel(x + 2, y + 3, WHITE);
      screen.drawRect(x, y, 3, 3, WHITE);
      break;
  }
}

void drawNumber(long value, uint8_t x, uint8_t y) {
  uint8_t digit;
  uint8_t offset = 0;

  if (value >= 10000) {
    digit = (value / 10000) % 10;
    drawDigit(digit, x + offset, y);
    offset += 5;
  }
  if (value >= 1000) {
    digit = (value / 1000) % 10;
    drawDigit(digit, x + offset, y);
    offset += 5;
  }
  if (value >= 100) {
    digit = (value / 100) % 10;
    drawDigit(digit, x + offset, y);
    offset += 5;
  }
  if (value >= 10) {
    digit = (value / 10) % 10;
    drawDigit(digit, x + offset, y);
    offset += 5;
  }
  
  digit = value % 10;
  drawDigit(digit, x + offset, y);
}

void drawBattery(uint8_t percent, uint8_t x, uint8_t y) {
  screen.drawRect(x, y, BATTERY_GAUGE_WIDTH, 5, WHITE);

  uint8_t gauge = percent / BATTERY_GAUGE_WIDTH;

  for (uint8_t offset = 0; offset < gauge; offset++) {
    screen.drawFastVLine(x + offset, y, 5, WHITE);
  }
  
  screen.drawPixel(x + BATTERY_GAUGE_WIDTH - 1, y, BLACK);
  screen.drawPixel(x + BATTERY_GAUGE_WIDTH - 1, y + 4, BLACK);
  screen.drawPixel(x + BATTERY_GAUGE_WIDTH - 2, y + 1, WHITE);
  screen.drawPixel(x + BATTERY_GAUGE_WIDTH - 2, y + 3, WHITE);
}

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high << 8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}
