/*
 ____  _____ _        _
| __ )| ____| |      / \
|  _ \|  _| | |     / _ \
| |_) | |___| |___ / ___ \
|____/|_____|_____/_/   \_\
http://bela.io

\example craft-settings

Adjusting Trill Settings
========================

This example will work with all types of Trill sensor and will allow you to adjust
the sensitivity and threshold settings but it is set for Trill Craft by default as
it is where these changes would be more obvious and necessary..

The first thing to do is make sure that the correct sensor type is
given to `touchSensor.setup();`. If you have changed the address of the sensor
then you will need to add that to this function too.

There are two important sensor settings that you may want to adjust when working
with the Trill sensors: the `threshold` and the `prescalar`.

The `threshold` setting is simply the threshold above which to read and is for
ignoring any noise that might be present in the lowest regions of the sensor reading.

The `prescalar` setting equates to the sensitivity of the sensor. Technically, this
value is a divider for the clock on the Cypress chip and so it decides how long the
chip charges the connected material for before taking a reading. There are 8 different
settings for the prescalar.

The rule of thumb when adjusting these values is:
- A higher value prescaler (i.e. longer charging time as it is a divider of the clock)
  is better for more resistive materials and larger conductive objects connected.
- A lower value prescaler is better for proximity sensing.

When connecting different materials to Trill Craft we recommend experimenting with
the settings using this example.

Parameters can be adjusted via the Arduino Serial Monitor by writting one-line commands
followed by a semicolon and the value to be set:
```
	prescaler: prescalerValue
	threshold: thresholdValue
	bits: numberOfBits
```

Commands can also be used to update the sensor baseline:
```
	baseline
```

or to change sensor mode of operation:
```
	mode: sensorMode
```
(where sensorMode can be one of {centroid, raw, baseline, differential}).

This example also prints the raw data from the sensor, which is useful to see the effects
of changing the different parameters (specially for Trill Craft).. This can be toggled
on and off by sending the character 't' over the Serial Monitor.
*/


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
        gLastMillis = millis();  // Give 100ms for the chip to catch up
      } else if(command == "baseline") {
        Serial.println("updating baseline");
        trillSensor.updateBaseline();
        gLastMillis = millis();  // Give 100ms for the chip to catch up
      } else if(command == "noiseThreshold") {
        Serial.print("setting noise threshold to ");
        Serial.println(commandValue.toInt());
        trillSensor.setNoiseThreshold(commandValue.toInt());
        gLastMillis = millis();  // Give 100ms for the chip to catch up
      } else if(command == "numBits") {
        Serial.print("setting numBits to ");
        Serial.println(commandValue.toInt());
        trillSensor.setScanSettings(0, commandValue.toInt());
        gLastMillis = millis();  // Give 100ms for the chip to catch up
      } else if(command == "mode") {
        Serial.print("setting mode to ");
        Serial.println(commandValue);
        trillSensor.setMode(modeFromString(commandValue));
        gLastMillis = millis();  // Give 100ms for the chip to catch up
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
