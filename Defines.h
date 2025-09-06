#ifndef MY_DEFINES_H
#define MY_DEFINES_H

#include <CenturyGothic24.h>
// #include <CenturyGothic60.h>
// #include <AlarmClock60.h> // no special chars, only nums
// #include <AlarmClockRegular24.h>

// #include <Segment760.h> // all chars but not very close to old watch
// #include <DigitalDisplayRegular60.h> // no special chars, only nums
// #include <DigitalDisplayRegular76.h>
// #include <Segment76.h> // some special chars but poor small size
// #include <Segment34.h>
// #include <Segment24.h>
// #include <Segment755.h>
// #include <Segment769.h>

#define SDA 23
#define SCL 22
#define RST 33
#define INT 32

// Pages to display, creates navigation via switch
const uint8_t pageBlueRing = 0;
const uint8_t pageGreenRing = 1;
const uint8_t pageGreen = 2;
const uint8_t pageEyePulse = 3;
const uint8_t pageOrbitPulse = 4;
const uint8_t pageEyePulsePause = 5;

uint8_t selectedPage = pageEyePulse;
uint8_t maxPageNumber = 5;
bool displayUpdate = true;  // Only draw the entire tft when the page needs to be wiped

// LCD colors
// const uint16_t colorCalmBlue = 0x02F5;
// const uint16_t colorLightBlue = 0x3BDB;
// const uint16_t colorMossGreen = 0x4B28;
// const uint16_t colorWhite = 0xFFFF;
// const uint16_t colorBlack = 0x0000;

// uint16_t colorBackground = colorCalmBlue;
// uint16_t colorText = colorWhite;

// LCD constants
const int centerX = 120;    // Center X of the display
const int centerY = 120;    // Center Y of the display
// const int maxRadius = 120;  // Maximum visible radius of the round display, waveshare 1.28"
// const int ringWidth = 6;    // Width of the ring

// GPIO
const uint8_t powerEnable = 26;
const uint8_t batteryPin = 35;
const uint8_t motionSensePin = 34;
#define BUTTON_PIN GPIO_NUM_14          // Pin 13 - PWR_PB Sense button press

// const uint16_t backlightLevel = 255;    // Pin defined in tft_espi library

#define LED_PIN GPIO_NUM_4
#define LED_COUNT 4
// uint8_t ledBrightness = 20;

// Touchscreen items - slow down response to limit multitouch events
bool touchNavDebounce = false;
bool touchNavEnabled = false;
unsigned long previousMillisTouchEvent = 0;
uint16_t touchNavigationDebounceTime = 150;

// Power button items
bool buttonLow = true;
bool sustainedPress = false;
unsigned long highStartTime = 0;
bool sleeping = false; // Track if the device is sleeping
#define SUSTAINED_HIGH_DURATION 2000  // Duration in ms for sustained high signal

// Bluetooth

// LED items
// unsigned long pixelPrevious = 0;        // Previous Pixel Millis
// unsigned long patternPrevious = 0;      // Previous Pattern Millis
// int           patternCurrent = 0;       // Current Pattern Number
// int           patternInterval = 5000;   // Pattern Interval (ms)
// bool          patternComplete = false;

// int           pixelInterval = 50;       // Pixel Interval (ms)
// int           pixelQueue = 0;           // Pattern Pixel Queue
// int           pixelCycle = 0;           // Pattern Pixel Cycle
// uint16_t      pixelNumber = LED_COUNT;  // Total Number of Pixels

// unsigned long lastColorUpdate = 0;
// const unsigned long colorUpdateInterval = 150; // ms, adjust for speed
// float colorWheelPos = 0.0; // 0-360
// const float colorStep = 90.0; // 360/4 = 90deg separation for 4 LEDs


#endif // MY_DEFINES_H