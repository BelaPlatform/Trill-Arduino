#include <Trill.h>

Trill slider;
boolean touchActive = false;

void setup() {
  // Initialise serial and touch sensor
  Serial.begin(115200);
  int ret = slider.begin(Trill::TRILL_BAR);
  if(ret != 0) {
    Serial.println("failed to initialise slider");  
    Serial.println("Error code: ");
    Serial.println(ret);
    Serial.println("\n");
  }
}

void loop() {
  // Read 20 times per second
  delay(50);
  slider.read();
  
  if(slider.getNumTouches() > 0) {
    for(int i = 0; i < slider.getNumTouches(); i++) {
        Serial.print(slider.touchLocation(i));
        Serial.print(" ");
        Serial.print(slider.touchSize(i));
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
