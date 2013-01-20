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
#define MODE_FILL 8
#define MODE_MENU 9
#define MODE_LOG 10
#define MODE_CREDITS 11
#define MODE_DISPLAY_OFF 12

#define MODE_BEGIN 0
#define MODE_END 12

void updateDisplay(int mode, int gear) {
  switch(mode) {
    case MODE_BASIC: writeBasic(gear, ' '); break;
    case MODE_BASIC_S: writeBasic(gear, accentChar(accent)); break;
    case MODE_ADVANCED: writeAdvanced(gear, ' '); break;
    case MODE_ADVANCED_S: writeAdvanced(gear, accentChar(accent)); break;
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
    else if (method == THEORETICAL) lcd.print("Y:");
    else if (method == LOW_BASED) lcd.print("L:");
    else if (method == HIGH_BASED) lcd.print("H:");
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
  if (method == MEAN_BASED) lcd.print("A:");
  else if (method == THEORETICAL) lcd.print("Y:");
  else if (method == LOW_BASED) lcd.print("L:");
  else if (method == HIGH_BASED) lcd.print("H:");
  else lcd.print("B:");
  lcd.print(toVoltage(baseline), 2);
  lcd.print("V");
}

void writeValues() {
  lcd.setCursor(0, 0);
  lcd.write((byte)ONE_COLON);
  lcd.print(formatValue(values[0]));
  lcd.print(" ");
  
  lcd.print("2:");
  lcd.print(formatValue(values[1]));
  lcd.print(" ");
  
  lcd.print("3:");
  lcd.print(formatValue(values[2]));
  
  lcd.setCursor(0, 1);
  lcd.write((byte)FOUR_COLON);
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
  else if (!debug && (mode == MODE_VOLTAGES || mode == MODE_VALUES)) mode = MODE_MENU; //Skip to Menu if not in Debug Mode
  stageEEPROM();
}

void upPressed() {
  if (!inMenu && mode == MODE_MENU) {
    inMenu = true;
  }
  else if (!inLog && mode == MODE_LOG) {
    inLog = true;
  }
  else if (tolerance + toleranceInterval <= 1023) {
    tolerance += toleranceInterval;
    stageEEPROM();
  }
}

void downPressed() {
  if (!inMenu && mode == MODE_MENU) {
    inMenu = true;
  }
  else if (!inLog && mode == MODE_LOG) {
    inLog = true;
  }
  else if (tolerance - toleranceInterval >= 0) {
    tolerance -= toleranceInterval;
    stageEEPROM();
  }
}
