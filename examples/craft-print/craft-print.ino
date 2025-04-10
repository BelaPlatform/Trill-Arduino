/*
   ____  _____ _        _
   | __ )| ____| |      / \
   |  _ \|  _| | |     / _ \
   | |_) | |___| |___ / ___ \
   |____/|_____|_____/_/   \_\
http://bela.io

\example craft-print

Trill Craft Print
=================

This is an example of how to communicate with the Trill Craft
sensor using the Trill Arduino library.

The sensor is set to Differential mode and readings from each of the
capacitive connections on the sensor ar eprinted to the serial port.
 */

#include <Trill.h>

Trill trillSensor; // for Trill Craft

void setup() {
	// put your setup code here, to run once:
	Serial.begin(115200);
	int ret;
	while((ret = trillSensor.setup(Trill::TRILL_CRAFT))) {
		Serial.println("failed to initialise trillSensor");
		Serial.print("Error code: ");
		Serial.println(ret);
	}
}

void loop() {
	// put your main code here, to run repeatedly:
	delay(100);
	trillSensor.requestRawData();

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
