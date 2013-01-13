/*
Handles the LED
*/

//State of the LED
#define LED_OFF 0
#define LED_IN_GEAR 1
#define LED_NEUTRAL 2

#define LED_BEGIN 0
#define LED_END 2

void setLED(boolean engaged) {
  if (led == LED_IN_GEAR && engaged) digitalWrite(ledPin, HIGH);
  else if (led == LED_NEUTRAL && !engaged) digitalWrite(ledPin, HIGH);
  else digitalWrite(ledPin, LOW);
}
