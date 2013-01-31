/*
The Menu System is how the user navigates through GearDisplay
It has two submenus, Menu and Log
*/

//Screens in the Menu System
#define MODE_BASIC 0
#define MODE_BASIC_S 1
#define MODE_ADVANCED 2
#define MODE_ADVANCED_S 3
#define MODE_VARIABLES_V 4
#define MODE_VARIABLES 5
#define MODE_VOLTAGES 6 //Debug Only
#define MODE_VALUES 7 //Debug only
#define MODE_V_IN 8 //Debug only
#define MODE_FILL 9
#define MODE_MENU 10
#define MODE_LOG 11
#define MODE_CREDITS 12
#define MODE_DISPLAY_OFF 13

#define MODE_BEGIN 0
#define MODE_END 13

void updateDisplay(int mode, int gear) {
  switch(mode) {
    case MODE_BASIC: writeBasic(gear, ' '); break;
    case MODE_BASIC_S: writeBasic(gear, accentChar(accent)); break;
    case MODE_ADVANCED: writeAdvanced(gear, ' '); break;
    case MODE_ADVANCED_S: writeAdvanced(gear, accentChar(accent)); break;
    case MODE_VARIABLES_V: writeVariables(gear, true); break;
    case MODE_VARIABLES: writeVariables(gear, false); break;
    case MODE_VOLTAGES: writeMenuVoltages(); break;
    case MODE_VALUES: writeMenuValues(); break;
    case MODE_V_IN: writeMenuVin(); break;
    case MODE_FILL: writeFill(gearChar(gear)); break;
    case MODE_MENU: writeMenu(menuMode); break;
    case MODE_LOG: writeLog(logMode); break;
    case MODE_CREDITS: writeCredits(); break;
    case MODE_DISPLAY_OFF: break;
  }
  
  displayOff = mode == MODE_DISPLAY_OFF;
  displayLastUpdated = millis();
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
    writeMethodChar(method);
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
  lcd.write((byte)SIGMA);
  lcd.print(":");
  lcd.print(getStandardDeviation(baseline), 2);
  
  //Print Tolerance
  lcd.setCursor(0, 1);
  if (voltage) {
    lcd.write((byte)T_COLON);
    lcd.print(toVoltage(tolerance), 2);
    lcd.print("V");
  }
  else {
    lcd.print("T:");
    lcd.print(formatValue(tolerance));
  }
  
  //Print Debug
  lcd.setCursor(7, 1);
  if (debug) lcd.print("D");
  
  //Print Baseline Voltage
  lcd.setCursor(9, 1);
  writeMethodChar(method);
  lcd.print(toVoltage(baseline), 2);
  lcd.print("V");
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

void writeCredits() {
  lcd.setCursor(0, 0);
  lcd.print("- Gear Display -");
  
  lcd.setCursor(0, 1);
  lcd.print("By Eric C Menze.");
}

//                      //
/* -- INPUT HANDLING -- */
//                      //

void modePressed() {
  mode++;
  if (mode > MODE_END) mode = MODE_BEGIN;
  else if (!debug && (mode == MODE_VOLTAGES || mode == MODE_VALUES)) mode = MODE_FILL; //Skip to Fill if not in Debug Mode
  stageEEPROM();
  delay(200);
}

void upPressed() {
  switch(mode) {
    case MODE_MENU: inMenu = true; break;
    case MODE_LOG: inLog = true; break;
    case MODE_V_IN: 
      if (lowVoltage < 5000) {
        lowVoltage += 100;
        if (lowVoltage < 3000) lowVoltage = 3000;
        stageEEPROM();
      }
      break;
    default: 
      if (tolerance + toleranceInterval <= 1023) {
        tolerance += toleranceInterval;
        stageEEPROM();
      }
      break;
  }
}

void downPressed() {
  switch(mode) {
    case MODE_MENU: inMenu = true; break;
    case MODE_LOG: inLog = true; break;
    case MODE_V_IN: 
      if (lowVoltage > 0) {
        lowVoltage -= 100;
        if (lowVoltage < 3000) lowVoltage = 0;
        stageEEPROM();
      }
      break;
    default: 
      if (tolerance - toleranceInterval > 0) {
        tolerance -= toleranceInterval;
        stageEEPROM();
      }
      break;
  }
}
