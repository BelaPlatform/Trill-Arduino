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

class Touches
{
public:
	Touches() { num_touches = 0; };
	typedef uint16_t TouchData_t;
	/* How many touches? */
	uint8_t getNumTouches() const;
	/* Location and size of a particular touch, ranging from 0 to N-1.
	   Returns -1 if no such touch exists. */
	int touchLocation(uint8_t touch_num) const;
	int touchSize(uint8_t touch_num) const;
	void processCentroids(uint8_t maxCentroids);
	TouchData_t const* centroids;
	TouchData_t const* sizes;
	uint8_t num_touches;/*  Number of touches. Updated by processCentroids() */
};

class Touches2D : public Touches
{
public:
	unsigned int getNumHorizontalTouches();
	int touchHorizontalLocation(uint8_t touch_num);
	int touchHorizontalSize(uint8_t touch_num);
protected:
	Touches2D() {};
	Touches horizontal;
};

class Trill : public Touches2D
{
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
			TRILL_FLEX = 6,
			TRILL_NUM_DEVICES = 7
		};

		struct TrillDefaults
		{
			Trill::Device device;
			Trill::Mode mode;
			uint8_t address;
		};

		struct TrillDefaults trillDefaults[TRILL_NUM_DEVICES + 1] = {
			{TRILL_NONE, AUTO, 0xFF},
			{TRILL_UNKNOWN, AUTO, 0xFF},
			{TRILL_BAR, CENTROID, 0x20},
			{TRILL_SQUARE, CENTROID, 0x28},
			{TRILL_CRAFT, DIFF, 0x30},
			{TRILL_RING, CENTROID, 0x38},
			{TRILL_HEX, CENTROID, 0x40},
			{TRILL_FLEX, DIFF, 0x48},
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

		/* --- Raw data handling --- */

		/* Request raw data; wrappers for Wire */
		boolean requestRawData(uint8_t max_length = 0xFF);
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
			kNumChannelsRing = 30,
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
		uint8_t raw_bytes_left_; /* How many bytes still remaining to request? */

		uint16_t buffer_[kCentroidLength2D * 2];/* Buffer for centroid response */
};

// first template argument is the max num of centroids
// the second argument is the number of readings that will be processed at the
// same time. This should be 0 if the data passed to process() is already ordered
template <uint8_t _maxNumCentroids, uint8_t _numReadings>
class CentroidDetection : public Touches
{
public:
	typedef uint16_t WORD;
	CentroidDetection() {};
	CentroidDetection(const unsigned int* order);
	int begin(const uint8_t* order, unsigned int numReadings) {
		return setup(order, numReadings);
	}
	// pass nullptr if the data passed to process() is already ordered.
	int setup(const uint8_t* order, unsigned int orderLength) {
		this->order = order;
		Touches::centroids = this->centroids;
		Touches::sizes = this->sizes;
		num_touches = 0;
		this->orderLength = orderLength;
		if(orderLength > _numReadings)
			return -1; // cannot work with more than _numReadings
		return 0;
	}

	void process(const WORD* rawData) {
		uint8_t nMax = _numReadings < orderLength ? _numReadings : orderLength;
		if(order) {
			for(unsigned int n = 0; n < nMax; ++n) {
				data[n] = rawData[order[n]];
			}
			cc.CSD_waSnsDiff = data;
		} else {
			// no reordering needed
			cc.CSD_waSnsDiff = rawData;
		}
		cc.calculateCentroids(centroids, sizes, _maxNumCentroids, 0, nMax, nMax);
		processCentroids(_maxNumCentroids);
	}

	void setMinimumTouchSize(TouchData_t minSize) {
		cc.wMinimumCentroidSize = minSize;
	}

private:
	// a small helper class, whose main purpose is to wrap the #include
	// and make all the variables related to it private and multi-instance safe
	class CalculateCentroids
	{
	public:
		typedef uint8_t BYTE;
		WORD const * CSD_waSnsDiff;
		WORD wMinimumCentroidSize = 0;
		BYTE SLIDER_BITS = 7;
		WORD wAdjacentCentroidNoiseThreshold = 400; // Trough between peaks needed to identify two centroids
		//WORD calculateCentroids(WORD *centroidBuffer, WORD *sizeBuffer, BYTE maxNumCentroids, BYTE minSensor, BYTE maxSensor, BYTE numSensors);
		// calculateCentroids is defined here:
		#include "calculateCentroids.h"
	};
	TouchData_t centroids[_maxNumCentroids];
	TouchData_t sizes[_maxNumCentroids * 2];
	const uint8_t* order;
	unsigned int orderLength;
	WORD data[_numReadings];
	CalculateCentroids cc;
};

class CustomSlider : public CentroidDetection<5, 30> {};
#endif /* TRILL_H */
