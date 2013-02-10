
//Methods of computation baselines
#define METHOD_MEAN_BASED 0
#define METHOD_TRIM1_MEAN 1
#define METHOD_TRIM1_THEORETICAL 2
#define METHOD_TRIM1_HIGHEST 3
#define METHOD_TRIM1_LOWEST 4
#define METHOD_TRIM2_MEAN 5
#define METHOD_TRIM2_THEORETICAL 6
#define METHOD_TRIM2_HIGHEST 7
#define METHOD_TRIM2_LOWEST 8
#define METHOD_TRIM3_MEAN 9
#define METHOD_TRIM3_THEORETICAL 10
#define METHOD_TRIM3_HIGHEST 11
#define METHOD_TRIM3_LOWEST 12
#define METHOD_THEORETICAL 13
#define METHOD_LOW_BASED 14
#define METHOD_HIGH_BASED 15

#define METHOD_BEGIN 0
#define METHOD_END 15

//int fineAdjust[GEARS] = {0, 0, 0, 0, 0, 0}; //No Fine Tuning
//int fineAdjust[GEARS] = {-5, 5, 4, -5, 4, -8}; //USB
int fineAdjust[GEARS] = {3, 5, 0, 0, -3, 0}; //In Shield, AH Sensors

void readValues(){
  vcc = readVcc();
  theoretical = vcc * 1023.0 / 1000.0 / 2.0;
  for (int g = 0; g < GEARS; g++) {
    values[g] = readHallEffect(gearPin[g]);
  }
}

int readHallEffect(int pin) {
  analogRead(pin); //Switch mux to pin
  delay(2); //Allow voltage to settle
  return analogRead(pin) + fineAdjust[pin];
}

int computeBaseline(int m) {
  switch (m) {
    case(METHOD_MEAN_BASED): return getMean();
    case(METHOD_TRIM1_MEAN): return getTrimmed(getMean(), 1);
    case(METHOD_TRIM1_THEORETICAL): return getTrimmed(theoretical, 1);
    case(METHOD_TRIM1_HIGHEST): return getTrimmed(0, 1);
    case(METHOD_TRIM1_LOWEST): return getTrimmed(1023, 1);
    case(METHOD_TRIM2_MEAN): return getTrimmed(getMean(), 2);
    case(METHOD_TRIM2_THEORETICAL): return getTrimmed(theoretical, 2);
    case(METHOD_TRIM2_HIGHEST): return getTrimmed(0, 2);
    case(METHOD_TRIM2_LOWEST): return getTrimmed(1023, 2);
    case(METHOD_TRIM3_MEAN): return getTrimmed(getMean(), 3);
    case(METHOD_TRIM3_THEORETICAL): return getTrimmed(theoretical, 3);
    case(METHOD_TRIM3_HIGHEST): return getTrimmed(0, 3);
    case(METHOD_TRIM3_LOWEST): return getTrimmed(1023, 3);
    case(METHOD_THEORETICAL): return theoretical;
    case(METHOD_LOW_BASED): return 0;
    case(METHOD_HIGH_BASED): return 1023;
    
    default: return -1;
  }
}

char methodChar(int m) {
  switch (m) {
    case (METHOD_MEAN_BASED): return 'A';
    case (METHOD_THEORETICAL): return 'Y';
    case (METHOD_LOW_BASED): return 'L';
    case (METHOD_HIGH_BASED): return 'H';
    default: return 'B';
  }
}

void writeMethodChar(int m) {
  lcd.print(methodChar(m));
  lcd.print(':');
}

int getMean() {
  int total = 0;
  for (int g = 0; g < GEARS; g++) {
    total += values[g];
  }
  return total / GEARS;
}

int getTrimmed(int subline, int toTrim) {
  if (toTrim >= GEARS) return subline;
  
  //Find which gear gets trimmed
  //(Furthest off the subline)
  boolean trim[GEARS];
  for(int init = 0; init < GEARS; init++) trim[init] = false;
  for (int i = 0; i < toTrim; i++) {
    int maximum = 0;
    int maximumValue = 0;
    for (int g = 0; g < GEARS; g++) {
      int differential = abs(values[g] - subline);
      if (!trim[g] && differential > maximumValue) {
        maximumValue = differential;
        maximum = g;
      }
    }
    trim[maximum] = true;
  }
  
  int trimmedTotal = 0;
  for (int g = 0; g < GEARS; g++) {
    if (!trim[g]) trimmedTotal += values[g];
  }
  
  return trimmedTotal / (GEARS - toTrim);
}

float getStandardDeviation(int b) {
  float variance = 0;
  for (int g = 0; g < GEARS; g++) {
    variance += pow(toVoltage(values[g] - b), 2);
  }
  variance = variance / GEARS;
  return sqrt(variance);
}

int countActive() {
  //Return the number of pins above tolerance
  int count = 0;
  
  for (int g = 0; g < GEARS; g++) {
    if (active(values[g])) count++;
  }
  
  return count;
}

int activePin() {
  //Return the pin above tolerance
  //Return -1 if more than one active
  
  if (numberActive == 0) return 0;
  if (numberActive >= 2) return -1;
  
  for (int g = 0; g < GEARS; g++) {
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
  
  for (int g = 0; g < GEARS; g++) {
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
  
  for (int g = 0; g < GEARS; g++) {
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
