  
#include <Wire.h>
#include <Trill.h>

#define I2C_ADDRESS 24

Trill slider(I2C_ADDRESS);

char gSerialBuffer[9];
int gSerialBufferIndex = 0;

int gCommandValues[4];
int gCommandValueIndex = 0;
long gLastMillis = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  if(!slider.begin())
    Serial.println("failed to initialise slider");
    
  slider.setMode(TRILL_MODE_DIFF);
  //delay(10);
  //slider.setIDACValue(160);
}

void loop() {
  if(Serial.available() > 0) {
    byte input = Serial.read();
    if(input == '\n') {
      if(gSerialBufferIndex > 0 && gCommandValueIndex < 4) {
        gSerialBuffer[gSerialBufferIndex] = '\0';
        gCommandValues[gCommandValueIndex++] = atoi(gSerialBuffer);
      }
      
      if(gCommandValueIndex > 0) {
        // TODO: send I2C message to chip
        Wire.beginTransmission(I2C_ADDRESS);
        Wire.write(0); // Always begin at byte 0 for commands
        for(int i = 0; i < gCommandValueIndex; i++)
          Wire.write(gCommandValues[i]);
        Wire.endTransmission();
        gLastMillis = millis();  // Give 100ms for the chip to catch up
      }  
      gCommandValueIndex = 0;
      gSerialBufferIndex = 0;
    }
    else if(input == ' ' || input == '\t' || input == ',') {
      if(gSerialBufferIndex > 0 && gCommandValueIndex < 4) {
        gSerialBuffer[gSerialBufferIndex] = '\0';
        gCommandValues[gCommandValueIndex++] = atoi(gSerialBuffer);
      }
      gSerialBufferIndex = 0;
    }
    else {  
      if(gSerialBufferIndex < 8)
        gSerialBuffer[gSerialBufferIndex++] = input;
    }
  }

  // Scan every 100ms
  if(millis() - gLastMillis > 100) {
    gLastMillis += 100;
    slider.requestRawData(60);
    
    if(slider.rawDataAvailable() > 0) {
      while(slider.rawDataAvailable() > 0) {
        int data = slider.rawDataRead();
        Serial.print(data);
        Serial.print(" ");
      }
      Serial.println("");
    }
  }
}
