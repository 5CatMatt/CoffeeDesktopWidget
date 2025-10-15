#ifndef MY_DEFINES_H
#define MY_DEFINES_H

#include <CenturyGothic24.h>
#include <CenturyGothic60.h>
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

uint8_t selectedPage = pageGreenRing;
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
const uint8_t batteryPin = 35;
const uint8_t motionSensePin = 34;
#define BUTTON_PIN GPIO_NUM_14          // Pin 13 - PWR_PB Sense button press

const uint8_t powerEnable = 26;     // Pin 11 - P_ENA
const uint8_t valveEnablePin = 17;  // Pin 28 - VALVE
const uint8_t pumpEnablePin = 25;   // Pin 10 - PUMP

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

// #define ACTIVE_LOW
#ifdef ACTIVE_LOW
  const uint8_t pinActivate = LOW;
  const uint8_t pinDeactivate = HIGH;
#else
  const uint8_t pinActivate = HIGH;
  const uint8_t pinDeactivate = LOW;
#endif

// Thermistor
#define SENSOR_PIN             34
#define REFERENCE_RESISTANCE   9998
#define NOMINAL_RESISTANCE     10000
#define NOMINAL_TEMPERATURE    25
#define B_VALUE                3900 // or 3900
#define ESP32_ANALOG_RESOLUTION 4095

double readTemp = 0;

// Lookup table for voltage to percentage conversion
const int numPoints = 21;
const float voltageTable[numPoints] = {4.20, 4.15, 4.10, 4.05, 4.00, 3.95, 3.90, 3.85, 3.80, 3.75, 3.70, 3.65, 3.60, 3.55, 3.50, 3.45, 3.40, 3.35, 3.30, 3.25, 3.20};
const int percentageTable[numPoints] = {100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 45, 40, 35, 30, 25, 20, 15, 10, 5, 0};
int batteryPercentage = 0;
float batteryVoltage = 0;
const float calibrationFactor = 1.116;
const float adcReferenceVoltage = 3.3;
const int adcResolution = 4095; // 12-bit ADC resolution
const float R1 = 100000.0; // 100k ohms
const float R2 = 100000.0; // 100k ohms

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