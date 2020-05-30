#include <Trill.h>

Trill trillSensor;
boolean touchActive = false;

void setup() {
  // Initialise serial and touch sensor
  Serial.begin(115200);
  int ret = trillSensor.begin(Trill::TRILL_BAR);
  if(ret != 0) {
    Serial.println("failed to initialise trillSensor");
    Serial.println("Error code: ");
    Serial.println(ret);
    Serial.println("\n");
  }
}

void loop() {
  // Read 20 times per second
  delay(50);
  trillSensor.read();
  
  if(trillSensor.getNumTouches() > 0) {
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
    Serial.println("0 0");   
    touchActive = false; 
  }
}
