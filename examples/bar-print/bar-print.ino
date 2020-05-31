/*
 ____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\
http://bela.io

\example bar-print

Trill Bar Print
===============

This is an example of how to communicate with the Trill Bar
sensor using the Trill Arduino library.

The sensor is set to Centroid mode and touch location and size
printed to the serial port for each of the 5 different simultaneous
touches.

The accompanying Processing sketch, `TrillBarDisplay.pde`, listens for
touch information on the Arduino serial port* and displays it in a
render of a Trill Bar.

*NOTE: you may need to update the Processing port number (gPortNumber)
to match that of your Arduino.
*/

#include <Trill.h>

Trill trillSensor;
boolean touchActive = false;

void setup() {
  // Initialise serial and touch sensor
  Serial.begin(115200);
  int ret = trillSensor.begin(Trill::TRILL_BAR);
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
