/*
The Menu Submenu can be entered from the Menu System
It provides the ability to change variables and affect system operation
*/

//Screens in the Menu Submenu
#define MENU_METHOD 0
#define MENU_LED 1
#define MENU_BRIGHTNESS 2
#define MENU_TOLERANCE_INTERVAL 3
#define MENU_SEPARATOR 4
#define MENU_LOGGING 5
#define MENU_DEBUG 6
#define MENU_VOLTAGES 7
#define MENU_VALUES 8
#define MENU_V_IN 9
#define MENU_UPDATE 10
#define MENU_EXIT 11

#define MENU_BEGIN 0
#define MENU_END 11

void writeMenu(int menuScreen) {
  if (!inMenu) writePrompt("ENTER MENU:");
  else {
    switch (menuScreen) {
      case MENU_METHOD: writeMenuMethod(); break;
      case MENU_LED: writeMenuLED(); break;
      case MENU_BRIGHTNESS: writeMenuBrightness(); break;
      case MENU_TOLERANCE_INTERVAL: writeMenuToleranceInterval(); break;
      case MENU_SEPARATOR: writeMenuSeparator(); break;
      case MENU_LOGGING: writeMenuLogging(); break;
      case MENU_DEBUG: writeMenuDebug(); break;
      case MENU_VOLTAGES: writeMenuVoltages(); break;
      case MENU_VALUES: writeMenuValues(); break;
      case MENU_V_IN: writeMenuVin(); break;
      case MENU_UPDATE: writeMenuUpdate(); break;
      case MENU_EXIT: writePrompt("EXIT MENU: "); break;
    }
  }
}

void writeMenuMethod() {
  lcd.setCursor(0, 0);
  lcd.print("Method Used:");
  
  lcd.setCursor(0, 1);
  switch(method) {
    case METHOD_MEAN_BASED:        lcd.print("Mean (Default)  "); break;
    case METHOD_TRIM1_MEAN:        lcd.print("Trim 1 - Mean   "); break;
    case METHOD_TRIM1_THEORETICAL: lcd.print("Trim 1 - Theory "); break;
    case METHOD_TRIM1_HIGHEST:     lcd.print("Trim 1 - Highest"); break;
    case METHOD_TRIM1_LOWEST:      lcd.print("Trim 1 - Lowest "); break;
    case METHOD_TRIM2_MEAN:        lcd.print("Trim 2 - Mean   "); break;
    case METHOD_TRIM2_THEORETICAL: lcd.print("Trim 2 - Theory "); break;
    case METHOD_TRIM2_HIGHEST:     lcd.print("Trim 2 - Highest"); break;
    case METHOD_TRIM2_LOWEST:      lcd.print("Trim 2 - Lowest "); break;
    case METHOD_TRIM3_MEAN:        lcd.print("Trim 3 - Mean   "); break;
    case METHOD_TRIM3_THEORETICAL: lcd.print("Trim 3 - Theory "); break;
    case METHOD_TRIM3_HIGHEST:     lcd.print("Trim 3 - Highest"); break;
    case METHOD_TRIM3_LOWEST:      lcd.print("Trim 3 - Lowest "); break;
    case METHOD_THEORETICAL:       lcd.print("Theoretical     "); break;
    case METHOD_LOW_BASED:         lcd.print("Low  (0 VDC)    "); break;
    case METHOD_HIGH_BASED:        lcd.print("High (+5 VDC)   "); break;
  }
}

void writeMenuLED() {
  lcd.setCursor(0, 0);
  lcd.print("LED:");
  
  lcd.setCursor(0, 1);
  switch (led) {
    case LED_IN_GEAR: lcd.print("ON - In Gear"); break;
    case LED_NEUTRAL: lcd.print("ON - Neutral"); break;
    case LED_CITY:    lcd.print("ON - City   "); break;
    case LED_HIGHWAY: lcd.print("ON - Highway"); break;
    case LED_ERROR:   lcd.print("ON - Error  "); break;
    case LED_OFF:     lcd.print("OFF         ");
    default:          lcd.print("UNKNOWN     "); break;
  }
}

void writeMenuBrightness() {
  lcd.setCursor(0, 0);
  lcd.print("LCD Brightness:");
  
  lcd.setCursor(0, 1);
  lcd.print("PWM:");
  lcd.print(padLeft(brightness, 3, " "));
  
  lcd.setCursor(8, 1);
  lcd.print("(");
  lcd.print(formatPercent((float)brightness/255 * 100, 2));
  lcd.print(") ");
}

void writeMenuToleranceInterval() {
  lcd.setCursor(0, 0);
  lcd.print("Toler. Interval:");
  
  lcd.setCursor(0, 1);
  lcd.print(padLeft(toleranceInterval, 3, " "));
  
  lcd.setCursor(5, 1);
  lcd.print("(");
  lcd.print(toVoltage(toleranceInterval), 4);
  lcd.print("V)");
}

void writeMenuSeparator() {
  lcd.setCursor(0, 0);
  lcd.print("Accent Char: ");
  lcd.print(accentChar(accent));
  
  lcd.setCursor(0, 1);
  lcd.print(accentChar(accent));
  lcd.print('N');
  lcd.print(accentChar(accent));
  lcd.print(" ");
  
  lcd.print(accentChar(accent));
  lcd.print('1');
  lcd.print(accentChar(accent));
  lcd.print(" ");
  
  lcd.print(accentChar(accent));
  lcd.print('4');
  lcd.print(accentChar(accent));
  lcd.print(" ");
  
  lcd.print(accentChar(accent));
  lcd.print('R');
  lcd.print(accentChar(accent));
}

void writeMenuLogging() {
  lcd.setCursor(0, 0);
  lcd.print("Logging:");
  
  lcd.setCursor(0, 1);
  if (enableLog) lcd.print("ON ");
  else lcd.print("OFF");
}

void writeMenuDebug() {
  lcd.setCursor(0, 0);
  lcd.print("Debug Mode:");
  
  lcd.setCursor(0, 1);
  if (debug) lcd.print("ON ");
  else lcd.print("OFF");
}

void writeMenuValues() {
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

void writeMenuVoltages() {
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

void writeMenuVin() {
  lcd.setCursor(0, 0);
  lcd.print("Vcc:");
  lcd.print(padLeft(vcc, 4, " "));
  lcd.print("mV ");
  
  lcd.print((float)vcc/1000, 2);
  lcd.print("V");
  
  lcd.setCursor(0, 1);
  lcd.print("Low:");
  lcd.print(padLeft(lowVoltage, 4, " "));
  lcd.print("mV ");
  
  lcd.print((float)lowVoltage/1000, 2);
  lcd.print("V");
  
}

void writeMenuUpdate() {
  lcd.setCursor(0, 0);
  lcd.print("Update Interval:");
  
  lcd.setCursor(0, 1);
  lcd.print(padLeft(updateInterval, 4, " "));
  lcd.print(" mS  Ex:");
  
  lcd.print(millis() % 10000);
}

//                      //
/* -- INPUT HANDLING -- */
//                      //

void menuModePressed() {
  menuMode ++;
  if (menuMode > MENU_END) menuMode = MENU_BEGIN;
  delay(200);
}

void menuUpPressed() {
  switch(menuMode) {
    case MENU_METHOD:
      method++;
      if (method > METHOD_END) method = METHOD_BEGIN;
      stageEEPROM();
      break;
    case MENU_LED: 
      led++;
      if (led > LED_END) led = LED_BEGIN;
      stageEEPROM();
      break;
    case MENU_BRIGHTNESS:
      if (brightness + BRIGHTNESS_INTERVAL <= 255) {
        brightness += BRIGHTNESS_INTERVAL;
        stageEEPROM();
      }
      break;
    case MENU_TOLERANCE_INTERVAL:
      if (toleranceInterval < 255) {
        toleranceInterval++;
        stageEEPROM();
      }
      break;
    case MENU_SEPARATOR:
      accent++;
      if (accent > ACCENT_COUNT) accent = 0;
      stageEEPROM();
      break;
    case MENU_LOGGING: 
      enableLog = !enableLog;
      stageEEPROM();
      break;
    case MENU_DEBUG: 
      debug = !debug;
      stageEEPROM();
      break;
    case MENU_VOLTAGES: break;
    case MENU_VALUES: break;
    case MENU_V_IN: 
      if (lowVoltage < 5000) {
        lowVoltage += 100;
        if (lowVoltage < 3000) lowVoltage = 3000;
        stageEEPROM();
      }
      break;
    case MENU_UPDATE:
      if (updateInterval < 5000) {
        updateInterval += 10;
        stageEEPROM();
      }
      break;
    case MENU_EXIT: inMenu = false; break;
  }
}

void menuDownPressed() {
  switch(menuMode) {
    case MENU_METHOD:
      method--;
      if (method < METHOD_BEGIN) method = METHOD_END;
      stageEEPROM();
      break;
    case MENU_LED: 
      led--;
      if (led < LED_BEGIN) led = LED_END;
      stageEEPROM();
      break;
    case MENU_BRIGHTNESS:
      if (brightness - BRIGHTNESS_INTERVAL >= 0) {
        brightness -= BRIGHTNESS_INTERVAL;
        stageEEPROM();
      }
      break;
    case MENU_TOLERANCE_INTERVAL:
      if (toleranceInterval > 0) {
        toleranceInterval--;
        stageEEPROM();
      }
      break;
    case MENU_SEPARATOR:
      accent--;
      if (accent < 0) accent = ACCENT_COUNT;
      stageEEPROM();
      break;
    case MENU_LOGGING: 
      enableLog = !enableLog;
      stageEEPROM();
      break;
    case MENU_DEBUG: 
      debug = !debug;
      stageEEPROM();
      break;
    case MENU_VOLTAGES: break;
    case MENU_VALUES: break;
    case MENU_V_IN: 
      if (lowVoltage > 0) {
        lowVoltage -= 100;
        if (lowVoltage < 3000) lowVoltage = 0;
        stageEEPROM();
      }
      break;
    case MENU_UPDATE:
      if (updateInterval > 0) {
        updateInterval -= 10;
        stageEEPROM();
      }
      break;
    case MENU_EXIT: inMenu = false; break;
  }
}
