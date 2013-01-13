/*
Configures and handles the LCD Display
*/

#define SIGMA 0
#define ONE_COLON 1
#define T_COLON 2
#define FOUR_COLON 4

void setLCDBrightness(int v) {
  analogWrite(lcdBrightness, v);
}

//Create Custom LCD Characters
void createCustomCharacters() {
  byte sigma[8] = {
    B00000,
    B00000,
    B00000,
    B01111,
    B10010,
    B10010,
    B01100,
  };
  byte oneColon[8] = {
    B01000,
    B11011,
    B01011,
    B01000,
    B01011,
    B01011,
    B11100,
  };
  byte fourColon[8] = {
    B00100,
    B01101,
    B10101,
    B11110,
    B00101,
    B00101,
    B00100,
  };
  byte tColon[8] = {
    B11100,
    B01011,
    B01011,
    B01000,
    B01011,
    B01011,
    B01000,
  };

  lcd.createChar(SIGMA, sigma);
  lcd.createChar(ONE_COLON, oneColon);
  lcd.createChar(FOUR_COLON, fourColon);
  lcd.createChar(T_COLON, tColon);
}
