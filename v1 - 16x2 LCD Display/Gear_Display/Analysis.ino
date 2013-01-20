
//Methods of computation baselines
#define MEAN_BASED 0
#define TRIM1_MEAN 1
#define TRIM1_THEORETICAL 2
#define TRIM1_HIGHEST 3
#define TRIM1_LOWEST 4
#define TRIM2_MEAN 5
#define TRIM2_THEORETICAL 6
#define TRIM2_HIGHEST 7
#define TRIM2_LOWEST 8
#define TRIM3_MEAN 9
#define TRIM3_THEORETICAL 10
#define TRIM3_HIGHEST 11
#define TRIM3_LOWEST 12
#define THEORETICAL 13
#define LOW_BASED 14
#define HIGH_BASED 15

#define METHOD_BEGIN 0
#define METHOD_END 15

void readValues(){
  vcc = readVcc();
  for (int g = 0; g < gears; g++) {
    values[g] = readHallEffect(gearPin[g]);
  }
}

int readHallEffect(int pin) {
  return analogRead(pin);
}

int computeBaseline(int m) {
  if (m == MEAN_BASED) return getMean();
  else if (m == TRIM1_MEAN) return getTrimmed(getMean(), 1);
  else if (m == TRIM1_THEORETICAL) return getTrimmed(theoretical, 1);
  else if (m == TRIM1_HIGHEST) return getTrimmed(0, 1);
  else if (m == TRIM1_LOWEST) return getTrimmed(1023, 1);
  else if (m == TRIM2_MEAN) return getTrimmed(getMean(), 2);
  else if (m == TRIM2_THEORETICAL) return getTrimmed(theoretical, 2);
  else if (m == TRIM2_HIGHEST) return getTrimmed(0, 2);
  else if (m == TRIM2_LOWEST) return getTrimmed(1023, 2);
  else if (m == TRIM3_MEAN) return getTrimmed(getMean(), 3);
  else if (m == TRIM3_THEORETICAL) return getTrimmed(theoretical, 3);
  else if (m == TRIM3_HIGHEST) return getTrimmed(0, 3);
  else if (m == TRIM3_LOWEST) return getTrimmed(1023, 3);
  else if (m == THEORETICAL) return theoretical;
  else if (m == LOW_BASED) return 0;
  else if (m == HIGH_BASED) return 1023;
  
  return -1;
}

int getMean() {
  int total = 0;
  for (int g = 0; g < gears; g++) {
    total += values[g];
  }
  return total / gears;
}

int getTrimmed(int subline, int toTrim) {
  if (toTrim >= gears) return subline;
  
  //Find which gear gets trimmed
  //(Furthest off the subline)
  boolean trim[gears];
  for(int init = 0; init < gears; init++) trim[init] = false;
  for (int i = 0; i < toTrim; i++) {
    int maximum = 0;
    int maximumValue = 0;
    for (int g = 0; g < gears; g++) {
      int differential = abs(values[g] - subline);
      if (!trim[g] && differential > maximumValue) {
        maximumValue = differential;
        maximum = g;
      }
    }
    trim[maximum] = true;
  }
  
  int trimmedTotal = 0;
  for (int g = 0; g < gears; g++) {
    if (!trim[g]) trimmedTotal += values[g];
  }
  
  return trimmedTotal / (gears - toTrim);
}

float getStandardDeviation(int b) {
  float variance = 0;
  for (int g = 0; g < gears; g++) {
    variance += pow(toVoltage(values[g] - b), 2);
  }
  variance = variance / gears;
  return sqrt(variance);
}

int countActive() {
  //Return the number of pins above tolerance
  int count = 0;
  
  for (int g = 0; g < gears; g++) {
    if (active(values[g])) count++;
  }
  
  return count;
}

int activePin() {
  //Return the pin above tolerance
  //Return -1 if more than one active
  
  if (numberActive == 0) return 0;
  if (numberActive >= 2) return -1;
  
  for (int g = 0; g < gears; g++) {
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
  
  for (int g = 0; g < gears; g++) {
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
  
  for (int g = 0; g < gears; g++) {
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
