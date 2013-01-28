/*
This controls what happens on User Input (pressing buttons)
*/

//Checks the status of the three buttons; Mode, Up and Down
//and responds accordingly, passing off the press to the appropriate area
void handleButtons() {
  //Check for Mode Press
  if (checkPress(MODE_PIN)) {
    if (inMenu) menuModePressed();
    else if (inLog) logModePressed();
    else modePressed();
    lcd.clear();
  }
  else if (checkPress(UP_PIN)) {
    if (inMenu) menuUpPressed();
    else if (inLog) logUpPressed();
    else upPressed();
  }
  else if (checkPress(DOWN_PIN)) {
    if (inMenu) menuDownPressed();
    else if (inLog) logDownPressed();
    else downPressed();
  }
}

//Checks if a button is pressed
boolean checkPress(int pin) {
  return digitalRead(pin) == LOW && debounce(pin, LOW, 20, 5);
}

//Software Debounce of a swtich
boolean debounce(int pin, int v, int timeDelay, int bounceCount) {
  int toCheck = bounceCount;
  while (toCheck > 0) {
    delay(timeDelay);
    if (digitalRead(pin) != v) return false;
    toCheck --;
  }
  return true;
}
