
#include <Wire.h>
#include <Trill.h>

Trill slider(24);
boolean touchActive = false;

void setup() {
  // Initialise serial and touch sensor
  Serial.begin(115200);
  
  int deviceType = slider.begin();
  
  if(!deviceType)
    Serial.println("failed to initialise slider");  
else if(deviceType != TRILL_DEVICE_2D)
    Serial.println("attached device is not a 2D slider");
    
  slider.setMode(TRILL_MODE_NORMAL);
}

void loop() {  
  // Read 20 times per second
  delay(50);
  slider.read();
  
  if(slider.numberOfTouches() > 0) {
    Serial.print(slider.numberOfTouches());
    Serial.print(" ");
    Serial.print(slider.numberOfHorizontalTouches());
    Serial.print(" ");
    
    for(int i = 0; i < slider.numberOfTouches(); i++) {
        Serial.print(slider.touchLocation(i));
        Serial.print(" ");
        Serial.print(slider.touchSize(i));
        Serial.print(" ");
    }
    for(int i = 0; i < slider.numberOfHorizontalTouches(); i++) {
        Serial.print(slider.touchHorizontalLocation(i));
        Serial.print(" ");
        Serial.print(slider.touchHorizontalSize(i));
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
