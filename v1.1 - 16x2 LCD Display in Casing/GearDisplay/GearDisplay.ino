/*
Manual Transmission Gear Display
Ehryk Menze
*/

#include <EEPROM.h>
#include <LiquidCrystal.h>

/* Number of Gears, including Reverse. Only tested on a 5 speed */
#define GEARS 6

/* Define Pin Usage */
//Digital Input Pins
#define DOWN_PIN 2
#define UP_PIN 3
#define MODE_PIN 4
#define LED_PIN 5
#define LCD_BRIGHTNESS_PIN 6
#define LCD_RS_PIN 7
#define LCD_ENABLE_PIN 8
#define LCD_D7 9
#define LCD_D6 10
#define LCD_D5 11
#define LCD_D4 12
int gearPin[GEARS] = {0, 1, 2, 3, 4, 5}; //Analog Input Pins

/* Compiler Constants, Defaults */
#define BAUD 9600 //Used for Serial communication
#define DEFAULT_TOLERANCE 100 //Initial Tolerance Value
#define DEFAULT_TOLERANCE_INCREMENT 5 //Initial Tolerance Increment/Decrement Value
#define DEFAULT_BRIGHTNESS 255 //Initial Brightness Value
#define BRIGHTNESS_INTERVAL 15 //LCD Brightness Increment to increase/decrease by (PWM, ranges from 0-255)
#define LOW_VOLTAGE_LIMIT 3900 //How low in mV on VCC Before ShutDown() (Save values to EEPROM). Set above 5000 to disable when not using a large capacitor to keep alive post-power cut (Needs about 200ms)
#define ACCENT_COUNT 5
const char accents[ACCENT_COUNT] = {'-', '*', '.', '~', '='};
#define DEBUG_INTERVAL 1000 //How often to output Debug information, in milliseconds
#define EEPROM_INTERVAL 5000 //How often to wait after last change to write to EEPROM
#define LOG_INTERVAL 60000 //How often to auto-save Log information to EEPROM, in milliseconds

//Set up LCD pins for 4 bit mode
//initialize the library with the numbers of the interface pins
LiquidCrystal lcd(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

//Declare Global Variables
//Values will range between 0 and 1023, (0V and 5V respectively)
//With a Resolution of 4.9mV / unit
int values[6];
int theoretical = 511; //This is what should be read by the ideal Hall Effect Sensor, in a neutral magnetic environment. (1023 / 2)
int numberActive = -2; //Number of gears engaged
int method = 0; //Method of determining gear engagement (Starting at MEAN_BASED)
int mode = 0; //Which display mode is selected
boolean saveMode = true; //Persists mode across power cycles
boolean inMenu = false; //Whether the user is in the menu system
int menuMode = 0; //Which menu mode is selected
boolean inLog = false; //Whether the user is in the log system
int logMode = 0; //Which log mode is selected
int screenMode = 0; //Which 'version' of a given screen is being displayed
int vcc = 5000; //Voltage input in mV, should be 5000mV (5V) in theory
int brightness = 255; //LCD Brightness
char accent = 0; //Surrounds gear char
int gear = -2; //Which gear is active, if any. 0 = Neutral.
int lastGear = -2; //Which gear was last active, if any. 0 = Neutral.
int baseline = 0; //The baseline from which any gear out of tolerance from is considered engaged
int tolerance; //How much a gear can vary from the baseline before considered engaged
int toleranceInterval = DEFAULT_TOLERANCE_INCREMENT; //How much to vary the tolerance on a single press
boolean inGear = false; //Whether or not the vehicle is in a gear
int led = 1; //Whether or not to light the LED when a gear is engaged (default), neutral, or off
boolean updateNeeded = 0; //Whether or not to update the display immediately
unsigned long displayLastUpdated = 0; //When the display was last updated
unsigned long updateInterval = 500; //When to update the display between non-immediate updates

unsigned long lastLoopStart = 0;

//Debugging
boolean debug = true;
unsigned long debugRefresh = 0;

//These are addresses in the EEPROM for persistent storage
boolean enableEEPROM = true; //Uses EEPROM
boolean clearEEPROM = false; // -- SET TO TRUE ONLY WHEN ADDRESSES CHANGE, or to reset EEPROM to defaults --
boolean eepromUpdateNeeded = false;
boolean powerLoss = false;
unsigned long eepromLastChanged = -1;
unsigned long eepromLastUpdated = 0;

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


/*  SETUP  */
//setup() gets called once at power on/reset of Arduino
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(BAUD);
  analogReference(DEFAULT); //Use 5v Default AREF
  
  //Set pin modes
  for (int g = 0; g < GEARS; g++) {
    pinMode(g, INPUT);
  }
  pinMode(LED_PIN, OUTPUT);
  pinMode(MODE_PIN, INPUT);
  pinMode(UP_PIN, INPUT);
  pinMode(DOWN_PIN, INPUT);
  pinMode(LCD_BRIGHTNESS_PIN, OUTPUT);
  
  //Use internal pull-up resistors
  digitalWrite(MODE_PIN, HIGH);
  digitalWrite(UP_PIN, HIGH);
  digitalWrite(DOWN_PIN, HIGH);
  
  //Set up LCD
  brightness = DEFAULT_BRIGHTNESS;
  setLCDBrightness(brightness);
  lcd.begin(16, 2);
  createCustomCharacters();
  
  tolerance = DEFAULT_TOLERANCE;
  
  //Initialize Log
  timeInGear[0] = 0;
  for (int g = 0; g < GEARS; g++) {
    timeInGear[g + 1] = 0;
    shiftsToGear[g] = 0;
  }
  
  //Read from EEPROM
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
  int previous = gear;
  gear = activeGear();
  inGear = gear > 0;
  
  if (previous > 0) lastGear = previous; // If not Neutral
  updateNeeded = previous != gear;
  
  setLED(inGear);
  setLCDBrightness(brightness);
  
  //This saves all values to EEPROM, if a change has happened, and the requisite amount of time has passed since the last change
  if (enableEEPROM && eepromUpdateNeeded && eepromLastChanged >= 0 && millis() - eepromLastChanged > EEPROM_INTERVAL) writeEEPROM();
  
  if (enableLog) {
    if (previous == gear) { //Remained in a gear (or neutral)
      timeInGear[gear] += loopTime / 100; //Store in tenths
      lifeTimeInGear[gear] += loopTime / 100;
    }
    upTime += loopTime / 100; //Loop success, increment uptime
    lifeUpTime += loopTime / 100;
    if (previous == 0 && inGear) { //Shifted into a gear from Neutral
      shiftsToGear[gear - 1]++;
      lifeShiftsToGear[gear - 1]++;
    }
  }
  
  //This writes the Log values, if the log interval (in ms) has passed
  if (enableLog && millis() - logRefreshed > LOG_INTERVAL) writeLog();
  
  //This rereads in the case of an error or fault
  //(doesn't display errors, leaves last displayed values)
  if (!debug && gear < 0) return;
  
  if (debug && millis() - debugRefresh > DEBUG_INTERVAL) writeDebug(loopTime);
  
  //Update the display
  if (updateNeeded || millis() - displayLastUpdated > updateInterval)
    updateDisplay(mode, gear);
}

