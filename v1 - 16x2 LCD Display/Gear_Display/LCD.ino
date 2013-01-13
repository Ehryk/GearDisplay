/*
Configures and handles the LCD Display
*/

//Create Custom LCD Characters
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

void createCustomCharacters() {
  lcd.createChar(0, sigma);
  lcd.createChar(1, oneColon);
  lcd.createChar(4, fourColon);
  lcd.createChar(2, tColon);
}
