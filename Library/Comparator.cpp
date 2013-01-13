/*
Manual Transmission Gear Display
Ehryk Menze
*/

#include <Arduino.h>
#include <Comparator.h>

Comparator::Comparator()
{
	analogPins = [ A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15 ];
}

Comparator::Comparator();

//Declare Value Variables
//Values will range between 0 and 1023, (0V and 5V respectively)
//With a Resolution of 4.9mV / unit
int gear1value = 0;
int gear2value = 0;
int gear3value = 0;
int gear4value = 0;
int gear5value = 0;
int gearRvalue = 0;
//This is what should be read by the ideal Hall Effect Sensor
//Probably useless in this application
int theoretical = 511;
int numberActive = -2;
int mode = 0;
boolean inGear = false;
unsigned long debugRefresh = 0;
//How often to refresh the serial port, in milliseconds
int refreshInterval = 1000;
boolean eepromUpdateNeeded = false;
unsigned long eepromChange = 0;
int eepromMode = 0;
int eepromTolerance = 1;
int eepromMethod = 2;
int gear = -2;
int total = 0;
int average = 0;

//Parameters
boolean debug = true;
int baud = 9600;
int tolerance = 200;

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(baud);
  
  //Set pin modes
  pinMode(gear1pin, INPUT);
  pinMode(gear2pin, INPUT);
  pinMode(gear3pin, INPUT);
  pinMode(gear4pin, INPUT);
  pinMode(gear5pin, INPUT);
  pinMode(gearRpin, INPUT);
  pinMode(modePin, INPUT);
  pinMode(toleranceUpPin, INPUT);
  pinMode(toleranceDownPin, INPUT);
  
  //Use internal pull-up resistors
  digitalWrite(modePin, HIGH);
  digitalWrite(toleranceUpPin, HIGH);
  digitalWrite(toleranceDownPin, HIGH);
  
  //Set up LCD
  lcd.begin(16, 2);
  lcd.createChar(0, sigma);
  lcd.createChar(1, oneColon);
  lcd.createChar(4, fourColon);
  lcd.createChar(2, tColon);
  
  writeCredits();
  delay(1500);
  lcd.clear();
}

boolean checkPress(int pin) {
  return digitalRead(pin) == LOW && debounce(pin, LOW, 20, 6);
}

void loop() {
  // put your main code here, to run repeatedly:
  //Check for Mode Press
  if (checkPress(modePin)) {
    mode++;
    if (mode > 8) mode = 0;
    delay(100);
    lcd.clear();
  }
  else if (checkPress(toleranceUpPin) && tolerance < 995) tolerance += 5;
  else if (checkPress(toleranceDownPin) && tolerance > 0) tolerance -= 5;
  
  //Update the value variables
  readValues();
  total = getTotal();
  average = getMean(total);
  numberActive = countActive();
  gear = activeGear();
  inGear = gear > 0;
  
  //This rereads in the case of an error or fault
  //(doesn't display errors, leaves last displayed values)
  if (!debug && gear < 0) return;
  
  if (debug && millis() - debugRefresh > refreshInterval) {
    Serial.print("Mode: ");
    Serial.println(mode);
    
    Serial.print("Total: ");
    Serial.println(total);
    
    Serial.print("Average: ");
    Serial.print(average);
    Serial.print(" (");
    Serial.print(formatValue(average));
    Serial.print(") ");
    Serial.print(toVoltage(average));
    Serial.println("V");
    
    Serial.print("Tolerance: ");
    Serial.print(tolerance);
    Serial.print(" ");
    Serial.print(toVoltage(tolerance));
    Serial.println("V");
    
    Serial.print("In Gear: ** ");
    Serial.print(gearName(gear));
    Serial.print(" (");
    Serial.print(gearChar(gear));
    Serial.println(") **");
    
    if (inGear) {
      Serial.print(" - Active Gear: ");
      Serial.print(activeValue());
      Serial.print(" ");
      Serial.print(activeVoltage());
      Serial.println("V");
      Serial.print(" - Differential: ");
      Serial.println(abs(activeValue() - average));
    }
    else {
      Serial.println(" - Active Gear: N/A");
      Serial.println(" - Differential: N/A");
    }
    
    Serial.print("Gear 1: ");
    Serial.print(gear1value);
    Serial.print(" (");
    Serial.print(formatValue(gear1value));
    Serial.print(") ");
    Serial.print(toVoltage(gear1value));
    Serial.println("V");
    
    Serial.print("Gear 2: ");
    Serial.print(gear2value);
    Serial.print(" (");
    Serial.print(formatValue(gear2value));
    Serial.print(") ");
    Serial.print(toVoltage(gear2value));
    Serial.println("V");
    
    Serial.print("Gear 3: ");
    Serial.print(gear3value);
    Serial.print(" (");
    Serial.print(formatValue(gear3value));
    Serial.print(") ");
    Serial.print(toVoltage(gear3value));
    Serial.println("V");
    
    Serial.print("Gear 4: ");
    Serial.print(gear4value);
    Serial.print(" (");
    Serial.print(formatValue(gear4value));
    Serial.print(") ");
    Serial.print(toVoltage(gear4value));
    Serial.println("V");
    
    Serial.print("Gear 5: ");
    Serial.print(gear5value);
    Serial.print(" (");
    Serial.print(formatValue(gear5value));
    Serial.print(") ");
    Serial.print(toVoltage(gear5value));
    Serial.println("V");
    
    Serial.print("Gear R: ");
    Serial.print(gearRvalue);
    Serial.print(" (");
    Serial.print(formatValue(gearRvalue));
    Serial.print(") ");
    Serial.print(toVoltage(gearRvalue));
    Serial.println("V");
    
    Serial.print("Standard Deviation: ");
    Serial.println(getStandardDeviation(average));
    
    Serial.println();
    
    debugRefresh = millis();
  }
  
  //Update the display
  updateDisplay(mode, gear);
}

void readValues(){
  gear1value = readHallEffect(gear1pin);
  gear2value = readHallEffect(gear2pin);
  gear3value = readHallEffect(gear3pin);
  gear4value = readHallEffect(gear4pin);
  gear5value = readHallEffect(gear5pin);
  gearRvalue = readHallEffect(gearRpin);
}

int getTotal() {
  return gear1value + gear2value + gear3value + gear4value + gear5value + gearRvalue;
}

int getMean() {
  return gear1value + gear2value + gear3value + gear4value + gear5value + gearRvalue / 6;
}

int getTrimmed(int method) {
  int baseline = 0;
  if (method == MEAN) {
    baseline = getMean();
  }
  else if (method == THEORETICAL) {
    baseline = theoretical;
  }
  int differentials[6] = [abs(gear1value - baseline), abs(gear2value - baseline), abs(gear3value - baseline), abs(gear4value - baseline), abs(gear5value - baseline), abs(gearRvalue - baseline)];
  return total / 6;
}

float getStandardDeviation(int average) {
  float variance = 0;
  variance += pow(toVoltage(gear1value - average), 2);
  variance += pow(toVoltage(gear2value - average), 2);
  variance += pow(toVoltage(gear3value - average), 2);
  variance += pow(toVoltage(gear4value - average), 2);
  variance += pow(toVoltage(gear5value - average), 2);
  variance += pow(toVoltage(gearRvalue - average), 2);
  variance = variance / 6;
  return sqrt(variance);
}

float toVoltage(int value) {
  return value * 5.0 / 1023.0;
}

int readHallEffect(int pin) {
  return analogRead(pin);
}

int countActive() {
  //Return the number of pins above tolerance
  int count = 0;
  
  if (active(gear1value)) count ++;
  if (active(gear2value)) count ++;
  if (active(gear3value)) count ++;
  if (active(gear4value)) count ++;
  if (active(gear5value)) count ++;
  if (active(gearRvalue)) count ++;
  
  return count;
}

int activePin() {
  //Return the pin above tolerance
  //Return -1 if more than one active
  
  if (numberActive == 0) return 0;
  if (numberActive >= 2) return -1;
  
  if (active(gear1value)) return gear1pin;
  if (active(gear2value)) return gear2pin;
  if (active(gear3value)) return gear3pin;
  if (active(gear4value)) return gear4pin;
  if (active(gear5value)) return gear5pin;
  if (active(gearRvalue)) return gearRpin;
  
  return -2;
}

int activeGear() {
  //Return the pin above tolerance
  //Return -1 if more than one active
  
  if (numberActive == 0) return 0;
  if (numberActive >= 2) return -1;
  
  if (active(gear1value)) return 1;
  if (active(gear2value)) return 2;
  if (active(gear3value)) return 3;
  if (active(gear4value)) return 4;
  if (active(gear5value)) return 5;
  if (active(gearRvalue)) return 6;
  
  return -2;
}

int activeValue() {
  //Return the pin above tolerance
  //Return -1 if more than one active
  
  if (numberActive == 0) return 0;
  if (numberActive >= 2) return -1;
  
  if (active(gear1value)) return gear1value;
  if (active(gear2value)) return gear2value;
  if (active(gear3value)) return gear3value;
  if (active(gear4value)) return gear4value;
  if (active(gear5value)) return gear5value;
  if (active(gearRvalue)) return gearRvalue;
  
  return -2;
}

float activeVoltage() {
  return toVoltage(activeValue());
}

boolean active(int value) {
  int deviation = abs(value - average);
  return deviation > tolerance;
}

//Returns the Name of a Gear
char* gearName(int g) {
  switch (g) {
    case -2: return "Fault  ";
    case -1: return "Error  ";
    case 0:  return "Neutral";
    case 1:  return "First  ";
    case 2:  return "Second ";
    case 3:  return "Third  ";
    case 4:  return "Fourth ";
    case 5:  return "Fifth  ";
    case 6:  return "Reverse";
  }
  return "Fault";
}

//Returns the Letter/Digit of a Gear
char gearChar(int g) {
  switch (g) {
    case -2: return 'F';
    case -1: return 'E';
    case 0:  return 'N';
    case 1:  return '1';
    case 2:  return '2';
    case 3:  return '3';
    case 4:  return '4';
    case 5:  return '5';
    case 6:  return 'R';
  }
  return 'F';
}

void updateDisplay(int mode, int gear) {
  switch(mode) {
    case 0: writeBasic(gear); break;
    case 1: writeAdvanced(gear); break;
    case 2: writeVariables(gear, true); break;
    case 3: writeVariables(gear, false); break;
    case 4: writeValues(); break;
    case 5: writeVoltages(); break;
    case 6: writeFill(gearChar(gear)); break;
    case 7: writeCredits(); break;
    case 8: break;
  }
}

void writeBasic(int g) {
  //Print Gear Char
  lcd.setCursor(1, 0);
  lcd.print('.');
  lcd.print(gearChar(g));
  lcd.print('.');
  
  lcd.setCursor(9, 0);
  lcd.print(" Gear");
  
  //Print Gear Name
  lcd.setCursor(0, 1);
  lcd.print(gearName(g));
  
  lcd.setCursor(9, 1);
  lcd.print("Display");
}

void writeAdvanced(int g) {
  //Print Gear Char
  lcd.setCursor(2, 0);
  lcd.print(gearChar(g));
  
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
    lcd.print(toVoltage(abs(activeValue() - average)));
    lcd.print("V");
  }
  else {
    lcd.print("A:");
    lcd.print(toVoltage(average));
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
  lcd.write((byte)0);
  lcd.print(":");
  lcd.print(getStandardDeviation(average), 2);
  
  //Print Tolerance
  lcd.setCursor(0, 1);
  if (voltage) {
    lcd.write((byte)2);
    lcd.print(toVoltage(tolerance), 2);
    lcd.print("V");
  }
  else {
    lcd.print("T:");
    lcd.print(padLeft(tolerance, 3));
  }
  
  //Print Debug
  lcd.setCursor(7, 1);
  if (debug) lcd.print("D");
  
  //Print Average Voltage
  lcd.setCursor(9, 1);
  lcd.print("A:");
  lcd.print(toVoltage(average), 2);
  lcd.print("V");
}

void writeValues() {
  lcd.setCursor(0, 0);
  lcd.write((byte)1);
  lcd.print(formatValue(gear1value));
  lcd.print(" ");
  
  lcd.print("2:");
  lcd.print(formatValue(gear2value));
  lcd.print(" ");
  
  lcd.print("3:");
  lcd.print(formatValue(gear3value));
  
  lcd.setCursor(0, 1);
  lcd.write((byte)4);
  lcd.print(formatValue(gear4value));
  lcd.print(" ");
  
  lcd.print("5:");
  lcd.print(formatValue(gear5value));
  lcd.print(" ");
  
  lcd.print("R:");
  lcd.print(formatValue(gearRvalue));
}

void writeVoltages() {
  lcd.setCursor(0, 0);
  lcd.print(toVoltage(gear1value), 2);
  lcd.print("V ");
  
  lcd.print(toVoltage(gear2value), 2);
  lcd.print("V ");
  
  lcd.print(toVoltage(gear3value), 2);
  
  lcd.setCursor(0, 1);
  lcd.print(toVoltage(gear4value), 2);
  lcd.print("V ");
  
  lcd.print(toVoltage(gear5value), 2);
  lcd.print("V ");
  
  lcd.print(toVoltage(gearRvalue), 2);
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

boolean debounce(int pin, int value, int timeDelay, int bounceCount) {
  int toCheck = bounceCount;
  while (toCheck > 0) {
    delay(timeDelay);
    if (digitalRead(pin) != value) return false;
    toCheck --;
  }
  return true;
}

String formatValue(int value) {
  if (value < 1000) return padLeft(value, 3);
  String n = String("+");
  n += String(padLeft(value % 1000, 2));
  return n;
}

String padLeft(int number, int padding) {
  String n = String(number);
  int currentMax = 10;
  for (int i=1; i<padding; i++){
    if (number < currentMax) {
      n = "0" + n;
    }
    currentMax *= 10;
  }
  return n;
}
