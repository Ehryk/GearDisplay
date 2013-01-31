/*
Handles the LED
*/

//State of the LED
#define LED_OFF 0
#define LED_IN_GEAR 1
#define LED_NEUTRAL 2
#define LED_CITY 3
#define LED_HIGHWAY 4
#define LED_ERROR 5

#define LED_BEGIN 0
#define LED_END 5

void setLED(boolean engaged) {
  if (led == LED_IN_GEAR && engaged) digitalWrite(LED_PIN, HIGH);
  else if (led == LED_NEUTRAL && !engaged) digitalWrite(LED_PIN, HIGH);
  else if (led == LED_CITY && engaged && gear != 5) digitalWrite(LED_PIN, HIGH);
  else if (led == LED_HIGHWAY && engaged && gear == 5) digitalWrite(LED_PIN, HIGH);
  else if (led == LED_ERROR && gear < 0) digitalWrite(LED_PIN, HIGH);
  else digitalWrite(LED_PIN, LOW);
}
