/*
The Log Submenu can be entered from the Menu System
It provided details accumulated over time about shifting habits, etc.
*/

//Screens in the Log Submenu
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

#define LOG_BEGIN 0
#define LOG_END 15

void logModePressed() {
  logMode ++;
  if (logMode > LOG_END) logMode = LOG_BEGIN;
}

void logUpPressed() {
  if (inLog && logMode == LOG_EXIT) {
    inLog = false;
  }
}

void logDownPressed() {
  if (inLog && logMode == LOG_EXIT) {
    inLog = false;
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
  lcd.write((byte)ONE_COLON);
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
  lcd.write((byte)FOUR_COLON);
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
  lcd.write((byte)ONE_COLON);
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
  lcd.write((byte)FOUR_COLON);
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
  lcd.write((byte)ONE_COLON);
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
  lcd.write((byte)FOUR_COLON);
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