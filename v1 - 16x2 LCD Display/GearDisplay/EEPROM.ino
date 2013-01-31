/*
Reads and writes to and from the EEPROM for persistant storage
*/

/* Define EEPROM Addresses */
#define EEPROM_SET_ADDRESS 0
#define EEPROM_MODE_ADDRESS 1
#define EEPROM_METHOD_ADDRESS 2
#define EEPROM_SAVE_MODE_ADDRESS 3
#define EEPROM_TOLERANCE_ADDRESS 4 //Reguires two bytes
#define EEPROM_LED_ADDRESS 6
#define EEPROM_BRIGHTNESS_ADDRESS 7
#define EEPROM_ACCENT_ADDRESS 8
#define EEPROM_TOLERANCE_INTERVAL_ADDRESS 9
#define EEPROM_ENABLE_LOG_ADDRESS 10
#define EEPROM_DEBUG_ADDRESS 11
#define EEPROM_UPDATE_ADDRESS 12 //Reguires two bytes
#define EEPROM_LOW_VOLTAGE_ADDRESS 14 //Reguires two bytes

/* Value to set in EEPROM to be sure it's been set */
#define EEPROM_SET_VALUE 42

/* EEPROM Log Addresses */
//Based on a single starting address
//Move the start address if EEPROM gets worn at the given start address
int EEPROM_LOG_START_ADDRESS = 128;
int eepromLogGearTime[1 + GEARS] = { EEPROM_LOG_START_ADDRESS, EEPROM_LOG_START_ADDRESS + 4, EEPROM_LOG_START_ADDRESS + 8, EEPROM_LOG_START_ADDRESS + 12, EEPROM_LOG_START_ADDRESS + 16, EEPROM_LOG_START_ADDRESS + 20, EEPROM_LOG_START_ADDRESS + 24};
int eepromLogUpTime = EEPROM_LOG_START_ADDRESS + 28;
int eepromLogShiftCount[GEARS] = {EEPROM_LOG_START_ADDRESS + 32, EEPROM_LOG_START_ADDRESS + 34, EEPROM_LOG_START_ADDRESS + 36, EEPROM_LOG_START_ADDRESS + 38, EEPROM_LOG_START_ADDRESS + 40, EEPROM_LOG_START_ADDRESS + 42};

boolean readEEPROM() {
  if (EEPROM.read(EEPROM_SET_ADDRESS) != EEPROM_SET_VALUE || clearEEPROM) return false; //EEPROM not set yet
  mode = EEPROM.read(EEPROM_MODE_ADDRESS);
  method = EEPROM.read(EEPROM_METHOD_ADDRESS);
  saveMode = EEPROM.read(EEPROM_SAVE_MODE_ADDRESS);
  tolerance = readUIntEEPROM(EEPROM_TOLERANCE_ADDRESS);
  led = EEPROM.read(EEPROM_LED_ADDRESS);
  brightness = EEPROM.read(EEPROM_BRIGHTNESS_ADDRESS);
  accent = EEPROM.read(EEPROM_ACCENT_ADDRESS);
  toleranceInterval = EEPROM.read(EEPROM_TOLERANCE_INTERVAL_ADDRESS);
  enableLog = EEPROM.read(EEPROM_ENABLE_LOG_ADDRESS);
  debug = EEPROM.read(EEPROM_DEBUG_ADDRESS);
  updateInterval = readUIntEEPROM(EEPROM_UPDATE_ADDRESS);
  lowVoltage = readUIntEEPROM(EEPROM_LOW_VOLTAGE_ADDRESS);
  return true;
}

void writeEEPROM() {
  EEPROM.write(EEPROM_SET_ADDRESS, EEPROM_SET_VALUE); //EEPROM marked as initialized
  EEPROM.write(EEPROM_MODE_ADDRESS, mode);
  EEPROM.write(EEPROM_METHOD_ADDRESS, method);
  EEPROM.write(EEPROM_SAVE_MODE_ADDRESS, saveMode);
  writeUIntEEPROM(EEPROM_TOLERANCE_ADDRESS, tolerance);
  EEPROM.write(EEPROM_LED_ADDRESS, led);
  EEPROM.write(EEPROM_BRIGHTNESS_ADDRESS, brightness);
  EEPROM.write(EEPROM_ACCENT_ADDRESS, accent);
  EEPROM.write(EEPROM_TOLERANCE_INTERVAL_ADDRESS, toleranceInterval);
  EEPROM.write(EEPROM_ENABLE_LOG_ADDRESS, enableLog);
  EEPROM.write(EEPROM_DEBUG_ADDRESS, debug);
  writeUIntEEPROM(EEPROM_UPDATE_ADDRESS, updateInterval);
  writeUIntEEPROM(EEPROM_LOW_VOLTAGE_ADDRESS, lowVoltage);
  
  eepromLastUpdated = millis();
  eepromUpdateNeeded = false; 
}

void stageEEPROM() {
  eepromUpdateNeeded = true;
  eepromLastChanged = millis();
}

void readLog() {
  if (clearLogBoolean) return;
  lifeUpTime = readULongEEPROM(eepromLogUpTime);
  lifeTimeInGear[0] = readULongEEPROM(eepromLogGearTime[0]);
  for(int g = 0; g < GEARS; g++) {
    lifeTimeInGear[g + 1] = readULongEEPROM(eepromLogGearTime[g + 1]);
    lifeShiftsToGear[g] = readUIntEEPROM(eepromLogShiftCount[g]);
  }
}

void writeLog() {
  writeULongEEPROM(eepromLogUpTime, lifeUpTime);
  writeULongEEPROM(eepromLogGearTime[0], lifeTimeInGear[0]);
  for(int g = 0; g < GEARS; g++) {
    writeULongEEPROM(eepromLogGearTime[g + 1], lifeTimeInGear[g + 1]);
    writeUIntEEPROM(eepromLogShiftCount[g], lifeShiftsToGear[g]);
  }
  logRefreshed = millis();
}

unsigned long readUIntEEPROM(int address) {
  return (EEPROM.read(address) << 8) + EEPROM.read(address+1);
}

void writeUIntEEPROM(int address, unsigned int v) {
  EEPROM.write(address, (v >> 8) % 256);
  EEPROM.write(address + 1, v % 256);
}

unsigned long readULongEEPROM(int address) {
  return (EEPROM.read(address) << 24) + (EEPROM.read(address+1) << 16) + (EEPROM.read(address+2) << 8) + EEPROM.read(address+3);
}

void writeULongEEPROM(int address, unsigned long v) {
  EEPROM.write(address, (v >> 24) % 256);
  EEPROM.write(address + 1, (v >> 16) % 256);
  EEPROM.write(address + 2, (v >> 8) % 256);
  EEPROM.write(address + 3, v % 256);
}

unsigned long resetEEPROM() {
  unsigned long resetStart = millis();
  for (int i = 0; i < 512; i++)
    EEPROM.write(i, 0);
   return millis() - resetStart;
}

void checkPowerLoss() {
  powerLoss = voltageLow();
}

boolean voltageLow() {
  if (lowVoltage == 0) return false;
  return readVcc() < lowVoltage;
}

void shutDown() {
  unsigned long saveTime = millis();
  if (!debug) {
    digitalWrite(LED_PIN, LOW); //Turn Off LED
    digitalWrite(LCD_BRIGHTNESS_PIN, LOW); //Turn Off LCD Backlight
  }
  else analogWrite(LCD_BRIGHTNESS_PIN, 30);
  if (enableLog) writeLog(); //Save Log Values
  if (enableEEPROM) writeEEPROM(); //Save Variable State
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("** POWER LOST **");
  
  if (debug) {
    saveTime = millis() - saveTime;
    Serial.println();
    Serial.println();
    Serial.println("POWER LOST.");
    Serial.print("Written to EEPROM in ");
    Serial.print(saveTime);
    Serial.println("ms");
    Serial.println();
  }
  
  do {
    if (debug) {
      Serial.print("Voltage Low: ");
      Serial.print(readVcc());
      Serial.println("mV");
    }
    delay(5000); //Wait until expected powerdown, check for voltage return every 5 seconds
  } while (voltageLow());
  
  powerLoss = false;
  if (debug) {
    Serial.print("POWER RESTORED. ");
    Serial.print(readVcc());
    Serial.println("mV");
    Serial.println();
  }
  return;
}

void clearLog(boolean lifeTime) {
  timeInGear[0] = 0;
  for (int i = 0; i < GEARS; i++) {
    timeInGear[i + 1] = 0;
    shiftsToGear[i] = 0;
  }
  
  if (lifeTime) {
    lifeTimeInGear[0] = 0;
    for (int i = 0; i < GEARS; i++) {
      lifeTimeInGear[i + 1] = 0;
      lifeShiftsToGear[i] = 0;
    }
    writeLog();
  }
  
  lcd.setCursor(0, 1);
  if (lifeTime) lcd.print("  Life Cleared! ");
  else lcd.print("  Trip Cleared! ");
  delay(2000);
}
