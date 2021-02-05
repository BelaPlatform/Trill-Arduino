/*
 ____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\
http://bela.io

Two sub-sliders
*/

#include <Trill.h>

Trill trillSensor;

const unsigned int NUM_TOTAL_PADS = 30;
CustomSlider::WORD rawData[NUM_TOTAL_PADS];

const uint8_t slider0NumPads = 15;
const uint8_t slider1NumPads = 15;

// Order of the pads used by each slider
uint8_t slider0Pads[slider0NumPads] = {14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
uint8_t slider1Pads[slider1NumPads] = {15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29};

const unsigned int maxNumCentroids = 3;
const unsigned int numSliders = 2;
CustomSlider sliders[numSliders];

void setup() {
  sliders[0].setup(slider0Pads, slider0NumPads);
  sliders[1].setup(slider1Pads, slider1NumPads);
  // Initialise serial and touch sensor
  Serial.begin(115200);
  int ret;
  while(trillSensor.setup(Trill::TRILL_SQUARE)) {
    Serial.println("failed to initialise trillSensor");
    Serial.println("Retrying...");
    delay(100);
  }
  Serial.println("Success initialising trillSensor");
  trillSensor.setMode(Trill::DIFF);
}

void loop() {
  trillSensor.requestRawData();
  // Read 20 times per second
  delay(50);
  unsigned n = 0;
  // read all the data from the device into a local buffer
  while(trillSensor.rawDataAvailable() > 0 && n < NUM_TOTAL_PADS) {
    rawData[n++] = trillSensor.rawDataRead();
  }
  for(uint8_t n = 0; n < numSliders; ++n) {
    // have each custom slider process the raw data into touches
    sliders[n].process(rawData);
    Serial.print("| s");
    Serial.print(n);
    Serial.print("[");
    Serial.print(sliders[n].getNumTouches());
    Serial.print("]: ");
    if(sliders[n].getNumTouches() > 0) {
      for(int i = 0; i < sliders[n].getNumTouches(); i++) {
          Serial.print(sliders[n].touchLocation(i));
          Serial.print(" ");
          Serial.print(sliders[n].touchSize(i));
          Serial.print(" ");
      }
    }
  }
  Serial.println("");
}
