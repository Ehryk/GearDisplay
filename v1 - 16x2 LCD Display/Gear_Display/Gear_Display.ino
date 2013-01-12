/*
Manual Transmission Gear Display
Ehryk Menze
*/

#include <LiquidCrystal.h>
#include <EEPROM.h>

#define MEAN_BASED 0
#define TRIMMED_MEAN 1
#define TRIMMED_THEORETICAL 2
#define THEORETICAL 3
#define LOW_BASED 4
#define HIGH_BASED 5

//Set up LCD pins for 4 bit mode
int lcdRS = 12;
int lcdEnable = 11;
int lcdD4 = 5;
int lcdD5 = 4;
int lcdD6 = 3;
int lcdD7 = 2;
//initialize the library with the numbers of the interface pins
LiquidCrystal lcd(lcdRS, lcdEnable, lcdD4, lcdD5, lcdD6, lcdD7);

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

//Configure Pins
int gears = 6;
int gearPin[6] = {0, 1, 2, 3, 4, 5};
int modePin = 8;
int upPin = 9;
int downPin = 10;
int ledPin = 1;

//Declare Value Variables
//Values will range between 0 and 1023, (0V and 5V respectively)
//With a Resolution of 4.9mV / unit
int values[6];
int theoretical = 511; //This is what should be read by the ideal Hall Effect Sensor, in a neutral magnetic environment.
int numberActive = -2; //Number of gears engaged
int method = MEAN_BASED; //Method of determining gear engagement
int mode = 0; //Which display mode is selected
int gear = -2; //Which gear is active, if any. 0 = Neutral.
int baseline = 0; //The baseline from which any gear out of tolerance from is considered engaged
int tolerance; //How much a gear can vary from the baseline before considered engaged
int toleranceInterval = 5; //How much to vary the tolerance on a single press
boolean inGear = false; //Whether or not the vehicle is in a gear
boolean led = true; //Whether or not to light the LED when a gear is engaged

//Debugging
boolean debug = true;
unsigned long debugRefresh = 0;
int refreshInterval = 1000; //How often to refresh the serial port, in milliseconds

//These are addresses in the EEPROM for persistent storage
int eepromSet = 0;
int eepromMode = 1;
int eepromMethod = 2;
int eepromTolerance[2] = {4, 5};
int eepromDebug = 6;
int eepromLED = 7;
int eepromToleranceInterval = 8;
boolean eepromUpdateNeeded = false;
unsigned long eepromLastUpdated = 0;

//Parameters
int baud = 9600;
int defaultTolerance = 200;

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(baud);
  
  //Set pin modes
  for (int g = 0; g < gears; g++) {
    pinMode(g, INPUT);
  }
  pinMode(modePin, INPUT);
  pinMode(upPin, INPUT);
  pinMode(downPin, INPUT);
  
  //Use internal pull-up resistors
  digitalWrite(modePin, HIGH);
  digitalWrite(upPin, HIGH);
  digitalWrite(downPin, HIGH);
  
  //Set up LCD
  lcd.begin(16, 2);
  lcd.createChar(0, sigma);
  lcd.createChar(1, oneColon);
  lcd.createChar(4, fourColon);
  lcd.createChar(2, tColon);
  
  readEEPROM();
   
  writeCredits();
  delay(1500);
  lcd.clear();
}

boolean checkPress(int pin) {
  return digitalRead(pin) == LOW && debounce(pin, LOW, 20, 6);
}

void loop() {
  // put your main code here, to run repeatedly:
  //Check for Mode Press
  if (checkPress(modePin)) {
    mode++;
    if (mode > 12) mode = 0;
    delay(100);
    lcd.clear();
  }
  else if (checkPress(upPin)) {
    if (mode == 7) { //Method Menu
      method++;
      if (method > HIGH_BASED) method = MEAN_BASED;
      eepromUpdateNeeded = true;
    }
    else if (mode == 8) { //LED Menu
      led = !led;
      eepromUpdateNeeded = true;
    }
    else if (mode == 9 && toleranceInterval < 255) { //Tolerance Interval Menu
      toleranceInterval++;
      eepromUpdateNeeded = true;
    }
    else if (tolerance < 1023 - toleranceInterval) {
      tolerance += toleranceInterval;
      eepromUpdateNeeded = true;
    }
  }
  else if (checkPress(downPin) && tolerance > 0) {
    if (mode == 7) { //Method Menu
      method--;
      if (method > MEAN_BASED) method = HIGH_BASED;
      eepromUpdateNeeded = true;
    }
    else if (mode == 8) { //LED Menu
      led = !led;
      eepromUpdateNeeded = true;
    }
    else if (mode == 9 && toleranceInterval > 1) { //Tolerance Interval Menu
      toleranceInterval--;
      eepromUpdateNeeded = true;
    }
    else if (tolerance > toleranceInterval) {
      tolerance -= toleranceInterval;
      eepromUpdateNeeded = true;
    }
  }
  
  //Update the value variables
  readValues();
  baseline = computeBaseline(method);
  numberActive = countActive();
  gear = activeGear();
  inGear = gear > 0;
  
  //This saves all values to EEPROM, if ten seconds have passed since a change has been made
  if (eepromUpdateNeeded && millis() - eepromLastUpdated > 10000) writeEEPROM();
  
  //This rereads in the case of an error or fault
  //(doesn't display errors, leaves last displayed values)
  if (!debug && gear < 0) return;
  
  if (debug && millis() - debugRefresh > refreshInterval) {
    Serial.print("Mode: ");
    Serial.println(mode);
    
    Serial.print("Baseline: ");
    Serial.print(baseline);
    Serial.print(" (");
    Serial.print(formatValue(baseline));
    Serial.print(") ");
    Serial.print(toVoltage(baseline));
    Serial.println("V");
    
    Serial.print("Tolerance: ");
    Serial.print(tolerance);
    Serial.print(" ");
    Serial.print(toVoltage(tolerance));
    Serial.println("V");
    
    Serial.print("In Gear: ** ");
    Serial.print(gearName(gear));
    Serial.print(" (");
    Serial.print(gearChar(gear));
    Serial.println(") **");
    
    if (inGear) {
      Serial.print(" - Active Gear: ");
      Serial.print(activeValue());
      Serial.print(" ");
      Serial.print(activeVoltage());
      Serial.println("V");
      Serial.print(" - Differential: ");
      Serial.println(abs(activeValue() - baseline));
    }
    else {
      Serial.println(" - Active Gear: N/A");
      Serial.println(" - Differential: N/A");
    }
    
    Serial.print("Gear 1: ");
    Serial.print(values[0]);
    Serial.print(" (");
    Serial.print(formatValue(values[0]));
    Serial.print(") ");
    Serial.print(toVoltage(values[0]));
    Serial.println("V");
    
    Serial.print("Gear 2: ");
    Serial.print(values[1]);
    Serial.print(" (");
    Serial.print(formatValue(values[1]));
    Serial.print(") ");
    Serial.print(toVoltage(values[1]));
    Serial.println("V");
    
    Serial.print("Gear 3: ");
    Serial.print(values[2]);
    Serial.print(" (");
    Serial.print(formatValue(values[2]));
    Serial.print(") ");
    Serial.print(toVoltage(values[2]));
    Serial.println("V");
    
    Serial.print("Gear 4: ");
    Serial.print(values[3]);
    Serial.print(" (");
    Serial.print(formatValue(values[3]));
    Serial.print(") ");
    Serial.print(toVoltage(values[3]));
    Serial.println("V");
    
    Serial.print("Gear 5: ");
    Serial.print(values[4]);
    Serial.print(" (");
    Serial.print(formatValue(values[4]));
    Serial.print(") ");
    Serial.print(toVoltage(values[4]));
    Serial.println("V");
    
    Serial.print("Gear R: ");
    Serial.print(values[5]);
    Serial.print(" (");
    Serial.print(formatValue(values[5]));
    Serial.print(") ");
    Serial.print(toVoltage(values[5]));
    Serial.println("V");
    
    Serial.print("Standard Deviation: ");
    Serial.println(getStandardDeviation(baseline));
    
    Serial.println();
    
    debugRefresh = millis();
  }
  
  //Update the display
  updateDisplay(mode, gear);
}

void readValues(){
  for (int g = 0; g < gears; g++) {
    values[g] = readHallEffect(gearPin[g]);
  }
}

int computeBaseline(int m) {
  if (m == MEAN_BASED) return getMean();
  else if (m == TRIMMED_MEAN) return getTrimmed(false);
  else if (m == TRIMMED_THEORETICAL) return getTrimmed(true);
  else if (m == THEORETICAL) return theoretical;
  else if (m == LOW_BASED) return 0;
  else if (m == HIGH_BASED) return 1023;
  
  return -1;
}

int getMean() {
  int total = 0;
  for (int g = 0; g < gears; g++) {
    total += values[g];
  }
  return total / gears;
}

int getTrimmed(boolean theoretical) {
  int subline = 0;
  if (theoretical) subline = theoretical;
  else subline = getMean();
  
  //Find which gear gets trimmed
  //(Furthest off the subline)
  int maximum = 0;
  int maximumValue = abs(values[0] - subline);
  for (int g = 0; g < gears; g++) {
    int differential = abs(values[g] - subline);
    if (differential > maximum) {
      maximumValue = differential;
      maximum = g;
    }
  }
  
  int trimmedTotal = 0;
  for (int g = 0; g < gears; g++) {
    if (g != maximum) trimmedTotal += values[g];
  }
  
  return trimmedTotal / (gears - 1);
}

float getStandardDeviation(int b) {
  float variance = 0;
  for (int g = 0; g < gears; g++) {
    variance += pow(toVoltage(values[g] - b), 2);
  }
  variance = variance / gears;
  return sqrt(variance);
}

float toVoltage(int value) {
  return value * 5.0 / 1023.0;
}

int readHallEffect(int pin) {
  return analogRead(pin);
}

int countActive() {
  //Return the number of pins avove tolerance
  int count = 0;
  
  for (int g = 0; g < gears; g++) {
    if (active(values[g])) count++;
  }
  
  return count;
}

int activePin() {
  //Return the pin above tolerance
  //Return -1 if more than one active
  
  if (numberActive == 0) return 0;
  if (numberActive >= 2) return -1;
  
  for (int g = 0; g < gears; g++) {
    if (active(values[g])) return gearPin[g];
  }
  
  //Houston, we have a problem
  return -2;
}

int activeGear() {
  //Return the gear number above tolerance
  //Return -1 if more than one active
  
  if (numberActive == 0) return 0;
  if (numberActive >= 2) return -1;
  
  for (int g = 0; g < gears; g++) {
    if (active(values[g])) return g + 1;
  }
  
  //Houston, we have a problem
  return -2;
}

int activeValue() {
  //Return the value above tolerance
  //Return -1 if more than one active
  
  if (numberActive == 0) return 0;
  if (numberActive >= 2) return -1;
  
  for (int g = 0; g < gears; g++) {
    if (active(values[g])) return values[g];
  }
  
  //Houston, we have a problem
  return -2;
}

float activeVoltage() {
  return toVoltage(activeValue());
}

boolean active(int v) {
  int deviation = abs(v - baseline);
  return deviation > tolerance;
}

//Returns the Name of a Gear
char* gearName(int g) {
  switch (g) {
    case -2: return "Fault  ";
    case -1: return "Error  ";
    case 0:  return "Neutral";
    case 1:  return "First  ";
    case 2:  return "Second ";
    case 3:  return "Third  ";
    case 4:  return "Fourth ";
    case 5:  return "Fifth  ";
    case 6:  return "Reverse";
  }
  return "Fault? ";
}

//Returns the Letter/Digit of a Gear
char gearChar(int g) {
  switch (g) {
    case -2: return 'F';
    case -1: return 'E';
    case 0:  return 'N';
    case 1:  return '1';
    case 2:  return '2';
    case 3:  return '3';
    case 4:  return '4';
    case 5:  return '5';
    case 6:  return 'R';
  }
  return '?';
}

void updateDisplay(int mode, int gear) {
  switch(mode) {
    case 0: writeBasic(gear); break;
    case 1: writeAdvanced(gear); break;
    case 2: writeVariables(gear, true); break;
    case 3: writeVariables(gear, false); break;
    case 4: writeValues(); break;
    case 5: writeVoltages(); break;
    case 6: writeFill(gearChar(gear)); break;
    case 7: writeMenuMethod(); break;
    case 8: writeMenuLED(); break;
    case 9: writeMenuToleranceInterval(); break;
    case 10: writeMenuDebug(); break;
    case 11: writeCredits(); break;
    case 12: break;
  }
}

void writeBasic(int g) {
  //Print Gear Char
  lcd.setCursor(1, 0);
  lcd.print('.');
  lcd.print(gearChar(g));
  lcd.print('.');
  
  lcd.setCursor(9, 0);
  lcd.print(" Gear");
  
  //Print Gear Name
  lcd.setCursor(0, 1);
  lcd.print(gearName(g));
  
  lcd.setCursor(9, 1);
  lcd.print("Display");
}

void writeAdvanced(int g) {
  //Print Gear Char
  lcd.setCursor(2, 0);
  lcd.print(gearChar(g));
  
  //Print Gear Voltage
  lcd.setCursor(9, 0);
  if (inGear) {
    lcd.print("G:");
    lcd.print(activeVoltage());
    lcd.print("V");
  }
  else lcd.print("       ");
  
  //Print Gear Name
  lcd.setCursor(0, 1);
  lcd.print(gearName(g));
  
  //Print Average / Difference
  lcd.setCursor(9, 1);
  if (inGear) {
    lcd.print("D:");
    lcd.print(toVoltage(abs(activeValue() - baseline)));
    lcd.print("V");
  }
  else {
    if (method == MEAN_BASED) lcd.print("A:");
    else lcd.print("B:");
    lcd.print(toVoltage(baseline));
    lcd.print("V");
  }
}

void writeVariables(int g, boolean voltage) {
  //Print Gear Char
  lcd.setCursor(0, 0);
  lcd.print(gearChar(g));
  
  //Print Gear Name
  lcd.setCursor(2, 0);
  lcd.print(gearName(g));
  
  //Print Standard Deviation
  lcd.setCursor(10, 0);
  lcd.write((byte)0);
  lcd.print(":");
  lcd.print(getStandardDeviation(baseline), 2);
  
  //Print Tolerance
  lcd.setCursor(0, 1);
  if (voltage) {
    lcd.write((byte)2);
    lcd.print(toVoltage(tolerance), 2);
    lcd.print("V");
  }
  else {
    lcd.print("T:");
    lcd.print(padLeft(tolerance, 3));
  }
  
  //Print Debug
  lcd.setCursor(7, 1);
  if (debug) lcd.print("D");
  
  //Print Baseline Voltage
  lcd.setCursor(9, 1);
  if (method == MEAN_BASED) lcd.print("A:");
  else lcd.print("B:");
  lcd.print(toVoltage(baseline), 2);
  lcd.print("V");
}

void writeValues() {
  lcd.setCursor(0, 0);
  lcd.write((byte)1);
  lcd.print(formatValue(values[0]));
  lcd.print(" ");
  
  lcd.print("2:");
  lcd.print(formatValue(values[1]));
  lcd.print(" ");
  
  lcd.print("3:");
  lcd.print(formatValue(values[2]));
  
  lcd.setCursor(0, 1);
  lcd.write((byte)4);
  lcd.print(formatValue(values[3]));
  lcd.print(" ");
  
  lcd.print("5:");
  lcd.print(formatValue(values[4]));
  lcd.print(" ");
  
  lcd.print("R:");
  lcd.print(formatValue(values[5]));
}

void writeVoltages() {
  lcd.setCursor(0, 0);
  lcd.print(toVoltage(values[0]), 2);
  lcd.print("V ");
  
  lcd.print(toVoltage(values[1]), 2);
  lcd.print("V ");
  
  lcd.print(toVoltage(values[2]), 2);
  
  lcd.setCursor(0, 1);
  lcd.print(toVoltage(values[3]), 2);
  lcd.print("V ");
  
  lcd.print(toVoltage(values[4]), 2);
  lcd.print("V ");
  
  lcd.print(toVoltage(values[5]), 2);
}

void writeFill(char c) {
  lcd.setCursor(0, 0);
  for (int column = 0; column < 16; column ++) {
    lcd.print(c);
  }
  lcd.setCursor(0, 1);
  for (int column = 0; column < 16; column ++) {
    lcd.print(c);
  }
}

void writeMenuMethod() {
  lcd.setCursor(0, 0);
  lcd.print("Method Used:");
  
  lcd.setCursor(0, 1);
  switch(method) {
    case MEAN_BASED: lcd.print("Mean Based");
    case TRIMMED_MEAN: lcd.print("Trimmed - Mean");
    case TRIMMED_THEORETICAL: lcd.print("Trimmed - Theory");
    case THEORETICAL: lcd.print("Theoretical");
    case LOW_BASED: lcd.print("Low (0VDC)");
    case HIGH_BASED: lcd.print("High (+5VDC");
  }
}

void writeMenuLED() {
  lcd.setCursor(0, 0);
  lcd.print("In Gear LED:");
  
  lcd.setCursor(0, 1);
  if (led) lcd.print("ON");
  else lcd.print("OFF");
}

void writeMenuToleranceInterval() {
  lcd.setCursor(0, 0);
  lcd.print("T Interval:");
  
  lcd.setCursor(0, 1);
  lcd.print(toleranceInterval);
}

void writeMenuDebug() {
  lcd.setCursor(0, 0);
  lcd.print("Debug Mode:");
  
  lcd.setCursor(0, 1);
  if (debug) lcd.print("ON");
  else lcd.print("OFF");
}

void writeCredits() {
  lcd.setCursor(0, 0);
  lcd.print("- Gear Display -");
  
  lcd.setCursor(0, 1);
  lcd.print("By Eric C Menze.");
}

boolean debounce(int pin, int value, int timeDelay, int bounceCount) {
  int toCheck = bounceCount;
  while (toCheck > 0) {
    delay(timeDelay);
    if (digitalRead(pin) != value) return false;
    toCheck --;
  }
  return true;
}

String formatValue(int v) {
  if (v < 1000) return padLeft(v, 3);
  String n = String("+");
  n += String(padLeft(v % 1000, 2));
  return n;
}

String padLeft(int number, int padding) {
  String n = String(number);
  int currentMax = 10;
  for (int i=1; i<padding; i++){
    if (number < currentMax) {
      n = "0" + n;
    }
    currentMax *= 10;
  }
  return n;
}

boolean readEEPROM() {
  if (EEPROM.read(eepromSet) != 42) return false; //EEPROM not set yet
  mode = EEPROM.read(eepromMode);
  method = EEPROM.read(eepromMethod);
  tolerance = EEPROM.read(eepromTolerance[0])*256 + EEPROM.read(eepromTolerance[1]);
  debug = EEPROM.read(eepromDebug);
  led = EEPROM.read(eepromLED);
  toleranceInterval = EEPROM.read(eepromToleranceInterval);
}

void writeEEPROM() {
  EEPROM.write(eepromSet, 42); //EEPROM marked as initialized
  EEPROM.write(eepromMode, mode);
  EEPROM.write(eepromMethod, method);
  EEPROM.write(eepromTolerance[0], tolerance/256);
  EEPROM.write(eepromTolerance[1], tolerance%256);
  EEPROM.write(eepromDebug, debug);
  EEPROM.write(eepromLED, led);
  EEPROM.write(eepromToleranceInterval, toleranceInterval);
  
  eepromLastUpdated = millis();
  eepromUpdateNeeded = false; 
}
