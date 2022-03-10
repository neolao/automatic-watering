#include <Encoder.h> // http://www.pjrc.com/teensy/td_libs_Encoder.html
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BUTTON_CLK_PIN 2
#define BUTTON_DT_PIN 3
#define BUTTON_SW_PIN 4

Adafruit_SSD1306 screen(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Encoder rotaryButton(BUTTON_CLK_PIN, BUTTON_DT_PIN);
long rotaryButtonValue = 0;

void setup() {
  Serial.begin(9600);
  Serial.println(F("Initialize System"));
  
  setupScreen();
  setupRotaryButton();
}

void loop() {
  long newPosition = rotaryButton.read();
  if (newPosition != rotaryButtonValue) {
    rotaryButtonValue = newPosition;
  }
  
  
  if (digitalRead(BUTTON_SW_PIN) == LOW) {
    rotaryButtonValue = 0;
  }

  
  rotaryButtonValue = (rotaryButtonValue / 4) % 20;


  screen.clearDisplay();
  screen.setCursor(0, 5);
  screen.println(rotaryButtonValue);
  /*
  if (swState == LOW) {
    screen.println("Pressed");
  } else {
    screen.println("Released");
  }
  */
  screen.display();
  
}

void setupScreen() {
  if (!screen.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    //Serial.println("Screen allocation failed");
    for (;;);
  }
  //delay(2000);
  screen.clearDisplay();
  screen.setTextWrap(false);
  
  screen.setTextSize(2);
  screen.setTextColor(WHITE);
  screen.print("Init");
  screen.display();
  delay(500);
}

void setupRotaryButton() {
  pinMode(BUTTON_SW_PIN, INPUT_PULLUP);
}
