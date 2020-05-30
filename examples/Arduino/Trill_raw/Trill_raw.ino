
#include <Trill.h>

Trill trillSensor; // for Trill Craft

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  int ret = trillSensor.begin(Trill::TRILL_CRAFT);
  if(ret != 0) {
    Serial.println("failed to initialise trillSensor");
    Serial.print("Error code: ");
    Serial.println(ret);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
  trillSensor.requestRawData();
   
  if(trillSensor.rawDataAvailable() > 0) {
    while(trillSensor.rawDataAvailable() > 0) {
      int data = trillSensor.rawDataRead();
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
}
