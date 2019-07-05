/*****************************
 * Trill library for Arduino
 * (c) 2019 Andrew McPherson
 *
 * This library communicates with the Trill sensors
 * using I2C.
 *
 * BSD license
 */

#ifndef TRILL_H
#define TRILL_H

#if (ARDUINO >= 100)
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#include "Wire.h"

#define TRILL_MODE_NORMAL	0
#define TRILL_MODE_RAW		1
#define TRILL_MODE_BASELINE	2
#define TRILL_MODE_DIFF		3

#define TRILL_DEVICE_NONE	0
#define TRILL_DEVICE_1D		1
#define TRILL_DEVICE_2D		2

#define TRILL_SPEED_ULTRA_FAST 0
#define TRILL_SPEED_FAST	   1
#define TRILL_SPEED_NORMAL     2
#define TRILL_SPEED_SLOW	   3
 

class Trill {
public:
	Trill(uint8_t i2c_address, uint8_t reset_pin = 0);

	/* Initialise the hardware */
	int begin();

	/* --- Main communication --- */

	/* Return the type of device attached, or 0 if none is attached. 
	   Same as begin(), but without re-initialising the system. */
	int identify();

	/* Return the device type already identified */
	int deviceType();

	/* Read the latest scan value from the sensor. Returns true on success. */
	boolean read();

	/* Update the baseline value on the sensor */
	void updateBaseline();

	/* Reset the sensor, if reset pin is enabled */
	void reset();

	/* --- Data processing --- */

	/* How many touches? < 0 means error. */
	int numberOfTouches();
	/* How many horizontal touches for 2D? */
	int numberOfHorizontalTouches();

	/* Location and size of a particular touch, ranging from 0 to N-1.
	   Returns -1 if no such touch exists. */
	int touchLocation(uint8_t touch_num);
	int touchSize(uint8_t touch_num);

	/* These methods for horizontal touches on 2D sliders */
	int touchHorizontalLocation(uint8_t touch_num);
	int touchHorizontalSize(uint8_t touch_num);

	/* --- Raw data handling --- */

	/* Request raw data; wrappers for Wire */
	void requestRawData(uint8_t max_length = 0xFF);
	int rawDataAvailable();
	int rawDataRead();

	/* --- Scan configuration settings --- */
	void setMode(uint8_t mode);
	void setScanSettings(uint8_t speed, uint8_t num_bits);
	void setPrescaler(uint8_t prescaler);
	void setNoiseThreshold(uint8_t threshold);
	void setIDACValue(uint8_t value);
	void setMinimumTouchSize(uint16_t size);

private:
	void prepareForDataRead();	

	enum {
		kCommandNone = 0,
		kCommandMode = 1,
		kCommandScanSettings = 2,
		kCommandPrescaler = 3,
		kCommandNoiseThreshold = 4,
		kCommandIdac = 5,
		kCommandBaselineUpdate = 6,
		kCommandMinimumSize = 7,
		kCommandIdentify = 255
	};

	enum {
		kOffsetCommand = 0,
		kOffsetData = 4
	};

	enum {
		kNormalLengthDefault = 20,
		kNormalLengthMax = 20,
		kNormalLength2D = 32,
		kMaxTouchNum1D = 5,
		kMaxTouchNum2D = 4,
		
		kRawLength1D = 52,
		kRawLength2D = 60,
		kRawLengthDefault = 60,
		kRawLengthMax = 60
	};

	uint8_t i2c_address_;	/* Address of this slider on I2C bus */
	uint8_t reset_pin_;		/* Pin for active-high reset of sensor */
	uint8_t device_type_;	/* Which type of device is connected, if any */
	uint8_t firmware_version_;	/* Firmware version running on the device */
	uint8_t mode_;			/* Which mode the device is in */
	uint8_t last_read_loc_;	/* Which byte reads will begin from on the device */
	uint8_t num_touches_;	/* Number of touches on last read */
	uint8_t raw_bytes_left_; /* How many bytes still remaining to request? */

	uint8_t buffer_[kNormalLengthMax];	/* Buffer for standard response */
};

#endif /* TRILL_H */
