#include <Trill.h>

/*
 ____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\
http://bela.io

\example raw-print-multi

Trill Print Multiple
====================

This is a example of using multiple trill sensors with a single Arduino board.
The I2C pins of all the sensors are connected together (SCL with SCL, SDA with SDA).
Each sensor has a unique address. Here is the table of address for each sensor type,
the first column is the default address:

| Type:  | Addresses (trillHex)                              |
|--------|----------------------------------------------|
| BAR    | 0x20 0x21 0x22 0x23 0x24 0x25 0x26 0x27 0x28 |
| SQUARE | 0x28 0x29 0x2A 0x2B 0x2C 0x2D 0x2E 0x2F 0x30 |
| CRAFT  | 0x30 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 |
| RING   | 0x38 0x39 0x3A 0x3B 0x3C 0x3D 0x3E 0x3F 0x40 |
| HEX    | 0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 |
| FLEX   | 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F 0x50 |

In this example the sensor readings are printed to the console as raw values using the
`rawDataRead()` method.
*/

Trill trillBar; // for Trill Bar
Trill trillSquare; // for Trill Square
Trill trillCraft; // for Trill Craft
Trill trillRing; // for Trill Ring
Trill trillHex; // for Trill Hex
Trill trillFlex; // for Trill Flex


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  if(trillBar.setup(Trill::TRILL_BAR) != 0)
    Serial.println("failed to initialise trill bar");

  if(trillSquare.setup(Trill::TRILL_SQUARE) != 0)
    Serial.println("failed to initialise trill square");

  if(trillCraft.setup(Trill::TRILL_CRAFT) != 0)
    Serial.println("failed to initialise trill craft");

  if(trillRing.setup(Trill::TRILL_RING) != 0)
    Serial.println("failed to initialise trill ring");

  if(trillHex.setup(Trill::TRILL_HEX) != 0)
    Serial.println("failed to initialise trill hex");

  if(trillFlex.setup(Trill::TRILL_FLEX) != 0)
  {
    Serial.println("failed to initialise trill flex");
  } else {
    delay(10);
    trillFlex.setPrescaler(4);
    delay(10);
    trillFlex.updateBaseline();
    delay(10);
  }

  trillBar.setMode(Trill::DIFF);
  trillSquare.setMode(Trill::DIFF);
  trillCraft.setMode(Trill::DIFF);
  trillRing.setMode(Trill::DIFF);
  trillHex.setMode(Trill::DIFF);
  trillFlex.setMode(Trill::DIFF);
}

void printLine(unsigned int n, char character = '_') {
  for(int i = 0; i < n; i++) {
    Serial.print(character);
  }
  Serial.println("");
}

void printRawData(Trill & trill) {
  while(trill.rawDataAvailable() > 0) {
    int data = trill.rawDataRead();
    if(data < 1000)
      Serial.print(0);
    if(data < 100)
      Serial.print(0);
    if(data < 10)
      Serial.print(0);
    Serial.print(data);
    Serial.print(" ");
  }
  Serial.println("");
}

void loop() {
  // put your main code here, to run repeatedly:

  delay(100);

  Serial.println("");

  trillBar.requestRawData();
  if(trillBar.rawDataAvailable() > 0) {
    Serial.print("BAR    ");
    printRawData(trillBar);
  }

  trillSquare.requestRawData();
  if(trillSquare.rawDataAvailable() > 0) {
    Serial.print("SQUARE ");
    printRawData(trillSquare);
  }

  trillCraft.requestRawData();
  if(trillCraft.rawDataAvailable() > 0) {
    Serial.print("CRAFT  ");
    printRawData(trillCraft);
  }

  trillRing.requestRawData();
  if(trillRing.rawDataAvailable() > 0) {
    Serial.print("RING  ");
    printRawData(trillRing);
  }

  trillHex.requestRawData();
  if(trillHex.rawDataAvailable() > 0) {
    Serial.print("HEX    ");
    printRawData(trillHex);
  }

  trillFlex.requestRawData();
  if(trillFlex.rawDataAvailable() > 0) {
    Serial.print("FLEX    ");
    printRawData(trillFlex);
  }

  printLine(156, '_');
}
