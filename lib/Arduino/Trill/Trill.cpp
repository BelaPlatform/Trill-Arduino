/*****************************
 * Trill library for Arduino
 * (c) 2019 Andrew McPherson
 *
 * This library communicates with the Trill sensors
 * using I2C.
 *
 * BSD license
 */

#include "Trill.h"

#define MAX_TOUCH_1D_OR_2D ((device_type_ == TRILL_SQUARE ? kMaxTouchNum2D : kMaxTouchNum1D))
#define RAW_LENGTH ((device_type_ == TRILL_BAR ? 2 * kNumChannelsBar \
			: device_type_ == TRILL_RING ? 2 * kNumChannelsRing \
			: 2 * kNumChannelsMax))

Trill::Trill()
: device_type_(TRILL_NONE), firmware_version_(0),
  mode_(AUTO), last_read_loc_(0xFF), num_touches_(0),
  raw_bytes_left_(0)
{
}

/* Initialise the hardware. Returns the type of device attached, or 0
   if none is attached. */
int Trill::begin(Device device, Mode mode, uint8_t i2c_address) {

	if(128 <= i2c_address)
		i2c_address = trillDefaults[device+1].address;

	/* Unknown default address */
	if(128 <= i2c_address) {
		return  -2;
	}

	i2c_address_ = i2c_address;

	/* Start I2C */
	Wire.begin();

	/* Check the type of device attached */
	if(identify() != 0) {
		// Unable to identify device
		return 2;
	}

	/* Check for wrong device type */
	if(TRILL_UNKNOWN != device && device_type_ != device) {
		device_type_ = TRILL_NONE;
		return -3;
	}

	/* Check for device mode */
	if(AUTO == mode)
		mode = trillDefaults[device+1].mode;
	if(AUTO == mode) {
		return -1;
	}

	/* Put the device in the correspondent mode */
	setMode(mode);

	/* Set default scan settings */
	setScanSettings(0, 12);

	updateBaseline();

	/* Wait to process the command before sending the second identify command */
	delay(25);

	return 0;
}

/* Return the type of device attached, or 0 if none is attached. */
int Trill::identify() {
	Wire.beginTransmission(i2c_address_);
	Wire.write(kOffsetCommand);
	Wire.write(kCommandIdentify);
	Wire.endTransmission();

	/* Give Trill time to process this command */
	delay(25);

	last_read_loc_ = kOffsetCommand;
	Wire.requestFrom(i2c_address_, (uint8_t)3);

	if(Wire.available() < 3) {
		/* Unexpected or no response; no valid device connected */
		device_type_ = TRILL_NONE;
		firmware_version_ = 0;
		return device_type_;
	}

	Wire.read();	// Discard first input
	device_type_ = (Device)Wire.read();
	firmware_version_ = Wire.read();

	return 0;
}

/* Read the latest scan value from the sensor. Returns true on success. */
boolean Trill::read() {
	uint8_t loc = 0;
	uint8_t length = kCentroidLengthDefault;

	/* Set the read location to the right place if needed */
	prepareForDataRead();

	if(device_type_ == TRILL_SQUARE || device_type_ == TRILL_HEX)
		length = kCentroidLength2D;

	if(device_type_ == TRILL_RING)
		length = kCentroidLengthRing;

	Wire.requestFrom(i2c_address_, length);
	while(Wire.available()) {
		buffer_[loc++] = Wire.read();
	}

	/* Check for read error */
	if(loc < length) {
		num_touches_ = 0;
		return false;
	}

	/* Look for the first instance of 0xFFFF (no touch)
   	   in the buffer */
	for(loc = 0; loc < MAX_TOUCH_1D_OR_2D; loc++) {
		if(buffer_[2 * loc] == 0xFF && buffer_[2 * loc + 1] == 0xFF)
			break;
	}
	num_touches_ = loc;

	if(device_type_ == TRILL_SQUARE || device_type_ == TRILL_HEX) {
		/* Look for the number of horizontal touches in 2D sliders
		   which might be different from number of vertical touches */
		for(loc = 0; loc < kMaxTouchNum2D; loc++) {
			if(buffer_[2 * loc + 4 * kMaxTouchNum2D] == 0xFF && 
			   buffer_[2 * loc + 4 * kMaxTouchNum2D + 1] == 0xFF)
				break;
		}
		num_touches_ |= (loc << 4);
	}

	return true;
}

/* Update the baseline value on the sensor */
void Trill::updateBaseline() {
	Wire.beginTransmission(i2c_address_);
	Wire.write(kOffsetCommand);
	Wire.write(kCommandBaselineUpdate);
	Wire.endTransmission();

	last_read_loc_ = kOffsetCommand;
}

/* How many touches? < 0 means error. */
unsigned int Trill::getNumTouches() {
	if(mode_ != CENTROID)
		return 0;

	/* Lower 4 bits hold number of 1-axis or vertical touches */
	return (num_touches_ & 0x0F);
}

/* How many horizontal touches for 2D? */
unsigned int Trill::getNumHorizontalTouches() {
	if(!is2D())
		return 0;

	/* Upper 4 bits hold number of horizontal touches */
	return (num_touches_ >> 4);
}

/* Location and size of a particular touch, ranging from 0 to N-1.
   Returns -1 if no such touch exists. */
int Trill::touchLocation(uint8_t touch_num) {
	int result;

	if(mode_ != CENTROID)
		return -1;
	if(touch_num >= MAX_TOUCH_1D_OR_2D)
		return -1;

	result = buffer_[2*touch_num] * 256;
	result += buffer_[2*touch_num + 1];

	return result;
}

int Trill::touchSize(uint8_t touch_num) {
	int result;

	if(mode_ != CENTROID)
		return -1;
	if(touch_num >= MAX_TOUCH_1D_OR_2D)
		return -1;

	result = buffer_[2*touch_num + 2*MAX_TOUCH_1D_OR_2D] * 256;
	result += buffer_[2*touch_num + 2*MAX_TOUCH_1D_OR_2D + 1];

	return result;
}

/* These methods for horizontal touches on 2D sliders */
int Trill::touchHorizontalLocation(uint8_t touch_num) {
	int result;

	if(!is2D())
		return -1;
	if(touch_num >= kMaxTouchNum2D)
		return -1;

	result = buffer_[2*touch_num + 4*kMaxTouchNum2D] * 256;
	result += buffer_[2*touch_num + 4*kMaxTouchNum2D + 1];

	return result;
}

int Trill::touchHorizontalSize(uint8_t touch_num) {
	int result;

	if(!is2D())
		return -1;
	if(touch_num >= kMaxTouchNum2D)
		return -1;

	result = buffer_[2*touch_num + 6*kMaxTouchNum2D] * 256;
	result += buffer_[2*touch_num + 6*kMaxTouchNum2D + 1];

	return result;
}

/* Request raw data; wrappers for Wire */
void Trill::requestRawData(uint8_t max_length) {
	uint8_t length;

	prepareForDataRead();

	if(max_length == 0xFF) {
		length = RAW_LENGTH;
	}
	if(length > kRawLength)
		length = kRawLength;

	/* The raw data might be longer than the Wire.h maximum buffer
	 * (BUFFER_LENGTH in Wire.h).
	 * If so, split it into two reads. */
	if(length <= BUFFER_LENGTH) {
		Wire.requestFrom(i2c_address_, length);
		raw_bytes_left_ = 0;
	}
	else {
		Wire.requestFrom(i2c_address_, (uint8_t)BUFFER_LENGTH);
		raw_bytes_left_ = length - BUFFER_LENGTH;
	}
}

int Trill::rawDataAvailable() {
	/* Raw data items are 2 bytes long; return number of them available */
	return ((Wire.available() + raw_bytes_left_) >> 1);
}

/* Raw data is in 16-bit big-endian format */
int Trill::rawDataRead() {
	int result;

	if(Wire.available() < 2) {
		/* Read more bytes if we need it */
		if(raw_bytes_left_ > 0) {
			/* Move read pointer on device */
			Wire.beginTransmission(i2c_address_);
			Wire.write(kOffsetData + BUFFER_LENGTH);
			Wire.endTransmission();
			last_read_loc_ = kOffsetData + BUFFER_LENGTH;

			/* Now gather what's left */
			Wire.requestFrom(i2c_address_, raw_bytes_left_);
			raw_bytes_left_ = 0;
		}

		/* Check again if we've got anything... */
		if(Wire.available() < 2)
			return 0;
	}

	result = (int)Wire.read() * 256;
	result += (int)Wire.read();
	return result;
}

/* Scan configuration settings */
void Trill::setMode(Mode mode) {
	Wire.beginTransmission(i2c_address_);
	Wire.write(kOffsetCommand);
	Wire.write(kCommandMode);
	Wire.write(mode);
	Wire.endTransmission();

	mode_ = mode;
	last_read_loc_ = kOffsetCommand;
}

void Trill::setScanSettings(uint8_t speed, uint8_t num_bits) {
	if(speed > 3)
		speed = 3;
	if(num_bits < 9)
		num_bits = 9;
	if(num_bits > 16)
		num_bits = 16;
	Wire.beginTransmission(i2c_address_);
	Wire.write(kOffsetCommand);
	Wire.write(kCommandScanSettings);
	Wire.write(speed);
	Wire.write(num_bits);
	Wire.endTransmission();

	last_read_loc_ = kOffsetCommand;
}

void Trill::setPrescaler(uint8_t prescaler) {
	Wire.beginTransmission(i2c_address_);
	Wire.write(kOffsetCommand);
	Wire.write(kCommandPrescaler);
	Wire.write(prescaler);
	Wire.endTransmission();

	last_read_loc_ = kOffsetCommand;
}

void Trill::setNoiseThreshold(uint8_t threshold) {
	if(threshold > 255)
		threshold = 255;
	if(threshold < 0)
		threshold = 0;
	Wire.beginTransmission(i2c_address_);
	Wire.write(kOffsetCommand);
	Wire.write(kCommandNoiseThreshold);
	Wire.write(threshold);
	Wire.endTransmission();

	last_read_loc_ = kOffsetCommand;
}

void Trill::setIDACValue(uint8_t value) {
	Wire.beginTransmission(i2c_address_);
	Wire.write(kOffsetCommand);
	Wire.write(kCommandIdac);
	Wire.write(value);
	Wire.endTransmission();

	last_read_loc_ = kOffsetCommand;
}

void Trill::setMinimumTouchSize(uint16_t size) {
	Wire.beginTransmission(i2c_address_);
	Wire.write(kOffsetCommand);
	Wire.write(kCommandMinimumSize);
	Wire.write(size >> 8);
	Wire.write(size & 0xFF);
	Wire.endTransmission();

	last_read_loc_ = kOffsetCommand;
}

void Trill::setAutoScanInterval(uint16_t interval) {
	Wire.beginTransmission(i2c_address_);
	Wire.write(kOffsetCommand);
	Wire.write(kCommandAutoScanInterval);
	Wire.write(interval >> 8);
	Wire.write(interval & 0xFF);
	Wire.endTransmission();

	last_read_loc_ = kOffsetCommand;
}

/* Prepare the device to read data if it is not already prepared */
void Trill::prepareForDataRead() {
	if(last_read_loc_ != kOffsetData) {
		Wire.beginTransmission(i2c_address_);
		Wire.write(kOffsetData);
		Wire.endTransmission();

		last_read_loc_ = kOffsetData;
	}
}

int Trill::getButtonValue(uint8_t button_num)
{
	if(mode_ != CENTROID)
		return -1;
	if(button_num > 1)
		return -1;
	if(device_type_ != TRILL_RING)
		return -1;

	return (((buffer_[4*MAX_TOUCH_1D_OR_2D+2*button_num] << 8) + buffer_[4*MAX_TOUCH_1D_OR_2D+2*button_num+1]) & 0x0FFF);
}

unsigned int Trill::getNumChannels()
{
	switch(device_type_) {
		case TRILL_BAR: return kNumChannelsBar;
		case TRILL_RING: return kNumChannelsRing;
		default: return kNumChannelsMax;
	}
}

bool Trill::is1D()
{
	if(CENTROID != mode_)
		return false;
	switch(device_type_) {
		case TRILL_BAR:
		case TRILL_RING:
		case TRILL_CRAFT:
			return true;
		default:
			return false;
	}
}

bool Trill::is2D()
{
	if(CENTROID != mode_)
		return false;
	switch(device_type_) {
		case TRILL_SQUARE:
		case TRILL_HEX:
			return true;
		default:
			return false;
	}
}

Trill::Device probe(uint8_t i2c_adresss) {
	Trill t;
	/* Start I2C */
	Wire.begin();

	/* Check the type of device attached */
	if(t.identify() != 0) {
		// Unable to identify device
		return Trill::TRILL_NONE;
	}
	return t.deviceType();
}
