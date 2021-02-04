/*
 ____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\
http://bela.io

Two sub-sliders on a Trill Craft
*/

#include <Trill.h>

Trill trillSensor;

const unsigned int NUM_PADS = 30;
CentroidDetection<0,0>::WORD rawData[NUM_PADS];

uint8_t order[] = {14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0}; // OK

// we have two subsliders, one using pads as specified in order above,
// the other using pads 15-29 in order.

const uint8_t cd0NumPads = sizeof(order);
const uint8_t cd1Offset = 15;
const uint8_t cd1NumPads = 15;

const unsigned int maxNumCentroids = 3;
CentroidDetection<maxNumCentroids,cd0NumPads> cd0; // needs an internal buffer cd0NumPads long to store reordered data
CentroidDetection<maxNumCentroids,0> cd1; // 0 because it doesn't have to reorder the data
const unsigned int numCds = 2;
Touches* cds[] = {&cd0, &cd1}; // array to iterate through when printing

void setup() {
  cd0.setup(order);
  cd1.setup(nullptr); // this will assume the data passed in is already ordered
  // Initialise serial and touch sensor
  Serial.begin(115200);
  int ret = trillSensor.setup(Trill::TRILL_SQUARE);
  if(ret != 0) {
    Serial.println("failed to initialise trillSensor");
    Serial.print("Error code: ");
    Serial.println(ret);
    while(1)
      ;
  } else {
    Serial.println("Success initialising trillSensor================================================================================================================");
  }
  trillSensor.setMode(Trill::DIFF);
}

void loop() {
  trillSensor.requestRawData();
  // Read 20 times per second
  delay(50);
  unsigned n = 0;
  while(trillSensor.rawDataAvailable() > 0 && n < NUM_PADS) {
    rawData[n++] = trillSensor.rawDataRead();
  }
  cd0.process(rawData); // no need for a second argument: it already knows the size of rawData from the content of order
  cd1.process(rawData + cd1Offset, cd1NumPads); // the second argument tells it how many pads to process starting from rawData+cd1Offset
  for(uint8_t n = 0; n < numCds; ++n) {
    Touches* cd = cds[n];
    Serial.print("| cd");
    Serial.print(n);
    Serial.print("[");
    Serial.print(cd->getNumTouches());
    Serial.print("]: ");
    if(cd->getNumTouches() > 0) {
      for(int i = 0; i < cd->getNumTouches(); i++) {
          Serial.print(cd->touchLocation(i));
          Serial.print(" ");
          Serial.print(cd->touchSize(i));
          Serial.print(" ");
      }
    }
  }
  Serial.println("");
}
