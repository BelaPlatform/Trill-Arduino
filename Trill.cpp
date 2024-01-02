/*
 * Trill library for Arduino
 * (c) 2020 bela.io
 *
 * This library communicates with the Trill sensors
 * using I2C.
 *
 * BSD license
 */

#include "Trill.h"

// some implementations of Wire (e.g.: Particle OS) do not define BUFFER_LENGTH
#ifndef BUFFER_LENGTH
#define BUFFER_LENGTH 32
#endif // BUFFER_LENGTH

#define MAX_TOUCH_1D_OR_2D (((device_type_ == TRILL_SQUARE || device_type_ == TRILL_HEX) ? kMaxTouchNum2D : kMaxTouchNum1D))
#define RAW_LENGTH ((device_type_ == TRILL_BAR ? 2 * kNumChannelsBar \
			: device_type_ == TRILL_RING ? 2 * kNumChannelsRing \
			: 2 * kNumChannelsMax))

Trill::Trill()
: device_type_(TRILL_NONE), firmware_version_(0),
  mode_(AUTO), last_read_loc_(0xFF), raw_bytes_left_(0), wire_(&Wire)
{
}

/* Initialise the hardware. Returns the type of device attached, or 0
   if none is attached. */
int Trill::begin(Device device, uint8_t i2c_address, TwoWire* wire) {
	wire_ = wire;

	if(128 <= i2c_address)
		i2c_address = trillDefaults[device+1].address;

	/* Unknown default address */
	if(128 <= i2c_address) {
		return  -2;
	}

	i2c_address_ = i2c_address;

	/* Start I2C */
	wire_->begin();

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
	Mode mode = trillDefaults[device+1].mode;
	if(AUTO == mode) {
		return -1;
	}

	/* Put the device in the correspondent mode */
	setMode(mode);
	delay(interCommandDelay);

	Touches::centroids = buffer_;
	Touches::sizes = buffer_ + MAX_TOUCH_1D_OR_2D;
	if(is2D()) {
		horizontal.centroids = buffer_ + 2 * MAX_TOUCH_1D_OR_2D;
		horizontal.sizes = buffer_ + 3 * MAX_TOUCH_1D_OR_2D;
	} else
		horizontal.num_touches = 0;

	/* Set default scan settings */
	setScanSettings(0, 12);
	delay(interCommandDelay);

	updateBaseline();
	delay(interCommandDelay); // not really needed, but it ensures the first command the user sends after calling setup() will be adequately timed. Hopefully this is not a source of confusion...

	return 0;
}

/* Return the type of device attached, or 0 if none is attached. */
int Trill::identify() {
	wire_->beginTransmission(i2c_address_);
	wire_->write(kOffsetCommand);
	wire_->write(kCommandIdentify);
	int ret = wire_->endTransmission();
	if(ret)
		return ret;

	/* Give Trill time to process this command */
	delay(25);

	last_read_loc_ = kOffsetCommand;
	wire_->requestFrom(i2c_address_, (uint8_t)3);

	if(wire_->available() < 3) {
		/* Unexpected or no response; no valid device connected */
		device_type_ = TRILL_NONE;
		firmware_version_ = 0;
		return -1;
	}

	wire_->read();	// Discard first input
	device_type_ = (Device)wire_->read();
	firmware_version_ = wire_->read();

	return 0;
}

/* Read the latest scan value from the sensor. Returns true on success. */
boolean Trill::read() {
	if(CENTROID != mode_)
		return false;
	uint8_t loc = 0;
	uint8_t length = kCentroidLengthDefault;

	/* Set the read location to the right place if needed */
	prepareForDataRead();

	if(device_type_ == TRILL_SQUARE || device_type_ == TRILL_HEX)
		length = kCentroidLength2D;

	if(device_type_ == TRILL_RING)
		length = kCentroidLengthRing;

	wire_->requestFrom(i2c_address_, length);
	while(wire_->available() >= 2) {
		uint8_t msb = wire_->read();
		uint8_t lsb = wire_->read();
		buffer_[loc] = lsb + (msb << 8);
		++loc;
	}

	uint8_t maxNumCentroids = MAX_TOUCH_1D_OR_2D;
	boolean ret = true;
	/* Check for read error */
	if(loc * 2 < length) {
		maxNumCentroids = 0;
		ret = false;
	}

	processCentroids(maxNumCentroids);
	if(is2D())
		horizontal.processCentroids(maxNumCentroids);

	return ret;
}

/* Update the baseline value on the sensor */
void Trill::updateBaseline() {
	wire_->beginTransmission(i2c_address_);
	wire_->write(kOffsetCommand);
	wire_->write(kCommandBaselineUpdate);
	wire_->endTransmission();

	last_read_loc_ = kOffsetCommand;
}

/* Request raw data; wrappers for Wire */
boolean Trill::requestRawData(uint8_t max_length) {
	uint8_t length = 0;

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
		wire_->requestFrom(i2c_address_, length);
		raw_bytes_left_ = 0;
	}
	else {
		int ret = wire_->requestFrom(i2c_address_, (uint8_t)BUFFER_LENGTH);
		if(ret > 0)
			raw_bytes_left_ = length - ret;
		else {
			// failed transmission. Device died?
			raw_bytes_left_ = 0;
			return false;
		}
	}
	return true;
}

int Trill::rawDataAvailable() {
	/* Raw data items are 2 bytes long; return number of them available */
	return ((wire_->available() + raw_bytes_left_) >> 1);
}

/* Raw data is in 16-bit big-endian format */
int Trill::rawDataRead() {

	if(wire_->available() < 2) {
		/* Read more bytes if we need it */
		if(raw_bytes_left_ > 0) {
			/* Move read pointer on device */
			wire_->beginTransmission(i2c_address_);
			wire_->write(kOffsetData + BUFFER_LENGTH);
			wire_->endTransmission();
			last_read_loc_ = kOffsetData + BUFFER_LENGTH;

			/* Now gather what's left */
			wire_->requestFrom(i2c_address_, raw_bytes_left_);
			raw_bytes_left_ = 0;
		}

		/* Check again if we've got anything... */
		if(wire_->available() < 2)
			return 0;
	}

	int result = ((uint8_t)wire_->read()) << 8;
	result += (int)wire_->read();
	return result;
}

/* Scan configuration settings */
void Trill::setMode(Mode mode) {
	wire_->beginTransmission(i2c_address_);
	wire_->write(kOffsetCommand);
	wire_->write(kCommandMode);
	wire_->write(mode);
	wire_->endTransmission();

	mode_ = mode;
	last_read_loc_ = kOffsetCommand;
	num_touches = 0;
}

void Trill::setScanSettings(uint8_t speed, uint8_t num_bits) {
	if(speed > 3)
		speed = 3;
	if(num_bits < 9)
		num_bits = 9;
	if(num_bits > 16)
		num_bits = 16;
	wire_->beginTransmission(i2c_address_);
	wire_->write(kOffsetCommand);
	wire_->write(kCommandScanSettings);
	wire_->write(speed);
	wire_->write(num_bits);
	wire_->endTransmission();

	last_read_loc_ = kOffsetCommand;
}

void Trill::setPrescaler(uint8_t prescaler) {
	wire_->beginTransmission(i2c_address_);
	wire_->write(kOffsetCommand);
	wire_->write(kCommandPrescaler);
	wire_->write(prescaler);
	wire_->endTransmission();

	last_read_loc_ = kOffsetCommand;
}

void Trill::setNoiseThreshold(uint8_t threshold) {
	wire_->beginTransmission(i2c_address_);
	wire_->write(kOffsetCommand);
	wire_->write(kCommandNoiseThreshold);
	wire_->write(threshold);
	wire_->endTransmission();

	last_read_loc_ = kOffsetCommand;
}

void Trill::setIDACValue(uint8_t value) {
	wire_->beginTransmission(i2c_address_);
	wire_->write(kOffsetCommand);
	wire_->write(kCommandIdac);
	wire_->write(value);
	wire_->endTransmission();

	last_read_loc_ = kOffsetCommand;
}

void Trill::setMinimumTouchSize(uint16_t size) {
	wire_->beginTransmission(i2c_address_);
	wire_->write(kOffsetCommand);
	wire_->write(kCommandMinimumSize);
	wire_->write(size >> 8);
	wire_->write(size & 0xFF);
	wire_->endTransmission();

	last_read_loc_ = kOffsetCommand;
}

void Trill::setAutoScanInterval(uint16_t interval) {
	wire_->beginTransmission(i2c_address_);
	wire_->write(kOffsetCommand);
	wire_->write(kCommandAutoScanInterval);
	wire_->write(interval >> 8);
	wire_->write(interval & 0xFF);
	wire_->endTransmission();

	last_read_loc_ = kOffsetCommand;
}

/* Prepare the device to read data if it is not already prepared */
void Trill::prepareForDataRead() {
	if(last_read_loc_ != kOffsetData) {
		wire_->beginTransmission(i2c_address_);
		wire_->write(kOffsetData);
		wire_->endTransmission();

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

	return buffer_[2 * MAX_TOUCH_1D_OR_2D + button_num];
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
		case TRILL_FLEX:
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

uint8_t Touches::getNumTouches() const
{
	return num_touches;
}

int Touches::touchLocation(uint8_t touch_num) const
{
	if(touch_num < num_touches)
		return centroids[touch_num];
	else
		return -1;
}

int Touches::touchSize(uint8_t touch_num) const
{
	if(touch_num < num_touches)
		return sizes[touch_num];
	else
		return -1;
}

unsigned int Touches2D::getNumHorizontalTouches() {
	return horizontal.getNumTouches();
}

void Touches::processCentroids(uint8_t maxCentroids) {
	// Look for 1st instance of 0xFFFF (no touch) in the buffer
	for(num_touches = 0; num_touches < maxCentroids; ++num_touches)
	{
		if(0xffff == centroids[num_touches])
			break;// at the first non-touch, break
	}
	// now num_touches is the number of active touches in the array
}

/* These methods for horizontal touches on 2D sliders */
int Touches2D::touchHorizontalLocation(uint8_t touch_num) {
	return horizontal.touchLocation(touch_num);
}

int Touches2D::touchHorizontalSize(uint8_t touch_num) {
	return horizontal.touchSize(touch_num);
}
