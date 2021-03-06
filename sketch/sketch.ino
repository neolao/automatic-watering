#include <Encoder.h> // http://www.pjrc.com/teensy/td_libs_Encoder.html
#include <LowPower.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BUTTON_CLK_PIN 2
#define BUTTON_DT_PIN 4
#define BUTTON_SW_PIN 3
#define MOISTURE_SENSOR_1_PIN A0
#define MOISTURE_SENSOR_2_PIN A1
#define MOISTURE_SENSOR_3_PIN A2
#define MOISTURE_SENSOR_4_PIN A3
#define PUMP_1_PIN 5
#define PUMP_2_PIN 6
#define PUMP_3_PIN 7
#define PUMP_4_PIN 8
//#define ENCODER_DO_NOT_USE_INTERRUPTS

#define MAX_VOLT 4.2
#define MIN_VOLT 3.2
#define BATTERY_GAUGE_WIDTH 10

#define DELAY_POWER_SAVER 60000

Adafruit_SSD1306 screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Encoder rotaryButton(BUTTON_CLK_PIN, BUTTON_DT_PIN);
long rotaryPosition = 0;
long lastActivityTime;

enum Page {
  PAGE_HOME,
  PAGE_PLANT,
  PAGE_WATERING,
  PAGE_INTERVAL,
  PAGE_MOISTURE_THRESHOLD
};
Page currentPage = PAGE_HOME;
uint8_t homePageMenuIndex = 0;
uint8_t homePageMenuOffset = 0;

struct Plant {
  char* name;
  uint16_t moistureSensorPin;
  uint16_t minMoisture;
  uint16_t maxMoisture;
  uint16_t pumpPin;
  uint16_t pumpDuration; // Milliseconds
  uint8_t interval; // Hours
  uint16_t moistureThreshold;
};
#define PLANT_COUNT 8
Plant plants[PLANT_COUNT] = {
  {
    /* name              */ "Raisin", 
    /* moistureSensorPin */ MOISTURE_SENSOR_1_PIN, 
    /* minMoisture       */ 300, 
    /* maxMoisture .     */ 600, 
    /* pumpPin           */ PUMP_1_PIN, 
    /* pumpDuration      */ 2000, 
    /* interval          */ 24,
    /* moistureThreshold */ 500
   },
  {"Plante B", MOISTURE_SENSOR_2_PIN, 300, 600, PUMP_2_PIN, 2000, 24, 500},
  {"Plante C", MOISTURE_SENSOR_3_PIN, 300, 600, PUMP_3_PIN, 2000, 24, 500},
  {"Plante D", MOISTURE_SENSOR_4_PIN, 300, 600, PUMP_4_PIN, 2000, 24, 500},
  {"Plante E", MOISTURE_SENSOR_4_PIN, 300, 600, PUMP_4_PIN, 2000, 24, 500},
  {"Plante F", MOISTURE_SENSOR_4_PIN, 300, 600, PUMP_4_PIN, 2000, 24, 500},
  {"Plante G", MOISTURE_SENSOR_4_PIN, 300, 600, PUMP_4_PIN, 2000, 24, 500},
  {"Plante H", MOISTURE_SENSOR_4_PIN, 300, 600, PUMP_4_PIN, 2000, 24, 500}
};
uint8_t plantPageMenuIndex = 0;
uint8_t plantPageMenuOffset = 0;
#define PLANT_PAGE_MENU_SIZE 4
char *plantPageMenu[PLANT_PAGE_MENU_SIZE] = {
  "Retour",
  "Arroser",
  "Interval",
  "Seuil"
};
long pumpStartTime;

void setup() {
  /*
    Serial.begin(2000000);
    Serial.println(F("Initialize System"));
  //*/

  setupPowerSaver();
  setupScreen();
  setupRotaryButton();
  setupMoistureSensors();
  setupPumps();
}

void loop() {
  Plant plant = plants[homePageMenuIndex];
  
  if (currentPage == PAGE_WATERING) {
    if ((millis() - pumpStartTime) > plant.pumpDuration) {
      digitalWrite(plant.pumpPin, LOW);
      currentPage = PAGE_PLANT;
    } else {
      digitalWrite(plant.pumpPin, HIGH);
    }
  } else {
    checkPowerSaver();
  
    uint8_t newRotaryPosition = rotaryButton.read() / 4;
    if (newRotaryPosition != rotaryPosition) {
      if (newRotaryPosition > rotaryPosition) {
        onNext();
      } else {
        onPrevious();
      }
      rotaryPosition = newRotaryPosition;
    }
  
    if (digitalRead(BUTTON_SW_PIN) == LOW) {
      onClick();
      delay(200);
    }
  }

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

  screen.setTextSize(1);
  screen.setTextColor(WHITE);
  screen.display();
}

void setupRotaryButton() {
  pinMode(BUTTON_SW_PIN, INPUT_PULLUP);
}

void setupMoistureSensors() {
  pinMode(MOISTURE_SENSOR_1_PIN, INPUT);
  pinMode(MOISTURE_SENSOR_2_PIN, INPUT);
  pinMode(MOISTURE_SENSOR_3_PIN, INPUT);
  pinMode(MOISTURE_SENSOR_4_PIN, INPUT);
}

void setupPumps() {
  pinMode(PUMP_1_PIN, OUTPUT);
  pinMode(PUMP_2_PIN, OUTPUT);
  pinMode(PUMP_3_PIN, OUTPUT);
  pinMode(PUMP_4_PIN, OUTPUT);
}

void updateScreen() {
  screen.clearDisplay();

  // Header: Power Save Countdown
  long powerSaverCountdown = (DELAY_POWER_SAVER - (millis() - lastActivityTime)) / 1000;
  drawNumber(powerSaverCountdown, 0, 0);
  //drawNumber(rotaryPosition, 0, 0);

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
    case PAGE_PLANT:
      displayPlantPage();
      break;
    case PAGE_WATERING:
      displayWateringPage();
      break;
    case PAGE_INTERVAL:
      displayIntervalPage();
      break;
    case PAGE_MOISTURE_THRESHOLD:
      displayMoistureThreshold();
      break;
  }

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

  // Menu
  uint8_t menuX = 20;
  uint8_t menuY = 10;
  screen.setTextSize(1);
  for (uint8_t index = homePageMenuOffset; index < PLANT_COUNT; index++) {
    screen.setCursor(menuX + 2, menuY + 2 + (index - homePageMenuOffset) * 12);
    screen.print(plants[index].name);
    if (index == homePageMenuIndex) {
      screen.fillRect(menuX, menuY + (index - homePageMenuOffset) * 12, SCREEN_WIDTH - menuX, 12, INVERSE);
    }
  }
}

void displayPlantPage() {
  Plant plant = plants[homePageMenuIndex];
  
  displayPlantTitle(plant);

  // Moisture sensor
  int16_t moistureValue = analogRead(plant.moistureSensorPin);
  int16_t moisturePercent = (1 - ((float) (moistureValue - plant.minMoisture) / (plant.maxMoisture - plant.minMoisture))) * 100;
  if (moisturePercent < 0) {
    moisturePercent = 0;
  }
  if (moisturePercent > 100) {
    moisturePercent = 100;
  }

  // Moisture gauge
  uint8_t startX = 0;
  uint8_t startY = 25;
  uint8_t width = 11;
  uint8_t height = 26;
  uint8_t gaugeHeight = ((float) (height - 2) / 100) * moisturePercent;
  screen.drawFastVLine(startX, startY, height, WHITE);
  screen.drawFastVLine(startX + width - 1, startY, height, WHITE);
  screen.drawFastHLine(startX, startY + height - 1, width, WHITE);
  screen.fillRect(startX + 2, startY + height - gaugeHeight, width - 4, gaugeHeight, WHITE);

  // Moisture percentage
  drawPercentage(moisturePercent, 1, 52);
  drawNumber(moistureValue, 0, 59);

  // Menu
  uint8_t menuX = 20;
  uint8_t menuY = 25;
  screen.setTextSize(1);
  for (uint8_t index = plantPageMenuOffset; index < PLANT_PAGE_MENU_SIZE; index++) {
    screen.setCursor(menuX + 2, menuY + 2 + (index - plantPageMenuOffset) * 12);
    screen.print(plantPageMenu[index]);
    if (index == plantPageMenuIndex) {
      screen.fillRect(menuX, menuY + (index - plantPageMenuOffset) * 12, SCREEN_WIDTH - menuX, 12, INVERSE);
    }
  }
}

void displayWateringPage() {
  Plant plant = plants[homePageMenuIndex];
  
  displayPlantTitle(plant);

  // Message
  screen.setCursor(0, 30);
  screen.setTextSize(1);
  screen.println("Arrosage ...");
  screen.setTextSize(2);
  screen.print(plant.pumpDuration - (millis() - pumpStartTime));
  screen.print(" ms");
}

void displayIntervalPage() {
  Plant plant = plants[homePageMenuIndex];

  displayPlantTitle(plant);

  screen.setCursor(0, 30);
  screen.setTextSize(3);
  screen.print(plant.interval);
  screen.print("h");
}

void displayMoistureThreshold() {
  Plant plant = plants[homePageMenuIndex];

  displayPlantTitle(plant);

  screen.setCursor(0, 30);
  screen.setTextSize(3);
  screen.print(plant.moistureThreshold);
}

void displayPlantTitle(Plant plant) {
  // Title
  screen.setCursor(0, 10);
  screen.setTextSize(1);
  screen.print(plant.name);

  // Separator
  screen.drawFastHLine(0, 20, SCREEN_WIDTH, WHITE);
}

void onNext() {
  wakeUp();

  if (currentPage == PAGE_HOME) {
    homePageMenuIndex++;
    if (homePageMenuIndex > PLANT_COUNT - 1) {
      homePageMenuIndex = 0;
      homePageMenuOffset = 0;
    } else if ((homePageMenuIndex - homePageMenuOffset) >= 4) {
      homePageMenuOffset++;
    }
  } else if (currentPage == PAGE_PLANT) {
    plantPageMenuIndex++;
    if (plantPageMenuIndex > PLANT_PAGE_MENU_SIZE - 1) {
      plantPageMenuIndex = 0;
      plantPageMenuOffset = 0;
    } else if ((plantPageMenuIndex - plantPageMenuOffset) >= 3) {
      plantPageMenuOffset++;
    }
  } else if (currentPage == PAGE_INTERVAL) {
    plants[homePageMenuIndex].interval++;
  } else if (currentPage == PAGE_MOISTURE_THRESHOLD) {
    plants[homePageMenuIndex].moistureThreshold++;
  }
}

void onPrevious() {
  wakeUp();

  if (currentPage == PAGE_HOME) {
    if (homePageMenuIndex == 0) {
      homePageMenuIndex = PLANT_COUNT - 1;
      homePageMenuOffset = PLANT_COUNT - 4;
    } else {
      homePageMenuIndex--;

      if ((homePageMenuIndex - homePageMenuOffset) < 0) {
        homePageMenuOffset--;
      }
    }
  } else if (currentPage == PAGE_PLANT) {
    if (plantPageMenuIndex == 0) {
      plantPageMenuIndex = PLANT_PAGE_MENU_SIZE - 1;
      plantPageMenuOffset = PLANT_PAGE_MENU_SIZE - 3;
    } else {
      plantPageMenuIndex--;

      if ((plantPageMenuIndex - plantPageMenuOffset) < 0) {
        plantPageMenuOffset--;
      }
    }
  } else if (currentPage == PAGE_INTERVAL) {
    plants[homePageMenuIndex].interval--;
  } else if (currentPage == PAGE_MOISTURE_THRESHOLD) {
    plants[homePageMenuIndex].moistureThreshold--;
  }
}

void onClick() {
  wakeUp();

  if (currentPage == PAGE_HOME) {
    currentPage = PAGE_PLANT;
    plantPageMenuIndex = 0;
  } else if (currentPage == PAGE_PLANT) {
    // Back
    if (plantPageMenuIndex == 0) {
      currentPage = PAGE_HOME;
    }

    // Watering
    if (plantPageMenuIndex == 1) {
      currentPage = PAGE_WATERING;
      pumpStartTime = millis();
    }

    // Interval
    if (plantPageMenuIndex == 2) {
      currentPage = PAGE_INTERVAL;
    }

    // Moisture threshold
    if (plantPageMenuIndex == 3) {
      currentPage = PAGE_MOISTURE_THRESHOLD;
    }
  } else if (currentPage == PAGE_INTERVAL) {
    currentPage = PAGE_PLANT;
  } else if (currentPage == PAGE_MOISTURE_THRESHOLD) {
    currentPage = PAGE_PLANT;
  }
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

void drawPercentCharacter(uint8_t x, uint8_t y) {
  screen.drawPixel(x, y, WHITE);
  screen.drawPixel(x + 2, y + 4, WHITE);
  screen.drawFastVLine(x, y + 3, 2, WHITE);
  screen.drawPixel(x + 1, y + 2, WHITE);
  screen.drawFastVLine(x + 2, y, 2, WHITE);
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

void drawPercentage(long value, uint8_t x, uint8_t y) {
  uint8_t digit;
  uint8_t offset = 0;

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

  drawPercentCharacter(x + offset + 5, y);
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
