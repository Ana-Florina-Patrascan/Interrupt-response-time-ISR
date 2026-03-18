
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int buttonPin = 2;
const int ledPin = 8;

volatile bool triggered = false;
volatile unsigned long lastInterruptTime = 0;

unsigned long t0 = 0, t1 = 0, delta = 0;
unsigned int countMode1 = 0;
unsigned int countMode2 = 0;
byte currentMode = 0;

bool blinkLed = false;
unsigned long lastBlinkTime = 0;
bool ledState = LOW;

bool toggleState = false;  // modificat: doar 2 stări (clipire / oprit)

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Trimite: a / b");

  Serial.println("Trimite 'a' pt ISR, 'b' pt attachInterrupt()");
}

void handlerMode1() {
  if (currentMode == 1) {
    unsigned long now = millis();
    if (now - lastInterruptTime > 300) {
      t0 = micros();
      triggered = true;
      countMode1++;
      lastInterruptTime = now;
    }
  }
}

void handlerMode2() {
  if (currentMode == 2) {
    unsigned long now = millis();
    if (now - lastInterruptTime > 300) {
      t0 = micros();
      triggered = true;
      countMode2++;
      lastInterruptTime = now;

      toggleState = !toggleState;

      if (toggleState) {
        blinkLed = true;  // LED clipește
      } else {
        blinkLed = false; // LED stins complet
        digitalWrite(ledPin, LOW);
      }
    }
  }
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();

    if (cmd == 'a') {
      detachInterrupt(digitalPinToInterrupt(buttonPin));
      attachInterrupt(digitalPinToInterrupt(buttonPin), handlerMode1, RISING);
      currentMode = 1;
      blinkLed = false;
      toggleState = false;
      digitalWrite(ledPin, LOW);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Modul 1: ISR");
      lcd.setCursor(0, 1);
      lcd.print("Activat");
      //delay(1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Astept apasare");

      Serial.println("Modul 1 activat: ISR");
    } else if (cmd == 'b') {
      detachInterrupt(digitalPinToInterrupt(buttonPin));
      attachInterrupt(digitalPinToInterrupt(buttonPin), handlerMode2, RISING);
      currentMode = 2;
      blinkLed = false;
      toggleState = false;
      digitalWrite(ledPin, LOW);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Modul 2: attach");
      lcd.setCursor(0, 1);
      lcd.print("Activat");
     // delay(1000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Astept apasare");

      Serial.println("Modul 2 activat: attachInterrupt()");
    }
  }

  if (triggered) {
    t1 = micros();
    delta = t1 - t0;

    if (currentMode == 1) {
      digitalWrite(ledPin, !digitalRead(ledPin));  // TOGGLE în mod 1
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Delay: ");
    lcd.print(delta);
    lcd.print("us");

    lcd.setCursor(0, 1);
    lcd.print("Mod ");
    lcd.print(currentMode);
    lcd.print(": ");
    lcd.print(currentMode == 1 ? countMode1 : countMode2);

    Serial.print("Delay: ");
    Serial.print(delta);
    Serial.print(" us | Mod: ");
    Serial.print(currentMode);
    Serial.print(" | Total: ");
    Serial.println(currentMode == 1 ? countMode1 : countMode2);

    //delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Astept apasare");

    triggered = false;
  }

  // LED clipitor (în mod 2, dacă activat)
  if (currentMode == 2 && blinkLed) {
    if (millis() - lastBlinkTime > 500) {
      lastBlinkTime = millis();
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
    }
  }
}
