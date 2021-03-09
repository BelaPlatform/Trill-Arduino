/*
 ____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\
http://bela.io

\example detect-all-devices

Detect All I2C Devices
======================

This example is a handy utility which will identify all connected
I2C devices and print information on them to the console.

When the program runs it will print the address and sensor type
of all the Trill sensors you currently have connected to the I2C bus
on the serial port.

This is particularly useful if you are unsure of the address of the sensor after
changing it via the solder bridges on the back. This example will also give
you a total count of the amount of Trill sensors connected to Bela.

NOTE: as this example scans several addresses on the i2c bus
it could cause non-Trill peripherals connected to it to malfunction.
*/

#include <Trill.h>

void setup() {
   // Initialise serial
   Serial.begin(9600);
   Serial.println("Trill devices detected on I2C bus:");
   Serial.println("Address     |     Type");
   unsigned int total = 0;
   for(uint8_t n = 0x20; n <= 0x50; ++n) {
      Trill::Device device = Trill::probe(n);
      if(device != Trill::TRILL_NONE) {
         Serial.print('#');
         Serial.print(n, HEX);
         Serial.print(" (");
         Serial.print(n);
         Serial.print(")");
         Serial.print("          ");
         printDeviceType((int)device);
         Serial.println("");
         ++total;
      }
   }
 Serial.print("Total: ");
 Serial.println(total);
}

void printDeviceType(int deviceType) {
   switch(deviceType) {
    case Trill::TRILL_BAR:
      Serial.print("bar");
      break;
    case Trill::TRILL_SQUARE:
      Serial.print("square");
      break;
    case Trill::TRILL_HEX:
      Serial.print("hex");
      break;
    case Trill::TRILL_RING:
      Serial.print("ring");
      break;
    case Trill::TRILL_CRAFT:
      Serial.print("craft");
      break;
    case Trill::TRILL_FLEX:
      Serial.print("flex");
      break;
    case Trill::TRILL_UNKNOWN:
      Serial.print("unknown");
      break;
    case Trill::TRILL_NONE:
      Serial.print("none");
      break;
  }
}

void loop() {
}
