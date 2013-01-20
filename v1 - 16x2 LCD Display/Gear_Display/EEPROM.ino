/*
Reads and writes to and from the EEPROM for persistant storage
*/

boolean readEEPROM() {
  if (EEPROM.read(eepromSet) != 42 || clearEEPROM) return false; //EEPROM not set yet
  mode = EEPROM.read(eepromMode);
  method = EEPROM.read(eepromMethod);
  tolerance = EEPROM.read(eepromTolerance)*256 + EEPROM.read(eepromTolerance+1);
  debug = EEPROM.read(eepromDebug);
  led = EEPROM.read(eepromLED);
  enableLog = EEPROM.read(eepromEnableLog);
  toleranceInterval = EEPROM.read(eepromToleranceInterval);
  brightness = EEPROM.read(eepromBrightness);
  accent = EEPROM.read(eepromAccent);
  return true;
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
  EEPROM.write(eepromAccent, accent);
  
  eepromLastUpdated = millis();
  eepromUpdateNeeded = false; 
}

void readLog() {
  if (clearLogBoolean) return;
  lifeUpTime = readULongEEPROM(eepromLogUpTime);
  lifeTimeInGear[0] = readULongEEPROM(eepromLogGearTime[0]);
  for(int g = 0; g < gears; g++) {
    lifeTimeInGear[g + 1] = readULongEEPROM(eepromLogGearTime[g + 1]);
    lifeShiftsToGear[g] = readUIntEEPROM(eepromLogShiftCount[g]);
  }
}

void writeLog() {
  writeULongEEPROM(eepromLogUpTime, lifeUpTime);
  writeULongEEPROM(eepromLogGearTime[0], lifeTimeInGear[0]);
  for(int g = 0; g < gears; g++) {
    writeULongEEPROM(eepromLogGearTime[g + 1], lifeTimeInGear[g + 1]);
    writeUIntEEPROM(eepromLogShiftCount[g], lifeShiftsToGear[g]);
  }
  logRefreshed = millis();
}

unsigned long readUIntEEPROM(int address) {
  return EEPROM.read(address)*256 + EEPROM.read(address+1);
}

void writeUIntEEPROM(int address, unsigned int v) {
  EEPROM.write(address, (v / 256) % 256);
  EEPROM.write(address + 1, v % 256);
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

void checkPowerLoss() {
  powerLoss = voltageLow();
}

boolean voltageLow() {
  return readVcc() < 3900;
}

void shutDown() {
  unsigned long saveTime = millis();
  if (!debug) {
    digitalWrite(ledPin, LOW); //Turn Off LED
    digitalWrite(LCD_BRIGHTNESS_PIN, LOW); //Turn Off LCD Backlight
  }
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
