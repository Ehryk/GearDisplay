/*
Debug Methods for variable inspection
*/

void writeDebug(unsigned long loopTime) {
  Serial.print("Mode: ");
  Serial.print(mode);
  Serial.print(" (Loop Time: ");
  Serial.print(loopTime);
  Serial.println("ms)");
  
  Serial.print("Baseline: ");
  Serial.print(baseline);
  Serial.print(" (");
  Serial.print(formatValue(baseline));
  Serial.print(") ");
  Serial.print(toVoltage(baseline));
  Serial.println("V");
  
  Serial.print("Tolerance: ");
  Serial.print(tolerance);
  Serial.print(" ");
  Serial.print(toVoltage(tolerance));
  Serial.println("V");
  
  Serial.print("Vcc: ");
  Serial.print(vcc);
  Serial.print("mV (");
  Serial.print(toVoltage(vcc));
  Serial.println("V)");
  
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
    Serial.println(abs(activeValue() - baseline));
  }
  else {
    Serial.println(" - Active Gear: N/A");
    Serial.println(" - Differential: N/A");
  }
  
  Serial.print("Gear 1: ");
  Serial.print(values[0]);
  Serial.print(" (");
  Serial.print(formatValue(values[0]));
  Serial.print(") ");
  Serial.print(toVoltage(values[0]));
  Serial.println("V");
  
  Serial.print("Gear 2: ");
  Serial.print(values[1]);
  Serial.print(" (");
  Serial.print(formatValue(values[1]));
  Serial.print(") ");
  Serial.print(toVoltage(values[1]));
  Serial.println("V");
  
  Serial.print("Gear 3: ");
  Serial.print(values[2]);
  Serial.print(" (");
  Serial.print(formatValue(values[2]));
  Serial.print(") ");
  Serial.print(toVoltage(values[2]));
  Serial.println("V");
  
  Serial.print("Gear 4: ");
  Serial.print(values[3]);
  Serial.print(" (");
  Serial.print(formatValue(values[3]));
  Serial.print(") ");
  Serial.print(toVoltage(values[3]));
  Serial.println("V");
  
  Serial.print("Gear 5: ");
  Serial.print(values[4]);
  Serial.print(" (");
  Serial.print(formatValue(values[4]));
  Serial.print(") ");
  Serial.print(toVoltage(values[4]));
  Serial.println("V");
  
  Serial.print("Gear R: ");
  Serial.print(values[5]);
  Serial.print(" (");
  Serial.print(formatValue(values[5]));
  Serial.print(") ");
  Serial.print(toVoltage(values[5]));
  Serial.println("V");
  
  Serial.print("Standard Deviation: ");
  Serial.println(getStandardDeviation(baseline));
  
  Serial.println();
  
  debugRefresh = millis();
}
