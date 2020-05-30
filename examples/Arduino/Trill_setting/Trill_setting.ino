  
#include <Trill.h>

Trill trillSensor;

String serialInput;
char gCommandToken = ':';
char gEndToken = '\n';

long gLastMillis = 0;

bool printSensorVal = true;

void setup() {
  // Initialise serial and touch sensor
  Serial.begin(115200);
  int ret = trillSensor.begin(Trill::TRILL_CRAFT);
  if(ret != 0) {
    Serial.println("failed to initialise trillSensor");  
    Serial.println("Error code: ");
    Serial.println(ret);
    Serial.println("\n");
  }
}

void loop() {
  
  if(Serial.available() > 0) {
    serialInput = Serial.readStringUntil(gEndToken);
    if(serialInput == "t") {
      printSensorVal = !printSensorVal;
    } else {
      int delimiterIndex = serialInput.indexOf(gCommandToken);
      String command = serialInput.substring(0, delimiterIndex);
      command.trim();
      String commandValue = serialInput.substring(delimiterIndex+1);
      commandValue.trim();
  
      if(command == "prescaler") {
        Serial.print("setting prescaler to ");
        Serial.println(commandValue.toInt());
        trillSensor.setPrescaler(commandValue.toInt());
      } else if(command == "baseline") {
        Serial.println("updatint baseline");
        trillSensor.updateBaseline();
      } else if(command == "noiseThreshold") {
        Serial.print("setting noise threshold to ");
        Serial.println(commandValue.toInt());
        trillSensor.setNoiseThreshold(commandValue.toInt());
      } else if(command == "numBits") {
        Serial.print("setting numBits to ");
        Serial.println(commandValue.toInt());
        trillSensor.setScanSettings(0, commandValue.toInt());
      } else if(command == "mode") {
        Serial.print("setting mode to ");
        Serial.println(commandValue);
        trillSensor.setMode(modeFromString(commandValue));
      } else {
        Serial.println("unknown command");
      }
    }
  }

  if(printSensorVal) {
    if(millis() - gLastMillis > 100) {
      gLastMillis += 100;
      trillSensor.requestRawData(60);
      
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
  }
}

Trill::Mode modeFromString(String &  modeString) {
  modeString.toLowerCase();
  if(modeString == "centroid") {
    return Trill::CENTROID;
  } else if(modeString == "raw") {
    return Trill::RAW;
  } else if(modeString == "baseline" || modeString == "base") {
    return Trill::BASELINE;
  } else if(modeString == "differential" || modeString == "diff") {
    return Trill::DIFF;
  }
  return Trill::AUTO;
}
