#include <Trill.h>

Trill trillSensor;
boolean touchActive = false;
int prevButtonState[2] = { 0 , 0 };

void setup() {
  // Initialise serial and touch sensor
  Serial.begin(115200);
  int ret = trillSensor.begin(Trill::TRILL_RING);
  if(ret != 0) {
    Serial.println("failed to initialise trillSensor");  
    Serial.print("Error code: ");
    Serial.println(ret);
  }
}

void loop() {
  // Read 20 times per second
  delay(50);
  trillSensor.read();
  
  if(trillSensor.getNumTouches() > 0) {
    Serial.print("T");
    Serial.print(" ");
    for(int i = 0; i < trillSensor.getNumTouches(); i++) {
        Serial.print(trillSensor.touchLocation(i));
        Serial.print(" ");
        Serial.print(trillSensor.touchSize(i));
        Serial.print(" ");
    }
    Serial.println("");
    touchActive = true;
  }
  else if(touchActive) {
    // Print a single line when touch goes off
    Serial.print("T");
    Serial.print(" ");
    Serial.println("0 0");   
    touchActive = false; 
  }

  for(int b = 0; b < trillSensor.getNumButtons(); b++) {
	int buttonState = trillSensor.getButtonValue(b);
	if(buttonState != prevButtonState[b]) {
            Serial.print("B");
            Serial.print(" ");
            Serial.print(b);
            Serial.print(" ");
            Serial.print(buttonState);
            Serial.print(" ");
            Serial.println("");
            prevButtonState[b] = buttonState;
        }
  }
  
}
