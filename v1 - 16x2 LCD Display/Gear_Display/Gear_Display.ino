/*
Manual Transmission Gear Display
Ehryk Menze
*/

#include <EEPROM.h>
#include <LiquidCrystal.h>

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

//Configure Pins
int gears = 6;
int gearPin[6] = {0, 1, 2, 3, 4, 5}; //Analog Input Pins
//Digital Input Pins
int ledPin = 2;
int modePin = 10;
int upPin = 11;
int downPin = 12;

//Declare Global Variables
//Values will range between 0 and 1023, (0V and 5V respectively)
//With a Resolution of 4.9mV / unit
int values[6];
int theoretical = 511; //This is what should be read by the ideal Hall Effect Sensor, in a neutral magnetic environment.
int numberActive = -2; //Number of gears engaged
int method = 0; //Method of determining gear engagement (Starting at MEAN_BASED)
int mode = 0; //Which display mode is selected
boolean saveMode = true; //Persists mode across power cycles
boolean inMenu = false; //Whether the user is in the menu system
int menuMode = 0; //Which menu mode is selected
boolean inLog = false; //Whether the user is in the log system
int logMode = 0; //Which log mode is selected
int screenMode = 0; //Which 'version' of a given screen is being displayed
int brightness = 255; //LCD Brightness
int brightnessInterval = 15; //LCD Brightness Increment to increase/decrease by
char accent = 0; //Surrounds gear char
char accents[5] = {'-', '*', '.', '~', '='};
int ACCENT_COUNT = 5;
int gear = -2; //Which gear is active, if any. 0 = Neutral.
int baseline = 0; //The baseline from which any gear out of tolerance from is considered engaged
int tolerance; //How much a gear can vary from the baseline before considered engaged
int toleranceInterval = 5; //How much to vary the tolerance on a single press
boolean inGear = false; //Whether or not the vehicle is in a gear
int led = 1; //Whether or not to light the LED when a gear is engaged (default), neutral, or off
int baud = 9600; //Used for Serial communication
int defaultTolerance = 200; //Initial Tolerance
unsigned long lastLoopStart = 0;

//Debugging
boolean debug = true;
unsigned long debugRefresh = 0;
int debugInterval = 1000; //How often to refresh the serial port, in milliseconds

//These are addresses in the EEPROM for persistent storage
boolean enableEEPROM = true; //Uses EEPROM
boolean clearEEPROM = false; // -- SET TO TRUE ONLY WHEN ADDRESSES CHANGE, or to reset EEPROM to defaults --
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
int eepromAccent = 11;
boolean eepromUpdateNeeded = false;
boolean powerLoss = false;
unsigned long eepromLastUpdated = 0;
int eepromInterval = 10000; //How often to wait after a change to write to EEPROM

//Logging
boolean enableLog = true;
boolean clearLogBoolean = false; // -- SET TO TRUE ONLY WHEN ADDRESSES CHANGE, or to reset persistent log  --
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

/*  SETUP  */
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
  
  createCustomCharacters();
  
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

/*  LOOP  */
//loop() gets called repeatedly for the duration of power
void loop() {
  checkPowerLoss(); //Check for power loss
  if (powerLoss) shutDown();
  
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
  
  setLED(inGear);
  setLCDBrightness(brightness);
  
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

