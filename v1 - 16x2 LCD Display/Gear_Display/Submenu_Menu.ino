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
#define MENU_EXIT 9

#define MENU_BEGIN 0
#define MENU_END 9

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
      case MENU_VOLTAGES: writeVoltages(); break;
      case MENU_VALUES: writeValues(); break;
      case MENU_EXIT: writePrompt("EXIT MENU: "); break;
    }
  }
}

void writeMenuMethod() {
  lcd.setCursor(0, 0);
  lcd.print("Method Used:");
  
  lcd.setCursor(0, 1);
  switch(method) {
    case MEAN_BASED:          lcd.print("Mean (Default)  "); break;
    case TRIM1_MEAN:          lcd.print("Trim 1 - Mean   "); break;
    case TRIM1_THEORETICAL:   lcd.print("Trim 1 - Theory "); break;
    case TRIM1_HIGHEST:       lcd.print("Trim 1 - Highest"); break;
    case TRIM1_LOWEST:        lcd.print("Trim 1 - Lowest "); break;
    case TRIM2_MEAN:          lcd.print("Trim 2 - Mean   "); break;
    case TRIM2_THEORETICAL:   lcd.print("Trim 2 - Theory "); break;
    case TRIM2_HIGHEST:       lcd.print("Trim 2 - Highest"); break;
    case TRIM2_LOWEST:        lcd.print("Trim 2 - Lowest "); break;
    case TRIM3_MEAN:          lcd.print("Trim 3 - Mean   "); break;
    case TRIM3_THEORETICAL:   lcd.print("Trim 3 - Theory "); break;
    case TRIM3_HIGHEST:       lcd.print("Trim 3 - Highest"); break;
    case TRIM3_LOWEST:        lcd.print("Trim 3 - Lowest "); break;
    case THEORETICAL:         lcd.print("Theoretical     "); break;
    case LOW_BASED:           lcd.print("Low  (0 VDC)    "); break;
    case HIGH_BASED:          lcd.print("High (+5 VDC)   "); break;
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

//                      //
/* -- INPUT HANDLING -- */
//                      //

void menuModePressed() {
  menuMode ++;
  if (menuMode > MENU_END) menuMode = MENU_BEGIN;
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
    case MENU_EXIT: inMenu = false; break;
  }
}
