/*
 ____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\
http://bela.io

\example hex-print

Trill Hex Print
===============

This is an example of how to communicate with the Trill Hex
sensor using the Trill Arduino library.

The sensor is set to Centroid mode and 2D touch location and size
printed to the serial port.
*/

#include <Trill.h>

Trill trillSensor;
boolean touchActive = false;

void setup() {
  // Initialise serial and touch sensor
  Serial.begin(115200);
  int ret = trillSensor.setup(Trill::TRILL_HEX);
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
    Serial.print(trillSensor.getNumTouches());
    Serial.print(" ");
    Serial.print(trillSensor.getNumHorizontalTouches());
    Serial.print(" ");

    for(int i = 0; i < trillSensor.getNumTouches(); i++) {
        Serial.print(trillSensor.touchLocation(i));
        Serial.print(" ");
        Serial.print(trillSensor.touchSize(i));
        Serial.print(" ");
    }
    for(int i = 0; i < trillSensor.getNumHorizontalTouches(); i++) {
        Serial.print(trillSensor.touchHorizontalLocation(i));
        Serial.print(" ");
        Serial.print(trillSensor.touchHorizontalSize(i));
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
