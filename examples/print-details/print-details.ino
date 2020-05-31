/*
 ____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\
http://bela.io

\example print-details

Trill Print Details
===================

This is an example of how to communicate with the Trill Craft
sensor using the Trill Arduino library.

On setup(), all details from the sensor (including I2C address
type, mode, number of capacitive contacts, etc) are printed to
serial.

Although this example works for Trill Craft and its default
address out of the box, it can be used with any other Trill
sensor by changing the parameters of the Trill.begin()
method.
*/

#include <Trill.h>

Trill trill;
boolean touchActive = false;

void setup() {
  // Initialise serial
  Serial.begin(9600);


  // Initialise Trill sensor
  if(int ret = trill.begin(Trill::TRILL_CRAFT) != 0) {
    Serial.println("Failed to initialise Trill device");
    Serial.print("Error code: ");
    Serial.println(ret);

  } else {

  Serial.println("Trill Device Details: ");
  int address = trill.getAddress();
  Serial.print("\t- I2C address: ");
  Serial.print("#");
  Serial.print(address, HEX);
  Serial.print(" (");
  Serial.print(address);
  Serial.println(")");

  int deviceType = trill.deviceType();
  Serial.print("\t- Trill device type: ");
  switch(deviceType) {
    case Trill::TRILL_BAR:
      Serial.println("bar");
      break;
    case Trill::TRILL_SQUARE:
      Serial.println("square");
      break;
    case Trill::TRILL_HEX:
      Serial.println("hex");
      break;
    case Trill::TRILL_RING:
      Serial.println("ring");
      break;
    case Trill::TRILL_CRAFT:
      Serial.println("craft");
      break;
    case Trill::TRILL_UNKNOWN:
      Serial.println("unknown");
      break;
    case Trill::TRILL_NONE:
      Serial.println("none");
      break;
  }
    int firmwareRev = trill.firmwareVersion();
    Serial.print("\t- Firmware version: ");
    Serial.println(firmwareRev);

    int mode = trill.getMode();
    Serial.print("\t- Sensor mode: ");
    switch(mode) {
      case Trill::CENTROID:
        Serial.println("centroid");
        break;
      case Trill::RAW:
        Serial.println("raw");
        break;
      case Trill::BASELINE:
        Serial.println("baseline");
        break;
      case Trill::DIFF:
        Serial.println("differential");
        break;
      case Trill::AUTO:
        Serial.println("auto");
        break;
    }

    Serial.print("\t- Number of available centroid dimensions: ");
    if(trill.is1D()) {
      Serial.println(1);
    } else if(trill.is2D()) {
      Serial.println(2);
    } else {
      Serial.println(0);
    }

    int numChannels = trill.getNumChannels();
    Serial.print("\t- Number of capacitive channels: ");
    Serial.println(numChannels);

    int numButtons = trill.getNumButtons();
    Serial.print("\t- Number of button channels: ");
    Serial.println(numButtons);
  }
  Serial.println();
}

void loop() {
}
