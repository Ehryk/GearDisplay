/*
Manual Transmission Gear Display
Ehryk Menze
*/

#include <LiquidCrystal.h>
#include <EEPROM.h>

#define METHOD_BEGIN 0
#define MEAN_BASED 0
#define TRIM1_MEAN 1
#define TRIM1_THEORETICAL 2
#define TRIM1_HIGHEST 3
#define TRIM1_LOWEST 4
#define TRIM2_MEAN 5
#define TRIM2_THEORETICAL 6
#define TRIM2_HIGHEST 7
#define TRIM2_LOWEST 8
#define THEORETICAL 9
#define LOW_BASED 10
#define HIGH_BASED 11
#define METHOD_END 11

#define LED_BEGIN 0
#define LED_OFF 0
#define LED_IN_GEAR 1
#define LED_NEUTRAL 2
#define LED_END 2

#define MODE_BEGIN 0
#define MODE_BASIC 0
#define MODE_BASIC_S 1
#define MODE_ADVANCED 2
#define MODE_ADVANCED_S 3
#define MODE_VARIABLES_V 4
#define MODE_VARIABLES 5
#define MODE_VOLTAGES 6
#define MODE_VALUES 7
#define MODE_FILL 8
#define MODE_MENU 9
#define MODE_LOG 10
#define MODE_CREDITS 11
#define MODE_DISPLAY_OFF 12
#define MODE_END 12

#define MENU_BEGIN 0
#define MENU_METHOD 0
#define MENU_LED 1
#define MENU_BRIGHTNESS 2
#define MENU_TOLERANCE_INTERVAL 3
#define MENU_LOGGING 4
#define MENU_DEBUG 5
#define MENU_VOLTAGES 6
#define MENU_VALUES 7
#define MENU_EXIT 8
#define MENU_END 8

#define LOG_BEGIN 0
#define LOG_HIGHWAY_CITY 0
#define LOG_HIGHWAY_CITY_T 1
#define LOG_HIGHWAY_CITY_PERCENT 2
#define LOG_HIGHWAY_CITY_PERCENT_T 3
#define LOG_UP_TIME 4
#define LOG_SHIFT_COUNT 5
#define LOG_SHIFT_COUNT_T 6
#define LOG_NEUTRAL 7
#define LOG_NEUTRAL_T 8
#define LOG_NEUTRAL_PERCENT 9
#define LOG_NEUTRAL_PERCENT_T 10
#define LOG_GEAR_TIME 11
#define LOG_GEAR_TIME_T 12
#define LOG_GEAR_PERCENT 13
#define LOG_GEAR_PERCENT_T 14
#define LOG_EXIT 15
#define LOG_END 15

//Set up LCD pins for 4 bit mode
int lcdRS = 9;
int lcdEnable = 8;
int lcdD4 = 7;
int lcdD5 = 6;
int lcdD6 = 5;
int lcdD7 = 4;
int lcdBrightness = 3;
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
int gearPin[6] = {0, 1, 2, 3, 4, 5}; //Analog Input Pins
//Digital Input Pins
int ledPin = 2;
int modePin = 10;
int upPin = 11;
int downPin = 12;

//Declare Value Variables
//Values will range between 0 and 1023, (0V and 5V respectively)
//With a Resolution of 4.9mV / unit
int values[6];
int theoretical = 511; //This is what should be read by the ideal Hall Effect Sensor, in a neutral magnetic environment.
int numberActive = -2; //Number of gears engaged
int method = MEAN_BASED; //Method of determining gear engagement
int mode = 0; //Which display mode is selected
boolean saveMode = true; //Persists mode across power cycles
boolean inMenu = false; //Whether the user is in the menu system
int menuMode = 0; //Which menu mode is selected
boolean inLog = false; //Whether the user is in the log system
int logMode = 0; //Which log mode is selected
int brightness = 255; //LCD Brightness
int brightnessInterval = 15; //LCD Brightness Increment to increase/decrease by
char separator = '-'; //Surrounds gear char
int gear = -2; //Which gear is active, if any. 0 = Neutral.
int baseline = 0; //The baseline from which any gear out of tolerance from is considered engaged
int tolerance; //How much a gear can vary from the baseline before considered engaged
int toleranceInterval = 5; //How much to vary the tolerance on a single press
boolean inGear = false; //Whether or not the vehicle is in a gear
int led = LED_IN_GEAR; //Whether or not to light the LED when a gear is engaged, neutral, or off
int baud = 9600; //Used for Serial communication
int defaultTolerance = 200; //Initial Tolerance
unsigned long lastLoopStart = 0;

//Debugging
boolean debug = true;
unsigned long debugRefresh = 0;
int debugInterval = 1000; //How often to refresh the serial port, in milliseconds

//These are addresses in the EEPROM for persistent storage
boolean enableEEPROM = true; //Uses EEPROM
boolean clearEEPROM = true; // -- SET TO TRUE ONLY WHEN ADDRESSES CHANGE, or to reset EEPROM to defaults --
int eepromSet = 0;
int eepromMode = 1;
int eepromMethod = 2;
int eepromSaveMode = 3;
int eepromTolerance = 4;
int eepromLED = 6;
int eepromEnableLog = 7;
int eepromToleranceInterval = 8;
int eepromDebug = 9;
int eepromBrightness = 10;
boolean eepromUpdateNeeded = false;
unsigned long eepromLastUpdated = 0;
int eepromInterval = 10000; //How often to wait after a change to write to EEPROM

//Logging
boolean enableLog = true;
boolean clearLog = false; // -- SET TO TRUE ONLY WHEN ADDRESSES CHANGE, or to reset persistent log  --
//Log Times in tenths of a second
unsigned long timeInGear[1 + 6]; //0 for Neutral, 1-R
unsigned long upTime = 0; //Time the unit has been on, persisting between power cycles
unsigned int shiftsToGear[6]; //Transitions from Neutral to a gear, 1-R
unsigned long lifeTimeInGear[1 + 6]; //Life Time persists across power cycles
unsigned long lifeUpTime = 0; //Life Time persists across power cycles
unsigned int lifeShiftsToGear[6]; //Life Shifts persists across power cycles
unsigned long logRefreshed = 0; //Time since log values last saved to EEPROM
int logInterval = 30000; //How often to refresh the serial port, in milliseconds
//-- EEPROM LOG ADDRESSES BASED ON START ADDRESS --
//Move the start address if EEPROM gets worn at the given start address
int eepromLogStartAddress = 128;
int eepromLogGearTime[1 + 6] = { eepromLogStartAddress, eepromLogStartAddress + 4, eepromLogStartAddress + 8, eepromLogStartAddress + 12, eepromLogStartAddress + 16, eepromLogStartAddress + 20, eepromLogStartAddress + 24};
int eepromLogUpTime = eepromLogStartAddress + 28;
int eepromLogShiftCount[6] = {eepromLogStartAddress + 32, eepromLogStartAddress + 34, eepromLogStartAddress + 36, eepromLogStartAddress + 38, eepromLogStartAddress + 40, eepromLogStartAddress + 42};

//setup() gets called once at power on/reset of Arduino
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(baud);
  
  //Set pin modes
  for (int g = 0; g < gears; g++) {
    pinMode(g, INPUT);
  }
  pinMode(ledPin, OUTPUT);
  pinMode(modePin, INPUT);
  pinMode(upPin, INPUT);
  pinMode(downPin, INPUT);
  pinMode(lcdBrightness, OUTPUT);
  
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
  
  tolerance = defaultTolerance;
  
  if (enableLog) {
    timeInGear[0] = 0;
    for (int g = 0; g < gears; g++) {
      timeInGear[g + 1] = 0;
      shiftsToGear[g] = 0;
    }
  }
  
  if (clearEEPROM) EEPROM.write(0, 0);
  if (enableEEPROM) readEEPROM();
  if (enableLog && !clearLog) readLog();
  
  writeCredits();
  delay(1500);
  lcd.clear();
  
  lastLoopStart = millis();
}

//loop() gets called repeatedly for the duration of power
void loop() {
  //Loop Timing
  unsigned long loopTime = millis() - lastLoopStart;
  if (millis() < lastLoopStart) loopTime = 0; //millis() overflow, ~70 days 
  lastLoopStart = millis();
  
  handleButtons();
  
  //Update the value variables
  readValues();
  baseline = computeBaseline(method);
  numberActive = countActive();
  int previousGear = gear;
  gear = activeGear();
  inGear = gear > 0;
  
  //Handle LED
  if (led == LED_IN_GEAR && inGear) digitalWrite(ledPin, HIGH);
  else if (led == LED_NEUTRAL && !inGear) digitalWrite(ledPin, HIGH);
  else digitalWrite(ledPin, LOW);
  //Handle LCD Brightness
  digitalWrite(lcdBrightness, brightness);
  
  //This saves all values to EEPROM, if the eeprom interval (in ms) has passed AND change has been made
  if (enableEEPROM && eepromUpdateNeeded && millis() - eepromLastUpdated > eepromInterval) writeEEPROM();
  
  if (enableLog) {
    if (previousGear == gear) { //Remained in a gear (or neutral)
      timeInGear[gear] += loopTime / 100; //Store in tenths
      lifeTimeInGear[gear] += loopTime / 100;
    }
    upTime += loopTime / 100; //Loop success, increment uptime
    lifeUpTime += loopTime / 100;
    if (previousGear == 0 && inGear) { //Shifted into a gear from Neutral
      shiftsToGear[gear - 1]++;
      lifeShiftsToGear[gear - 1]++;
    }
  }
  
  //This writes the Log values, if the log interval (in ms) has passed
  if (enableLog && millis() - logRefreshed > logInterval) writeLog();
  
  //This rereads in the case of an error or fault
  //(doesn't display errors, leaves last displayed values)
  if (!debug && gear < 0) return;
  
  if (debug && millis() - debugRefresh > debugInterval) writeDebug(loopTime);
  
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
  else if (m == TRIM1_MEAN) return getTrimmed(getMean(), 1);
  else if (m == TRIM1_THEORETICAL) return getTrimmed(theoretical, 1);
  else if (m == TRIM1_HIGHEST) return getTrimmed(0, 1);
  else if (m == TRIM1_LOWEST) return getTrimmed(1023, 1);
  else if (m == TRIM2_MEAN) return getTrimmed(getMean(), 2);
  else if (m == TRIM2_THEORETICAL) return getTrimmed(theoretical, 2);
  else if (m == TRIM2_HIGHEST) return getTrimmed(0, 2);
  else if (m == TRIM2_LOWEST) return getTrimmed(1023, 2);
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

int getTrimmed(int subline, int toTrim) {
  if (toTrim >= gears) return subline;
  
  //Find which gear gets trimmed
  //(Furthest off the subline)
  boolean trim[gears];
  for(int init = 0; init < gears; init++) trim[init] = false;
  for (int i = 0; i < toTrim; i++) {
    int maximum = 0;
    int maximumValue = 0;
    for (int g = 0; g < gears; g++) {
      int differential = abs(values[g] - subline);
      if (!trim[g] && differential > maximumValue) {
        maximumValue = differential;
        maximum = g;
      }
    }
    trim[maximum] = true;
  }
  
  int trimmedTotal = 0;
  for (int g = 0; g < gears; g++) {
    if (!trim[g]) trimmedTotal += values[g];
  }
  
  return trimmedTotal / (gears - toTrim);
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
    case MODE_BASIC: writeBasic(gear, ' '); break;
    case MODE_BASIC_S: writeBasic(gear, separator); break;
    case MODE_ADVANCED: writeAdvanced(gear, ' '); break;
    case MODE_ADVANCED_S: writeAdvanced(gear, separator); break;
    case MODE_VARIABLES_V: writeVariables(gear, true); break;
    case MODE_VARIABLES: writeVariables(gear, false); break;
    case MODE_VOLTAGES: writeVoltages(); break;
    case MODE_VALUES: writeValues(); break;
    case MODE_FILL: writeFill(gearChar(gear)); break;
    case MODE_MENU: writeMenu(menuMode); break;
    case MODE_LOG: writeLog(logMode); break;
    case MODE_CREDITS: writeCredits(); break;
    case MODE_DISPLAY_OFF: break;
  }
}

void writeMenu(int menuScreen) {
  if (!inMenu) writePrompt("ENTER MENU:");
  else {
    switch (menuScreen) {
      case MENU_METHOD: writeMenuMethod(); break;
      case MENU_LED: writeMenuLED(); break;
      case MENU_BRIGHTNESS: writeMenuBrightness(); break;
      case MENU_TOLERANCE_INTERVAL: writeMenuToleranceInterval(); break;
      case MENU_LOGGING: writeMenuLogging(); break;
      case MENU_DEBUG: writeMenuDebug(); break;
      case MENU_VOLTAGES: writeVoltages(); break;
      case MENU_VALUES: writeValues(); break;
      case MENU_EXIT: writePrompt("EXIT MENU:"); break;
    }
  }
}

void writeLog(int logScreen) {
  if (!inLog) writePrompt("ENTER LOG:");
  else {
    switch (logScreen) {
      case LOG_HIGHWAY_CITY: writeLogHwyCity(600, 'm', false); break;
      case LOG_HIGHWAY_CITY_T: writeLogHwyCity(36000, 'h', true); break;
      case LOG_HIGHWAY_CITY_PERCENT: writeLogHwyCity(-1, '%', false); break;
      case LOG_HIGHWAY_CITY_PERCENT_T: writeLogHwyCity(-1, '%', true); break;
      case LOG_UP_TIME: writeLogUpTime(36000, 'h'); break;
      case LOG_SHIFT_COUNT: writeLogShifts(false); break;
      case LOG_SHIFT_COUNT_T: writeLogShifts(true); break;
      case LOG_NEUTRAL: writeLogNeutral(600, 'm', false); break;
      case LOG_NEUTRAL_T: writeLogNeutral(36000, 'h', true); break;
      case LOG_NEUTRAL_PERCENT: writeLogNeutral(-1, '%', false); break;
      case LOG_NEUTRAL_PERCENT_T: writeLogNeutral(-1, '%', true); break;
      case LOG_GEAR_TIME: writeLogGearTime(600, 'm', false); break;
      case LOG_GEAR_TIME_T: writeLogGearTime(36000, 'h', true); break;
      case LOG_GEAR_PERCENT: writeLogGearPercent(false); break;
      case LOG_GEAR_PERCENT_T: writeLogGearPercent(true); break;
      case LOG_EXIT: writePrompt("EXIT LOG:"); break;
    }
  }
}

void writePrompt(char* title) {
  lcd.setCursor(0, 0);
  lcd.print(title);
  
  lcd.setCursor(0, 1);
  lcd.print("Press + or -");
}

void writeBasic(int g, char s) {
  //Print Gear Char
  lcd.setCursor(1, 0);
  lcd.print(s);
  lcd.print(gearChar(g));
  lcd.print(s);
  
  lcd.setCursor(9, 0);
  lcd.print(" Gear");
  
  //Print Gear Name
  lcd.setCursor(0, 1);
  lcd.print(gearName(g));
  
  lcd.setCursor(9, 1);
  lcd.print("Display");
}

void writeAdvanced(int g, char s) {
  //Print Gear Char
  lcd.setCursor(1, 0);
  lcd.print(s);
  lcd.print(gearChar(g));
  lcd.print(s);
  
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
    case MEAN_BASED:          lcd.print("Mean Based      "); break;
    case TRIM1_MEAN:          lcd.print("Trim 1 - Mean   "); break;
    case TRIM1_THEORETICAL:   lcd.print("Trim 1 - Theory"); break;
    case TRIM1_HIGHEST:       lcd.print("Trim 1 - Highest"); break;
    case TRIM1_LOWEST:        lcd.print("Trim 1 - Lowest "); break;
    case TRIM2_MEAN:          lcd.print("Trim 2 - Mean   "); break;
    case TRIM2_THEORETICAL:   lcd.print("Trim 2 - Theory "); break;
    case TRIM2_HIGHEST:       lcd.print("Trim 2 - Highest"); break;
    case TRIM2_LOWEST:        lcd.print("Trim 2 - Lowest "); break;
    case THEORETICAL:         lcd.print("Theoretical     "); break;
    case LOW_BASED:           lcd.print("Low (0VDC)      "); break;
    case HIGH_BASED:          lcd.print("High (+5VDC)    "); break;
  }
}

void writeMenuLogging() {
  lcd.setCursor(0, 0);
  lcd.print("Logging:");
  
  lcd.setCursor(0, 1);
  if (enableLog) lcd.print("ON ");
  else lcd.print("OFF");
}

void writeMenuLED() {
  lcd.setCursor(0, 0);
  lcd.print("LED:");
  
  lcd.setCursor(0, 1);
  if (led == LED_IN_GEAR) lcd.print("ON - In Gear");
  else if (led == LED_NEUTRAL) lcd.print("ON - Neutral");
  else lcd.print("OFF         ");
}

void writeMenuBrightness() {
  lcd.setCursor(0, 0);
  lcd.print("LCD Brightness:");
  
  lcd.setCursor(0, 1);
  lcd.print("V:");
  lcd.print(brightness);
  
  lcd.setCursor(8, 1);
  lcd.print("(");
  lcd.print(brightness/255 * 100);
  lcd.print("%)");
}

void writeMenuToleranceInterval() {
  lcd.setCursor(0, 0);
  lcd.print("Toler. Interval:");
  
  lcd.setCursor(0, 1);
  lcd.print(padLeft(toleranceInterval, 3));
  
  lcd.setCursor(5, 1);
  lcd.print("(");
  lcd.print(toVoltage(toleranceInterval), 2);
  lcd.print("V)");
}

void writeMenuDebug() {
  lcd.setCursor(0, 0);
  lcd.print("Debug Mode:");
  
  lcd.setCursor(0, 1);
  if (debug) lcd.print("ON ");
  else lcd.print("OFF");
}

void writeLogHwyCity(int divisor, char unit, boolean lifeTime) {
  //Highway = In Fifth Gear
  //City = In any other gear but Fifth
  float city = 0;
  float highway = 0;
  if (lifeTime) {
    for (int g = 1; g < gears + 1; g++) {
      if (g != 5) city += lifeTimeInGear[g];
    }
    highway = lifeTimeInGear[5];
  }
  else {
    for (int g = 1; g < gears + 1; g++) {
      if (g != 5) city += timeInGear[g];
    }
    highway = timeInGear[5];
  }
  
  lcd.setCursor(0, 0);
  if (lifeTime) lcd.print("T ");
  lcd.print("City:   ");
  if (divisor > 0) lcd.print(city / divisor, 2);
  else lcd.print(city / (city + highway) * 100, 1);
  lcd.print(unit);
  
  lcd.setCursor(0, 1);
  if (lifeTime) lcd.print("T ");
  lcd.print("Highway:");
  if (divisor > 0) lcd.print(highway / divisor, 2);
  else lcd.print(highway / (city + highway) * 100, 1);
  lcd.print(unit);
}

void writeLogUpTime(int divisor, char unit) {
  float up = upTime / divisor;
  float life = lifeUpTime / divisor;
  
  lcd.setCursor(0, 0);
  lcd.print("Up Time: ");
  lcd.print(up, 2);
  lcd.print(unit);
  
  lcd.setCursor(0, 1);
  lcd.print("Life:    ");
  lcd.print(life, 2);
  lcd.print(unit);
}

void writeLogShifts(boolean lifeTime) {
  lcd.setCursor(0, 0);
  lcd.write((byte)1);
  if (lifeTime) lcd.print(padLeft(lifeShiftsToGear[0], 3));
  else lcd.print(padLeft(shiftsToGear[0], 3));
  lcd.print(" ");
  
  lcd.print("2:");
  if (lifeTime) lcd.print(padLeft(lifeShiftsToGear[1], 3));
  else lcd.print(padLeft(shiftsToGear[1], 3));
  lcd.print(" ");
  
  lcd.print("3:");
  if (lifeTime) lcd.print(padLeft(lifeShiftsToGear[2], 3));
  else lcd.print(padLeft(shiftsToGear[2], 3));
  
  lcd.setCursor(0, 1);
  lcd.write((byte)4);
  if (lifeTime) lcd.print(padLeft(lifeShiftsToGear[3], 3));
  else lcd.print(padLeft(shiftsToGear[3], 3));
  lcd.print(" ");
  
  lcd.print("5:");
  if (lifeTime) lcd.print(padLeft(lifeShiftsToGear[4], 3));
  else lcd.print(padLeft(shiftsToGear[4], 3));
  lcd.print(" ");
  
  lcd.print("R:");
  if (lifeTime) lcd.print(padLeft(lifeShiftsToGear[5], 3));
  else lcd.print(padLeft(shiftsToGear[5], 3));
}

void writeLogGearTime(int divisor, char unit, boolean lifeTime) {
  lcd.setCursor(0, 0);
  lcd.write((byte)1);
  if (lifeTime) lcd.print(padLeft(lifeTimeInGear[1] / divisor, 2));
  else lcd.print(padLeft(timeInGear[1] / divisor, 2));
  lcd.print(unit);
  lcd.print(" ");
  
  lcd.print("2:");
  if (lifeTime) lcd.print(padLeft(lifeTimeInGear[2] / divisor, 2));
  else lcd.print(padLeft(timeInGear[2] / divisor, 2));
  lcd.print(unit);
  lcd.print(" ");
  
  lcd.print("3:");
  if (lifeTime) lcd.print(padLeft(lifeTimeInGear[3] / divisor, 2));
  else lcd.print(padLeft(timeInGear[3] / divisor, 2));
  lcd.print(unit);
  
  lcd.setCursor(0, 1);
  lcd.write((byte)4);
  if (lifeTime) lcd.print(padLeft(lifeTimeInGear[4] / divisor, 2));
  else lcd.print(padLeft(timeInGear[4] / divisor, 2));
  lcd.print(unit);
  lcd.print(" ");
  
  lcd.print("5:");
  if (lifeTime) lcd.print(padLeft(lifeTimeInGear[5] / divisor, 2));
  else lcd.print(padLeft(timeInGear[5] / divisor, 2));
  lcd.print(unit);
  lcd.print(" ");
  
  lcd.print("R:");
  if (lifeTime) lcd.print(padLeft(lifeTimeInGear[6] / divisor, 2));
  else lcd.print(padLeft(timeInGear[6] / divisor, 2));
  lcd.print(unit);
}

void writeLogGearPercent(boolean lifeTime) {
  unsigned long total;
  for (int g = 0; g < gears + 1; g++) {
    if (lifeTime) total += lifeTimeInGear[g];
    else total += timeInGear[g];
  }
  
  lcd.setCursor(0, 0);
  lcd.write((byte)1);
  if (lifeTime) lcd.print(padLeft(lifeTimeInGear[1] / total, 2));
  else lcd.print(padLeft(timeInGear[1] / total * 100, 0));
  lcd.print("% ");
  
  lcd.print("2:");
  if (lifeTime) lcd.print(padLeft(lifeTimeInGear[2] / total, 2));
  else lcd.print(padLeft(timeInGear[2] / total * 100, 0));
  lcd.print("% ");
  
  lcd.print("3:");
  if (lifeTime) lcd.print(padLeft(lifeTimeInGear[3] / total, 2));
  else lcd.print(padLeft(timeInGear[3] / total * 100, 0));
  lcd.print("%");
  
  lcd.setCursor(0, 1);
  lcd.write((byte)4);
  if (lifeTime) lcd.print(padLeft(lifeTimeInGear[4] / total, 2));
  else lcd.print(padLeft(timeInGear[4] / total * 100, 0));
  lcd.print("% ");
  
  lcd.print("5:");
  if (lifeTime) lcd.print(padLeft(lifeTimeInGear[5] / total, 2));
  else lcd.print(padLeft(timeInGear[5] / total * 100, 0));
  lcd.print("% ");
  
  lcd.print("R:");
  if (lifeTime) lcd.print(padLeft(lifeTimeInGear[6] / total, 2));
  else lcd.print(padLeft(timeInGear[6] / total * 100, 0));
  lcd.print("%");
}

void writeLogNeutral(int divisor, char unit, boolean lifeTime) {
  float gearTime = 0;
  float neutralTime = 0;
  if (lifeTime) {
    for (int g = 1; g < gears + 1; g++) {
      gearTime += lifeTimeInGear[g];
    }
    neutralTime = lifeTimeInGear[0];
  }
  else {
    for (int g = 1; g < gears + 1; g++) {
      gearTime += timeInGear[g];
    }
    neutralTime = timeInGear[0];
  }
  
  lcd.setCursor(0, 0);
  if (lifeTime) lcd.print("T ");
  lcd.print("In Gear: ");
  if (divisor > 0) lcd.print(gearTime / divisor, 2);
  else lcd.print(gearTime / (gearTime + neutralTime) * 100, 1);
  lcd.print(unit);
  
  lcd.setCursor(0, 1);
  if (lifeTime) lcd.print("T ");
  lcd.print("Neutral: ");
  if (divisor > 0) lcd.print(neutralTime / divisor, 2);
  else lcd.print(neutralTime / (gearTime + neutralTime) * 100, 1);
  lcd.print(unit);
}

void writeCredits() {
  lcd.setCursor(0, 0);
  lcd.print("- Gear Display -");
  
  lcd.setCursor(0, 1);
  lcd.print("By Eric C Menze.");
}

void handleButtons() {
  //Check for Mode Press
  if (checkPress(modePin)) {
    if (inMenu) {
      menuMode ++;
      if (menuMode > MENU_END) menuMode = MENU_BEGIN;
    }
    else if (inLog) {
      logMode ++;
      if (logMode > LOG_END) logMode = LOG_BEGIN;
    }
    else {
      mode++;
      if (mode > MODE_END) mode = MODE_BEGIN;
      else if (!debug && (mode == MODE_VOLTAGES || mode == MODE_VALUES)) mode = MODE_MENU;
    }
    delay(100);
    lcd.clear();
  }
  else if (checkPress(upPin)) {
    if (!inMenu && mode == MODE_MENU) {
      inMenu = true;
    }
    else if (inMenu && menuMode == MENU_METHOD) {
      method++;
      if (method > METHOD_END) method = METHOD_BEGIN;
      eepromUpdateNeeded = true;
    }
    else if (inMenu && menuMode == MENU_LED) {
      led++;
      if (led > LED_END) led = LED_BEGIN;
      eepromUpdateNeeded = true;
    }
    else if (inMenu && menuMode == MENU_BRIGHTNESS && brightness + brightnessInterval < 255) {
      brightness += brightnessInterval;
      eepromUpdateNeeded = true;
    }
    else if (inMenu && menuMode == MENU_LOGGING) {
      enableLog = !enableLog;
      eepromUpdateNeeded = true;
    }
    else if (inMenu && menuMode == MENU_TOLERANCE_INTERVAL && toleranceInterval < 255) {
      toleranceInterval++;
      eepromUpdateNeeded = true;
    }
    else if (inMenu && menuMode == MENU_DEBUG) {
      debug = !debug;
      eepromUpdateNeeded = true;
    }
    else if (inMenu && menuMode == MENU_EXIT) {
      inMenu = false;
    }
    else if (!inLog && mode == MODE_LOG) {
      inLog = true;
    }
    else if (inLog && logMode == LOG_EXIT) {
      inLog = false;
    }
    else if (tolerance < 1023 - toleranceInterval) {
      tolerance += toleranceInterval;
      eepromUpdateNeeded = true;
    }
  }
  else if (checkPress(downPin)) {
    if (!inMenu && mode == MODE_MENU) {
      inMenu = true;
    }
    else if (inMenu && menuMode == MENU_METHOD) {
      method--;
      if (method < MEAN_BASED) method = HIGH_BASED;
      eepromUpdateNeeded = true;
    }
    else if (inMenu && menuMode == MENU_LED) {
      led--;
      if (led < LED_BEGIN) led = LED_END;
      eepromUpdateNeeded = true;
    }
    else if (inMenu && menuMode == MENU_BRIGHTNESS && brightness > brightnessInterval) {
      brightness -= brightnessInterval;
      eepromUpdateNeeded = true;
    }
    else if (inMenu && menuMode == MENU_LOGGING) {
      enableLog = !enableLog;
      eepromUpdateNeeded = true;
    }
    else if (inMenu && menuMode == MENU_TOLERANCE_INTERVAL && toleranceInterval < 255) {
      toleranceInterval++;
      eepromUpdateNeeded = true;
    }
    else if (inMenu && menuMode == MENU_DEBUG) {
      debug = !debug;
      eepromUpdateNeeded = true;
    }
    else if (inMenu && menuMode == MENU_EXIT) {
      inMenu = false;
    }
    else if (!inLog && mode == MODE_LOG) {
      inLog = true;
    }
    else if (inLog && logMode == LOG_EXIT) {
      inLog = false;
    }
    else if (tolerance > toleranceInterval && tolerance > 0) {
      tolerance -= toleranceInterval;
      eepromUpdateNeeded = true;
    }
  }
}

boolean checkPress(int pin) {
  return digitalRead(pin) == LOW && debounce(pin, LOW, 20, 6);
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
  tolerance = EEPROM.read(eepromTolerance)*256 + EEPROM.read(eepromTolerance+1);
  debug = EEPROM.read(eepromDebug);
  led = EEPROM.read(eepromLED);
  enableLog = EEPROM.read(eepromEnableLog);
  toleranceInterval = EEPROM.read(eepromToleranceInterval);
  brightness = EEPROM.read(eepromBrightness);
}

void writeEEPROM() {
  EEPROM.write(eepromSet, 42); //EEPROM marked as initialized
  EEPROM.write(eepromMode, mode);
  EEPROM.write(eepromMethod, method);
  EEPROM.write(eepromSaveMode, saveMode);
  EEPROM.write(eepromTolerance, tolerance/256);
  EEPROM.write(eepromTolerance + 1, tolerance%256);
  EEPROM.write(eepromDebug, debug);
  EEPROM.write(eepromEnableLog, enableLog);
  EEPROM.write(eepromLED, led);
  EEPROM.write(eepromToleranceInterval, toleranceInterval);
  EEPROM.write(eepromBrightness, brightness);
  
  eepromLastUpdated = millis();
  eepromUpdateNeeded = false; 
}

void readLog() {
  lifeUpTime = readULongEEPROM(eepromLogUpTime);
  lifeTimeInGear[0] = readULongEEPROM(eepromLogGearTime[0]);
  for(int g = 0; g < gears; g++) {
    lifeTimeInGear[g + 1] = readULongEEPROM(eepromLogGearTime[g + 1]);
    lifeShiftsToGear[g] = readULongEEPROM(eepromLogShiftCount[g]);
  }
}

void writeLog() {
  writeULongEEPROM(eepromLogUpTime, lifeUpTime);
  writeULongEEPROM(eepromLogGearTime[0], lifeTimeInGear[0]);
  for(int g = 0; g < gears; g++) {
    writeULongEEPROM(eepromLogGearTime[g + 1], lifeTimeInGear[g + 1]);
    writeULongEEPROM(eepromLogShiftCount[g], lifeShiftsToGear[g]);
  }
  logRefreshed = millis();
}

unsigned long readULongEEPROM(int address) {
  return EEPROM.read(address)*16777216 + EEPROM.read(address+1)*65536 + EEPROM.read(address+2)*256 + EEPROM.read(address+3);
}

void writeULongEEPROM(int address, unsigned long v) {
  EEPROM.write(address, (v / 16777216) % 256);
  EEPROM.write(address + 1, (v / 65536) % 256);
  EEPROM.write(address + 2, (v / 256) % 256);
  EEPROM.write(address + 3, v % 256);
}

void writeDebug(unsigned long loopTime) {
  Serial.print("Mode: ");
  Serial.print(mode);
  Serial.print(" (Loop Time: ");
  Serial.print(loopTime);
  Serial.println("ms)");
  
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
