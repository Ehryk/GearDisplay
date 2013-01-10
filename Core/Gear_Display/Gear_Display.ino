/*
Manual Transmission Gear Display
Ehryk Menze
*/

#include <LiquidCrystal.h>
#include <stdlib.h>

//Configure Pins
int gear1pin = A0;
int gear2pin = A1;
int gear3pin = A2;
int gear4pin = A3;
int gear5pin = A4;
int gearRpin = A5;
int modePin = 8;
int toleranceUpPin = 9;
int toleranceDownPin = 10;

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
  
  writeCredits();
  delay(1500);
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
  }
  else if (checkPress(toleranceUpPin) && tolerance < 995) tolerance += 5;
  else if (checkPress(toleranceDownPin) && tolerance > 0) tolerance -= 5;
  
  //Update the value variables
  readValues();
  total = getTotal();
  average = getAverage(total);
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
    Serial.print(" ");
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
    Serial.print(" ");
    Serial.print(toVoltage(gear1value));
    Serial.println("V");
    
    Serial.print("Gear 2: ");
    Serial.print(gear2value);
    Serial.print(" ");
    Serial.print(toVoltage(gear2value));
    Serial.println("V");
    
    Serial.print("Gear 3: ");
    Serial.print(gear3value);
    Serial.print(" ");
    Serial.print(toVoltage(gear3value));
    Serial.println("V");
    
    Serial.print("Gear 4: ");
    Serial.print(gear4value);
    Serial.print(" ");
    Serial.print(toVoltage(gear4value));
    Serial.println("V");
    
    Serial.print("Gear 5: ");
    Serial.print(gear5value);
    Serial.print(" ");
    Serial.print(toVoltage(gear5value));
    Serial.println("V");
    
    Serial.print("Gear R: ");
    Serial.print(gearRvalue);
    Serial.print(" ");
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

int getAverage(int total) {
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
  //Return the number of pins avove tolerance
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
}

void writeAdvanced(int g) {
}

void writeVariables(int g, boolean voltage) {
}

void writeValues() {
}

void writeVoltages() {
}

void writeFill(char c) {
}

void writeCredits() {
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
