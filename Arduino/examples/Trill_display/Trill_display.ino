#include <Wire.h>
#include <Trill.h>

Trill slider(24);
boolean touchActive = false;

void setup() {
  // Initialise serial and touch sensor
  Serial.begin(115200);
  
  if(!slider.begin())
    Serial.println("failed to initialise slider");  
  
  slider.setMode(TRILL_MODE_NORMAL);
}

void loop() {
  // Read 20 times per second
  delay(50);
  slider.read();
  
  if(slider.numberOfTouches() > 0) {
    for(int i = 0; i < slider.numberOfTouches(); i++) {
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
