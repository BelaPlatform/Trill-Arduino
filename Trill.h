/*
 * Trill library for Arduino
 * (c) 2020 bela.io
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

#define TRILL_SPEED_ULTRA_FAST 	0
#define TRILL_SPEED_FAST	1
#define TRILL_SPEED_NORMAL    	2
#define TRILL_SPEED_SLOW	3


class Trill {
	public:
		Trill();

		enum Mode {
			AUTO = -1,
			CENTROID = 0,
			RAW = 1,
			BASELINE = 2,
			DIFF = 3,
			NUM_MODES = 5
		};

		enum Device {
			TRILL_NONE = -1,
			TRILL_UNKNOWN = 0,
			TRILL_BAR = 1,
			TRILL_SQUARE = 2,
			TRILL_CRAFT = 3,
			TRILL_RING = 4,
			TRILL_HEX = 5,
			TRILL_NUM_DEVICES = 7
		};

		struct TrillDefaults
		{
			Trill::Device device;
			Trill::Mode mode;
			uint8_t address;
		};

		struct TrillDefaults trillDefaults[TRILL_NUM_DEVICES] = {
			{TRILL_NONE, AUTO, 0xFF},
			{TRILL_UNKNOWN, AUTO, 0xFF},
			{TRILL_BAR, CENTROID, 0x20},
			{TRILL_SQUARE, CENTROID, 0x28},
			{TRILL_CRAFT, DIFF, 0x30},
			{TRILL_RING, CENTROID, 0x38},
			{TRILL_HEX, CENTROID, 0x40}
		};

		static constexpr uint8_t interCommandDelay = 15;
		/**
		 * An array containing the valid values for the speed parameter
		 * in setScanSettings()
		 */
		static constexpr uint8_t speedValues[4] = {0, 1, 2, 3};
		/**
		 * The maximum value for the setPrescaler() method
		 */
		static constexpr uint8_t prescalerMax = 8;


		/* Initialise the hardware */
		int begin(Device device, uint8_t i2c_address = 255);
		/* Initialise the hardware, it's the same as begin() */
		int setup(Device device, uint8_t i2c_address = 255) { return begin(device, i2c_address); }

		/* --- Main communication --- */

		/* Return the type of device attached, or 0 if none is attached.
		   Same as begin(), but without re-initialising the system. */
		int identify();

		/**
		 * Does the device have one axis of position sensing?
		 *
		 * @return `true` if the device has one axis of position sensing
		 * and is set in #CENTROID mode, `false`
		 * otherwise.
		 */
		bool is1D();
		/**
		 * Does the device have two axes of position sensing?
		 *
		 * @return `true` if the device has two axes of position sensing
		 * and is set in #CENTROID mode, `false`
		 * otherwise.
		 */
		bool is2D();

		static Device probe(uint8_t i2c_address) {
			Trill t;

			/* Start I2C */
			t.begin(Trill::TRILL_UNKNOWN, i2c_address);

			/* Check the type of device attached */
			if(t.identify() != 0) {
				// Unable to identify device
				return Trill::TRILL_NONE;
			}
			return t.deviceType();
		}

		/* Return the device type already identified */
		Device deviceType() { return device_type_; };

		/* Return firmware version */
		int firmwareVersion() { return firmware_version_; }

		/* Get the mode that the device is currently in */
		Mode getMode() { return mode_; }

		/* Get the current address of the device */
		uint8_t getAddress() { return i2c_address_; }

		/* Get the number of capacitive channels on the device */
		unsigned int getNumChannels();

		/* Return the number of "button" channels on the device */
		unsigned int getNumButtons() { return 2 * (getMode() == CENTROID && TRILL_RING == deviceType());};

		/* Read the latest scan value from the sensor. Returns true on success. */
		boolean read();

		/* Update the baseline value on the sensor */
		void updateBaseline();

		/* --- Data processing --- */

		/* Button value for Ring? */
		int getButtonValue(uint8_t button_num);

		/* How many touches? */
		unsigned int getNumTouches();
		/* How many horizontal touches for 2D? */
		unsigned int getNumHorizontalTouches();

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
		void setMode(Mode mode);
		void setScanSettings(uint8_t speed, uint8_t num_bits);
		void setPrescaler(uint8_t prescaler);
		void setNoiseThreshold(uint8_t threshold);
		void setIDACValue(uint8_t value);
		void setMinimumTouchSize(uint16_t size);
		void setAutoScanInterval(uint16_t interval);

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
			kCommandAutoScanInterval = 16,
			kCommandIdentify = 255
		};

		enum {
			kOffsetCommand = 0,
			kOffsetData = 4
		};

		enum {
			kMaxTouchNum1D = 5,
			kMaxTouchNum2D = 4
		};

		enum {
			kCentroidLengthDefault = 20,
			kCentroidLengthRing = 24,
			kCentroidLength2D = 32,
			kRawLength = 60
		};

		enum {
			kNumChannelsBar = 26,
			kNumChannelsRing = 28,
			kNumChannelsMax = 30
		};

		enum {
			kRawLengthBar = 52,
			kRawLengthHex = 60,
			kRawLengthRing = 56
		};

		uint8_t i2c_address_;	/* Address of this slider on I2C bus */
		Device device_type_;	/* Which type of device is connected, if any */
		uint8_t firmware_version_;	/* Firmware version running on the device */
		Mode mode_;			/* Which mode the device is in */
		uint8_t last_read_loc_;	/* Which byte reads will begin from on the device */
		uint8_t num_touches_;	/* Number of touches on last read */
		uint8_t raw_bytes_left_; /* How many bytes still remaining to request? */

		uint8_t buffer_[kCentroidLength2D];	/* Buffer for standard response */
};

#endif /* TRILL_H */
