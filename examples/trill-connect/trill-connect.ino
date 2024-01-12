/*
trill-connect: a Swiss-army knife sketch for Trill connectivity.

Connect your Trill to your computer, handheld or analog equipment via an ESP32-S3 board.
This was deesigned for the ESP32-S3 and tested on ESP32-S3-DEVKIT-C1 v1.0, selecting the "Adafruit
Feather ESP32-S3 No PSRAM" board (adafruit_feather_esp32s3_nopsram) in the Arduino-IDE. It should
work with minimal (e.g.: pin numbers only) modifications on most ESP32-S3 boards and it may work
to some extent on other boards with USB and/or BLE support.

Connect a Square and/or a Bar using external pull-ups and/or a Trill Hub, using pins 47 for SDA
and 21 for SCL.
Available features:
- USB_MOUSE/BLE_MOUSE: the board shows up on the host as a USB and/or BLE mouse, respectively.
The Square acts as a trackpad, while the Bar acts as a scroll wheel. There is no support for
clicks (it's complicated without a pressure sensor or a dedicated button!).
- USB_MIDI/BLE_MIDI: the board shows up on the host as a USB and/or BLE MIDI device,
respectively. High-resolution (14-bit, MSB + LSB) control change messages are send via USB,
while for BLE we use 7-bit (MSB only) CCs in order to reduce the bandwidth. The enclosed PureData
patch receives, interprets and labels the high-precision CC values. This sends:
  - square x position on CC 5 (MSB) and CC 37 (LSB)
  - square y position on CC 6 (MSB) and CC 38 (LSB)
  - square touch size on CC 7 (MSB) and CC 39 (LSB)
  - bar position on CC 8 (MSB) and CC 40 (LSB)
  - bar touch size on CC 9 (MSB) and CC 41 (LSB)
- ANALOG_OUT: sends touch information to the "analog" (actually PWM) outputs using the parameters
set in `pwmBaseFreq` and `pwmResolutionBits`, starting from the GPIO channel set in `pwmFirstGpio`.
With the default settings (39kHz carrier, 10 bit), a passive low-pass RC filter using a 10k
resistor and 22n capacitor already gives good results. The order of the outputs (starting from
`pwmFirstGpio`) is:
  - square x position (default: 35)
  - square y position (default: 36)
  - square touch size (default: 37)
  - bar position (default: 38)
  - bar touch size (default: 39)

Enable individual features below. Simultaneous use of most of them is allowed, with the only exception being
that BLE_MOUSE and BLE_MIDI which are mutually exclusive.
Note that enabling and using one interface may slow down the performance of other interfaces, as they are all
accessed from the same loop.
*/

#define USB_MOUSE  // uses "Adafruit TinyUSB Library"
#define USB_MIDI   // uses "Adafruit TinyUSB Library" and "MIDI Library"
#define BLE_MOUSE // requires manual install of github.com/T-vK/ESP32-BLE-Mouse
// #define BLE_MIDI  // uses "ESP32-BLE-MIDI" and "MIDI Library"
#define ANALOG_OUT

#if defined(BLE_MOUSE) && defined(BLE_MIDI)
#error BLE_MOUSE and BLE_MIDI are not allowed together
// this is because ESP32-BLE-Mouse uses the core BLE (BLEDevice.h, BLEUtils.h, etc) while
#endif

#ifdef USB_MOUSE
#include "Adafruit_TinyUSB.h"
// HID report descriptor using TinyUSB's template
// Single Report (no ID) descriptor
uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_MOUSE()
};
// USB HID object. For ESP32 these values cannot be changed after this declaration
// desc report, desc len, protocol, interval, use out endpoint
Adafruit_USBD_HID mouse(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_MOUSE, 2, false);
#endif  // USB_MOUSE

#ifdef BLE_MOUSE
#include <BleMouse.h>
BleMouse bleMouse("Trill BLE Mouse", "Augmented Instruments Ltd");
#endif  // BLE_MOUSE
#ifdef BLE_MIDI
#include <BLEMidi.h>
#endif  // BLE_MIDI

void mouseBegin() {
#ifdef USB_MOUSE
  mouse.begin();
#endif  // USB_MOUSE
#ifdef BLE_MOUSE
  bleMouse.begin();
#endif  // BLE_MOUSE
}

void mouseMove(int8_t x, int8_t y, int8_t scroll) {
#ifdef USB_MOUSE
  if (TinyUSBDevice.mounted())
    mouse.mouseReport(0, 0, x, y, scroll, 0);
#endif  // USB_MOUSE
#ifdef BLE_MOUSE
  if (bleMouse.isConnected())
    bleMouse.move(x, y, scroll);
#endif  // BLE_MOUSE
}

#ifdef USB_MIDI
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>
Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);
#endif  // USB_MIDI

void midiBegin() {
#ifdef BLE_MIDI
  BLEMidiServer.begin("Trill MIDI");
  // BLEMidiServer.enableDebugging();  // Uncomment if you want to see some debugging output from the library (not much for the server class...)
#endif  // BLE_MIDI
}

void controlChangeHighRes(byte channel, byte control, unsigned value) {
  uint8_t msb = (value >> 7) & 0x7f;
#ifdef USB_MIDI
  if (TinyUSBDevice.mounted()) {
    // send a high-resolution (14-bit) CC message:
    // (note that the MIDI class API numbers channels from 1:16 instead of 0:15)
    uint8_t lsb = value & 0x7f;
    MIDI.sendControlChange(control, lsb, channel + 1 + 32);  // send LSB at CC + 32
    MIDI.sendControlChange(control, msb, channel + 1);       // send MSB
  }
#endif  // USB_MIDI
#ifdef BLE_MIDI
  // send regular-resolution (7-bit) CC to save bandwidth
  if (BLEMidiServer.isConnected()) {
    BLEMidiServer.controlChange(channel, control, msb);
  }
#endif  // BLE_MIDI
}

#ifdef ANALOG_OUT
uint32_t pwmBaseFreq = 39000;
constexpr uint32_t pwmResolutionBits = 10;
unsigned int pwmFirstGpio = 35;

// Arduino-like analogWrite (i.e.: PWM)
// value has to be between 0 and ((1 << pwmResolutionBits) - 1)
void analogOut(uint8_t channel, uint32_t value) {
#ifdef ANALOG_OUT
  uint32_t duty = min(value, uint32_t((1 << pwmResolutionBits) - 1));
  ledcWrite(channel, duty);
#endif  // ANALOG_OUT
}
#endif

#include <Trill.h>
Trill bar;
Trill square;

constexpr unsigned RGB_LED_GPIO = 48;  // On ESP32-S3-DevKit-C1 v1.0
void setup() {
  Serial.begin(115200);
  pinMode(RGB_LED_GPIO, OUTPUT);
  neopixelWrite(RGB_LED_GPIO, 0, 0, 255);
  // give the USB host enough time to connect to the sw serial port so no log is lost
  delay(5000);
  neopixelWrite(RGB_LED_GPIO, 0, 255, 0);
  Serial.println("SETUP");
  int ret;
  // set pins foir I2C: 47 is SDA and 21 is SCL
  ret = Wire.setPins(47, 21);
  ret = bar.setup(Trill::TRILL_BAR);
  Serial.printf("bar.setup() returned %d\n\r", ret);
  ret = square.setup(Trill::TRILL_SQUARE);
  Serial.printf("square.setup() returned %d\n\r", ret);
  mouseBegin();
  midiBegin();
#ifdef ANALOG_OUT
  for (unsigned n = 0; n < 5; ++n) {
    // set up the PWM channels
    int actualFreq = ledcSetup(n, pwmBaseFreq, pwmResolutionBits);
    Serial.printf("Requested freq %d on channel %d, obtained %d\n\r", pwmBaseFreq, n, actualFreq);
    ledcAttachPin(pwmFirstGpio + n, n);
  }
#endif  // ANALOG_OUT
  neopixelWrite(RGB_LED_GPIO, 0, 0, 0);
}

int pastMillis = 0;
void loop() {
  // do not scan Trills more often than once every 7 ms
  while (millis() - pastMillis < 7)
    yield();
  pastMillis = millis();
  bar.read();
  square.read();

  int scroll = 0;
  int x = 0;
  int y = 0;
  static unsigned long pastBarMs = millis();
  static int barHadTouch = 0;
  int barHasTouch = bar.getNumTouches();
  if (millis() - pastBarMs > 100) {  // send scroll every 100 ms
    if (barHasTouch && bar.touchSize(0) > 500) {
      static int pastBar = bar.touchLocation(0);
      int val = bar.touchLocation(0);
      if (barHadTouch) {
        pastBarMs = millis();
        int diff = val - pastBar;
        if (abs(diff) > 20)
          scroll = diff > 0 ? 1 : -1;
      }
      pastBar = val;
    }
  }
  if (barHasTouch || barHadTouch) {
    unsigned int pos = 0;
    unsigned int size = 0;
    if (barHasTouch) {
      pos = bar.touchLocation(0);
      // remap to cover the full 14-bit range
      pos = map(pos, 0, 128 * 25, 0, 16383);
      pos = constrain(pos, 0, 16383);
      // enlarge to size to cover enough of the 14-bit range
      size = bar.touchSize(0) * 4;
      size = constrain(size, 0, 16383);
    }
    controlChangeHighRes(0, 8, pos);
    controlChangeHighRes(0, 9, size);
#ifdef ANALOG_OUT
    analogOut(3, pos >> (14 - pwmResolutionBits));
    analogOut(4, size >> (14 - pwmResolutionBits));
#endif  // ANALOG_OUT
  }
  barHadTouch = barHasTouch;

  static int squareHadTouch = 0;
  int squareHasTouch = square.getNumTouches();
  int thisX = 0;
  int thisY = 0;
  if (squareHasTouch && square.touchSize(0) > 500) {
    static int pastSquareX;
    static int pastSquareY;
    thisX = square.touchHorizontalLocation(0);
    thisY = square.touchLocation(0);
    if (squareHadTouch) {
      // only consider movement if we have a valid past value
      x = (thisX - pastSquareX) / 2;
      y = -(thisY - pastSquareY) / 2;  // - to fix the orientation so that it matches the screen
    }
    pastSquareX = thisX;
    pastSquareY = thisY;
  }
  if (squareHasTouch || squareHadTouch) {
    unsigned int xcc = 0;
    unsigned int ycc = 0;
    unsigned int size = 0;
    if (squareHasTouch) {
      // remap coordinates to cover the full 14-bit range
      xcc = map(thisX, 256, 128 * 14, 0, 16383);
      xcc = constrain(xcc, 0, 16383);
      ycc = map(thisY, 256, 128 * 14, 0, 16383);
      ycc = constrain(ycc, 0, 16383);
      // enlarge to size to cover a reasonable portion of the 14-bit range
      size = square.touchSize(0) * 4;
    }
    controlChangeHighRes(0, 5, xcc);
    controlChangeHighRes(0, 6, ycc);
    controlChangeHighRes(0, 7, size);
#ifdef ANALOG_OUT
    analogOut(0, xcc >> (14 - pwmResolutionBits));
    analogOut(1, ycc >> (14 - pwmResolutionBits));
    analogOut(2, size >> (14 - pwmResolutionBits));
#endif  // ANALOG_OUT
  }
  squareHadTouch = squareHasTouch;

  if (x || y || scroll) {
    mouseMove(x, y, scroll);
  }
  neopixelWrite(RGB_LED_GPIO, !(x || y || scroll) * 10, (x || y) * 120, !!scroll * 120);
}
