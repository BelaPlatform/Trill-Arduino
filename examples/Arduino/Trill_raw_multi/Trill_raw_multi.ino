
#include <Trill.h>

/*
This is a example of using multiple trill sensors with a single Arduino board.
The I2C pins of all the sensors are connected together (SCL with SCL, SDA with SDA).
Each sensor has a unique address. Here is the table of address for each sensor type,
the first column is the default address:

| Type:  | Addresses (hex)                              |
|--------|----------------------------------------------|
| BAR    | 0x20 0x21 0x22 0x23 0x24 0x25 0x26 0x27 0x28 |
| SQUARE | 0x28 0x29 0x2A 0x2B 0x2C 0x2D 0x2E 0x2F 0x30 |
| CRAFT  | 0x30 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 |
| RING   | 0x38 0x39 0x3A 0x3B 0x3C 0x3D 0x3E 0x3F 0x40 |
| HEX    | 0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 |

In this example the sensor readings are printed to the console as raw values using the 
`rawDataRead()` method.

*/

Trill craft(0x30); // for Trill Craft
Trill squareTrill(0x28); // for Trill squareTrill
Trill bar(0x20); // for Trill Bar
Trill hex(0x40); // for Trill Hex



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  if(!craft.begin())
    Serial.println("failed to initialise craft");

  if(!squareTrill.begin())
    Serial.println("failed to initialise squareTrill");

  if(!bar.begin())
    Serial.println("failed to initialise bar");

  if(!hex.begin())
    Serial.println("failed to initialise hex");
   
  craft.setMode(Trill::DIFF);
  squareTrill.setMode(Trill::DIFF);
  bar.setMode(Trill::DIFF);
  hex.setMode(Trill::DIFF);
}

void loop() {
  // put your main code here, to run repeatedly:

  delay(100);
  craft.requestRawData();
  squareTrill.requestRawData();
  bar.requestRawData();
  hex.requestRawData();
   
  if(craft.rawDataAvailable() > 0) {
    Serial.print("CRAFT ");
    while(craft.rawDataAvailable() > 0) {
      int data = craft.rawDataRead();
      Serial.print(data);
      Serial.print(" ");
    }
    Serial.println("");
  }

  if(squareTrill.rawDataAvailable() > 0) {
    Serial.print("SQUARE ");
    while(squareTrill.rawDataAvailable() > 0) {
      int data = squareTrill.rawDataRead();
      Serial.print(data);
      Serial.print(" ");
    }
    Serial.println("");
  }

  if(bar.rawDataAvailable() > 0) {
    Serial.print("BAR ");
    while(bar.rawDataAvailable() > 0) {
      int data = bar.rawDataRead();
      Serial.print(data);
      Serial.print(" ");
    }
    Serial.println("");
  }

  if(hex.rawDataAvailable() > 0) {
    Serial.print("HEX ");
    while(hex.rawDataAvailable() > 0) {
      int data = hex.rawDataRead();
      Serial.print(data);
      Serial.print(" ");
    }
    Serial.println("");
  }
}
