

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
  return "Fault? ";
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
  return '?';
}

//Returns the Accent Char
char accentChar(int a) {
  return accents[a];
}

String formatValue(int v) {
  if (v < 1000) return padLeft(v, 3, "0");
  String n = String("+");
  n += String(padLeft(v % 1000, 2, "0"));
  return n;
}

String formatPercent(float v, int padding) {
  String n = String("");
  n += String(padLeft(v, padding, " "));
  return n + "%";
}

String formatShifts(int v, int padding) {
  String n = String("");
  n += String(padLeft(v, padding, " "));
  return n;
}

String formatTime(unsigned long v, int padding) {
  String n = String("");
  n += String(padLeft(v, padding, " "));
  return n;
}

String padLeft(int number, int padding, char* pad) {
  String n = String(number);
  int currentMax = 10;
  for (int i=1; i<padding; i++){
    if (number < currentMax) {
      n = pad + n;
    }
    currentMax *= 10;
  }
  return n;
}

float toVoltage(int value) {
  return value * 5.0 / 1023.0;
}
