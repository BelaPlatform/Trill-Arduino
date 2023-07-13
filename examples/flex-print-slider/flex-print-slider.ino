/*
 ____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\
http://bela.io

\example flex-print-slider

Trill Flex Print Slider
=======================

This is an example of how to communicate with the Trill Flex
sensor using the Trill Arduino library.

The Trill Flex sensor is read in Differential mode by default. This is
good for when you are working with custom designed Flex PCBs.
See the `flex-print-raw` example to read each sensor channel independently.

In this example we will use the sensor in Centroid mode to see
touch location and size of touches on the default Flexible Bar sensor.
Touchese will be printed to the serial port for each of the 5 different
simultaneous touches.

The accompanying Processing sketch, `TrillFlexSlider.pde`, listens for
touch information on the Arduino serial port* and displays it in a
render of a Trill Flex.

*NOTE: you may need to update the Processing port number (gPortNumber)
to match that of your Arduino.
*/

#include <Trill.h>

Trill trillSensor;
boolean touchActive = false;

void setup() {
  // Initialise serial and touch sensor
  Serial.begin(115200);
  int ret = trillSensor.setup(Trill::TRILL_FLEX);
  if(ret != 0) {
    Serial.println("failed to initialise trillSensor");
    Serial.print("Error code: ");
    Serial.println(ret);
  }
  // Set the sensor into Centroid mode
  // The default for Trill Flex is Differential mode
  trillSensor.setMode(Trill::CENTROID);
  delay(10);
  // when the slider is connected we increase the
  // prescaler to deal with the increased baseline
  // capacitance it brings
  trillSensor.setPrescaler(4);
  delay(10);
  // after any prescaler change, it's always good to update
  // the baseline, too.
  trillSensor.updateBaseline();
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
