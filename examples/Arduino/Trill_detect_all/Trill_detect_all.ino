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
