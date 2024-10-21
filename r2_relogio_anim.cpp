RELOGIO R2
 
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

 
byte name0x2[] = { B11111, B11100, B10011, B00100, B10000, B10001, B10000, B10000 };
byte name0x0[] = { B00000, B00000, B00000, B00001, B00001, B00010, B00010, B00010 };
byte name0x1[] = { B11111, B00111, B11000, B00100, B00000, B10000, B00000, B00000 };
byte name0x3[] = { B00000, B00000, B00000, B10000, B10000, B01000, B01000, B01000 };
byte name1x0[] = { B00011, B00010, B00010, B00001, B00001, B00000, B00000, B00000 };
byte name1x1[] = { B00000, B00000, B10000, B00000, B00100, B11000, B01111, B01000 };
byte name1x2[] = { B11110, B00000, B00001, B00000, B00100, B00011, B11110, B00010 };
byte name1x3[] = { B11000, B01000, B01000, B10000, B10000, B00000, B00000, B00000 };

 
void setup() {
  lcd.begin(16, 2);
  lcd.createChar(0, name0x2);
  lcd.setCursor(2, 0);
  lcd.write(0);
  lcd.createChar(1, name0x0);
  lcd.write(1);
  lcd.createChar(2, name0x1);
  lcd.setCursor(1, 0);
  lcd.write(2);
  lcd.createChar(3, name0x3);
  lcd.setCursor(3, 0);
  lcd.write(3);
  lcd.createChar(4, name1x0);
  lcd.setCursor(0, 1);
  lcd.write(4);
  lcd.createChar(5, name1x1);
  lcd.setCursor(1, 1);
  lcd.write(5);
  lcd.createChar(6, name1x2);
  lcd.setCursor(2, 1);
  lcd.write(6);
  lcd.createChar(7, name1x3);
  lcd.setCursor(3, 1);
  lcd.write(7);
}
 
void loop() {};