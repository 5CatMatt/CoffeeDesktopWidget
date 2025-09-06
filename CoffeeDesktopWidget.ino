/*

Desktop widget

Target microcontroller is an ESP-32-WROOM

Battery voltage is available on hardware.

Burn-in may be an issue, multiple display pages will be created and will cycle 
automatically to lessen negative effect.

*/


// #include <WiFi.h>
// #include <ArduinoOTA.h>

#include <Defines.h>
#include <Secrets.h>

// Graphics
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

TFT_eSprite centerText = TFT_eSprite(&tft);
TFT_eSprite centerSubText = TFT_eSprite(&tft);
TFT_eSprite upperSubText = TFT_eSprite(&tft);

// === GLOBAL GUI VARIABLES WITH SERIAL KEYS ===
uint16_t colorBackground = 0x02F5; // Default: colorCalmBlue, KEY: BGC
uint16_t colorText = 0xFFFF;       // Default: colorWhite,    KEY: TXT
uint16_t colorMossGreen = 0x4B28;  // KEY: MOS
uint16_t colorCalmBlue = 0x02F5;   // KEY: CBL
uint16_t colorWhite = 0xFFFF;      // KEY: WHT
uint16_t colorPulseDraw = 0x02F5;    // KEY: CPD
int maxRadius = 120;               // KEY: MRD
int ringWidth = 12;                // KEY: RNW
float colorWheelPos = 0.0;         // KEY: CWP
float colorStep = 90.0;            // KEY: CST
unsigned long colorUpdateInterval = 100; // ms, KEY: CUI
int ledBrightness = 32;            // KEY: LBR
int backlightLevel = 128;          // KEY: BLT
float pulseSpeedGreen = 400.0;     // KEY: PSG
float pulseSpeedEye = 420.0;       // KEY: PSE
int eyePulseMaxOffset = 10;        // KEY: EPM
int eyePulseMinR = 3;              // KEY: EPMR
float pulseSpeedOrbit = 200.0;     // KEY: PSO
int minBuffer = 5;                 // KEY: MNB
int maxBuffer = 5;                 // KEY: MXB
unsigned long pauseDuration = 1000; // ms, KEY: PAD
// New pixel/pattern variables
unsigned long pixelPrevious = 0;        // Previous Pixel Millis, KEY: PXP
unsigned long patternPrevious = 0;      // Previous Pattern Millis, KEY: PTP
int           patternCurrent = 0;       // Current Pattern Number, KEY: PTN
int           patternInterval = 5000;   // Pattern Interval (ms), KEY: PTI
bool          patternComplete = false;  // KEY: PTC
int           pixelInterval = 50;       // Pixel Interval (ms), KEY: PXI
int           pixelQueue = 0;           // Pattern Pixel Queue, KEY: PXQ
int           pixelCycle = 0;           // Pattern Pixel Cycle, KEY: PXC
uint16_t      pixelNumber = LED_COUNT;  // Total Number of Pixels, KEY: PXN
unsigned long lastColorUpdate = 0;      // KEY: LCU

// Touchscreen
#include <CST816S.h>
CST816S touch(SDA, SCL, RST, INT);

#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Function prototype for FreeRTOS task
void MonitorTask(void *pvParameters);

void setup() {
  Serial.println("EPM: eyePulseMaxOffset (int, offset from maxRadius)");
  Serial.println("EPMR: eyePulseMinR (int, min radius)");
  Serial.begin(115200);
  Serial.println("\n=== Coffee Desktop Widget ===");

  Serial.println("PXP: pixelPrevious (ms, unsigned long)");
  Serial.println("PTP: patternPrevious (ms, unsigned long)");
  Serial.println("PTN: patternCurrent (int)");
  Serial.println("PTI: patternInterval (ms, int)");
  Serial.println("PTC: patternComplete (0/1, bool)");
  Serial.println("PXI: pixelInterval (ms, int)");
  Serial.println("PXQ: pixelQueue (int)");
  Serial.println("PXC: pixelCycle (int)");
  Serial.println("PXN: pixelNumber (uint16_t)");
  Serial.println("LCU: lastColorUpdate (ms, unsigned long)");


  // === SERIAL HELP FOR GUI VARIABLES ===
  Serial.println("\n=== GUI Variable Serial Control ===");
  Serial.println("Use: set <KEY> <value> (e.g. set MRD 90)");
  Serial.println("BGC: colorBackground (hex, e.g. 0x20C4)");
  Serial.println("TXT: colorText (hex)");
  Serial.println("MOS: colorMossGreen (hex)");
  Serial.println("CBL: colorCalmBlue (hex)");
  Serial.println("WHT: colorWhite (hex)");
  Serial.println("CPD: colorPulseDraw (hex)");
  Serial.println("MRD: maxRadius (int)");
  Serial.println("RNW: ringWidth (int)");
  Serial.println("CWP: colorWheelPos (float)");
  Serial.println("CST: colorStep (float)");
  Serial.println("CUI: colorUpdateInterval (ms)");
  Serial.println("LBR: ledBrightness (0-255)");
  Serial.println("BLT: backlightLevel (0-255)");
  Serial.println("PSG: pulseSpeedGreen (float, ms)");
  Serial.println("PSE: pulseSpeedEye (float, ms)");
  Serial.println("PSO: pulseSpeedOrbit (float, ms)");
  Serial.println("MNB: minBuffer (int)");
  Serial.println("MXB: maxBuffer (int)");
  Serial.println("PAD: pauseDuration (ms)");
  Serial.println("Type 'help' for this list again.\n");
  

  // setup GPIO
  pinMode(powerEnable, OUTPUT);
  digitalWrite(powerEnable, HIGH);  // LOW state disables the esp32 EN pin resulting in full shutdown

  // Set the wakeup pin as an input
  pinMode(BUTTON_PIN, INPUT);
  gpio_pullup_en(BUTTON_PIN);
  esp_sleep_enable_ext0_wakeup(BUTTON_PIN , 0);  // Button pulls low on press

  // Create a task to monitor the wakeup pin
  xTaskCreate(MonitorTask, "MonitorTask", 2048, NULL, 1, NULL);

  // Enable draw and touch
  SetupLCD();


  // WiFi and OTA features are disabled to save memory
  // WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.println(" CONNECTED");
  // SetupOTA();

  // Init NeoPixel strip, 4 px ring in this case
  strip.begin(); 
  strip.show();
  strip.setBrightness(ledBrightness);
}

void loop() {
  
  // SERIAL INTERFACE for all GUI variables
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.equalsIgnoreCase("help")) {
      setup(); // Print help again
    } else if (input.startsWith("set ")) {
      int keyStart = 4;
      String key = input.substring(keyStart, keyStart+3);
      String valStr = input.substring(keyStart+4);
      valStr.trim();
      if (key == "PXP") pixelPrevious = valStr.toInt();
      else if (key == "PTP") patternPrevious = valStr.toInt();
      else if (key == "PTN") patternCurrent = valStr.toInt();
      else if (key == "PTI") patternInterval = valStr.toInt();
      else if (key == "PTC") patternComplete = (valStr.toInt() != 0);
      else if (key == "PXI") pixelInterval = valStr.toInt();
      else if (key == "PXQ") pixelQueue = valStr.toInt();
      else if (key == "PXC") pixelCycle = valStr.toInt();
      else if (key == "PXN") pixelNumber = valStr.toInt();
      else if (key == "LCU") lastColorUpdate = valStr.toInt();
      else if (key == "BGC") colorBackground = (uint16_t)strtol(valStr.c_str(), NULL, 0);
      else if (key == "TXT") colorText = (uint16_t)strtol(valStr.c_str(), NULL, 0);
      else if (key == "MOS") colorMossGreen = (uint16_t)strtol(valStr.c_str(), NULL, 0);
      else if (key == "CBL") colorCalmBlue = (uint16_t)strtol(valStr.c_str(), NULL, 0);
      else if (key == "WHT") colorWhite = (uint16_t)strtol(valStr.c_str(), NULL, 0);
      else if (key == "CPD") colorPulseDraw = (uint16_t)strtol(valStr.c_str(), NULL, 0);
      else if (key == "MRD") maxRadius = valStr.toInt();
      else if (key == "RNW") ringWidth = valStr.toInt();
      else if (key == "CWP") colorWheelPos = valStr.toFloat();
      else if (key == "CST") colorStep = valStr.toFloat();
      else if (key == "CUI") colorUpdateInterval = valStr.toInt();
      else if (key == "LBR") { ledBrightness = valStr.toInt(); strip.setBrightness(ledBrightness); strip.show(); }
      else if (key == "BLT") { backlightLevel = valStr.toInt(); analogWrite(BACKLIGHT, backlightLevel); }
      else if (key == "PSG") pulseSpeedGreen = valStr.toFloat();
      else if (key == "PSE") pulseSpeedEye = valStr.toFloat();
      else if (key == "PSO") pulseSpeedOrbit = valStr.toFloat();
      else if (key == "MNB") minBuffer = valStr.toInt();
      else if (key == "MXB") maxBuffer = valStr.toInt();
      else if (key == "PAD") pauseDuration = valStr.toInt();
      else if (key == "EPM") eyePulseMaxOffset = valStr.toInt();
      else if (key == "EPMR") eyePulseMinR = valStr.toInt();
      else Serial.println("Unknown key. Type 'help' for list.");
      Serial.print("Set "); Serial.print(key); Serial.print(" to "); Serial.println(valStr);
    } else {
      Serial.println("Unknown command. Type 'help' for list.");
    }
  }
  // Network items
  // ArduinoOTA.handle();

  // Handle touch events
  NavigationDebounce();

  if (touch.available()) {
    if (touchNavEnabled) {
      selectedPage++;
      if (selectedPage > maxPageNumber) { selectedPage = 0; }
        displayUpdate = true;
        touchNavEnabled = false;
    }
  }

  // Page navigation
  PageNavigation();

  // colorWipe(strip.Color(0, 0, 255), 50); // Blue
  // colorWipe(strip.Color(0, 255, 0), 50); // Green
  // colorWipe(strip.Color(255, 0, 0), 50); // Red

  HandleLED();

}

// **************** Draw Functions - Pages **************** //

void PageNavigation() {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();
  unsigned long frameInterval = 33; // Default ~30 FPS

  switch (selectedPage) {
    case pageBlueRing:
      frameInterval = 42;
      break;
    case pageGreenRing:
      frameInterval = 33;
      break;
    case pageGreen:
      frameInterval = 20;
      break;
    case pageEyePulse:
      frameInterval = 42;
      break;
    case pageOrbitPulse:
      frameInterval = 16;
      break;
    case pageEyePulsePause:
      frameInterval = 42;
      break;
    default:
      selectedPage = pageBlueRing;
      frameInterval = 16;
      break;
  }


  if (now - lastUpdate < frameInterval) return;
    lastUpdate = now;

  switch (selectedPage) {
    case pageBlueRing:
      DrawBlueRingPage();
      break;
    case pageGreenRing:
      DrawGreenRingPage();
      break;
    case pageGreen:
      DrawGreenPage();
      break;
    case pageEyePulse:
      DrawEyePulsePage();
      break;
    case pageEyePulsePause:
      DrawEyePulsePausePage();
      break;
    case pageOrbitPulse:
      DrawOrbitPulsePage();
      break;
    default:
      DrawBlueRingPage();
      break;
  }
}

// **************** Pages **************** //

void DrawBlueRingPage() {
  // Blue page with white outter ring and centered text elements
  colorBackground = colorCalmBlue;
  colorText = colorWhite;

  if (displayUpdate) {
    displayUpdate = false;
    tft.fillScreen(colorCalmBlue);
    tft.drawSmoothArc(centerX, centerY, maxRadius, maxRadius - ringWidth, 0, 360, colorWhite, colorBackground);
  }

  static int progress = 0;
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if (now - lastUpdate >= 550) {
    progress += 1;
    if (progress > 100) progress = 0;
    lastUpdate = now;

    DrawAnimatedProgressBar(centerX - 60, centerY - 8, 120, 16, progress, 100, TFT_GREEN, TFT_DARKGREY);
  }
  // drawCenteredTextSprite(centerText, currentTemperature + "°", CenturyGothic60, 40, 90, MC_DATUM);
  // drawCenteredTextSprite(centerSubText, currentHumidity + "%", CenturyGothic24, 80, 150, MC_DATUM);
}

void DrawGreenRingPage() {
  // Green page with white outter ring and centered text elements
  colorBackground = colorMossGreen;
  colorText = colorWhite;

  if (displayUpdate) {
    displayUpdate = false;

    tft.fillScreen(colorMossGreen);

    tft.drawSmoothArc(centerX, centerY, maxRadius, maxRadius - ringWidth, 0, 360, colorWhite, colorBackground);
  }

  // drawCenteredTextSprite(centerSubText, batteryVoltage, CenturyGothic24, 80, 42, MC_DATUM);

  // drawCenteredTextSprite(centerText, currentTemperature + "°", CenturyGothic60, 40, 90, MC_DATUM);

  // drawCenteredTextSprite(centerSubText, currentHumidity + "%", CenturyGothic24, 80, 150, MC_DATUM);
  
}

void DrawGreenPage() {
  static unsigned long startTime = millis();
  unsigned long now = millis();
  float pulse = (sin((now - startTime) / pulseSpeedGreen) + 1.0) * 0.5; // 0..1

  tft.fillScreen(colorMossGreen);

  int cx = centerX;
  int cy = centerY;
  int maxR = maxRadius - 10;
  int minR = 30;
  int r = minR + (int)((maxR - minR) * pulse);

  tft.drawCircle(cx, cy, r, TFT_GREEN);
  tft.drawCircle(cx, cy, r / 2, TFT_WHITE);
  // tft.fillCircle(cx, cy, r / 4, TFT_YELLOW);
}

void DrawEyePulsePage(){
  float inversePulseSpeed = pulseSpeedEye; // Adjust for speed, higher = slower pulse
  static unsigned long startTime = millis();
  static int lastR = 0;
  unsigned long now = millis();
  float pulse = (sin((now - startTime) / inversePulseSpeed) + 1.0) * 0.5;

  int cx = centerX;
  int cy = centerY;
  int maxR = maxRadius - eyePulseMaxOffset;
  int minR = eyePulseMinR;
  int circleRadius = minR + (int)((maxR - minR) * pulse);

  // Erase previous frame by overdrawing with background
  tft.fillCircle(cx, cy, lastR, colorMossGreen);

  // Draw new frame
  tft.drawCircle(cx, cy, circleRadius, colorPulseDraw);
  // tft.drawCircle(cx, cy, r / 2, TFT_WHITE);
  // tft.fillCircle(cx, cy, r / 4, TFT_YELLOW);

  lastR = circleRadius;
}

void DrawEyePulsePausePage() {
  // minBuffer, maxBuffer, pauseDuration are now global
  static unsigned long startTime = millis();
  static int lastR = 0;
  static bool paused = false;
  static unsigned long pauseStart = 0;
  static bool atMax = false, atMin = false;

  unsigned long now = millis();
  float inversePulseSpeed = 4.2;
  int cx = centerX;
  int cy = centerY;
  int maxR = maxRadius - 10 - maxBuffer;
  int minR = 3 + minBuffer;
  float pulse = (sin((now - startTime) / inversePulseSpeed) + 1.0) * 0.5;
  int r = minR + (int)((maxR - minR) * pulse);

  // Detect if at min or max
  atMax = (r >= maxR);
  atMin = (r <= minR);

  if (!paused && (atMax || atMin)) {
    paused = true;
    pauseStart = now;
  }

  if (paused) {
    // Hold at min or max radius
    r = atMax ? maxR : minR;
    if (now - pauseStart >= pauseDuration) {
      paused = false;
      // Reset startTime so pulse resumes smoothly
      startTime = now - (atMax ? (inversePulseSpeed * 3.14159 / 2) : 0);
    }
  }

  // Erase previous frame by overdrawing with background
  tft.fillCircle(cx, cy, lastR, colorMossGreen);

  // Draw new frame
  tft.drawCircle(cx, cy, r, TFT_GREEN);

  lastR = r;
}

void DrawOrbitPulsePage(){
  static unsigned long startTime = millis();
  static int lastR = 0;
  unsigned long now = millis();
  float pulse = (sin((now - startTime) / pulseSpeedOrbit) + 1.0) * 0.5; // Even faster pulse

  int cx = centerX;
  int cy = centerY;
  int maxR = maxRadius - 10;
  int minR = 30;
  int r = minR + (int)((maxR - minR) * pulse);

  // Erase previous frame by overdrawing with background
  tft.fillCircle(cx, cy, lastR + 4, colorMossGreen);

  // Animated highlight angle
  float highlightAngle = fmod((now - startTime) / 3.0, 360.0);
  float rad = highlightAngle * 3.14159 / 180.0;
  int hx = cx + (int)(r * 0.7 * cos(rad));
  int hy = cy + (int)(r * 0.7 * sin(rad));

  // Draw outer pulsing ring with color cycling
  for (int i = 0; i < 3; ++i) {
    float ringPulse = (sin((now - startTime) / (200.0 + i * 80)) + 1.0) * 0.5;
    int rr = r - i * 8;
    uint32_t color = ColorFromHSV(fmod(highlightAngle + i * 60, 360.0), 1.0, 1.0 - i * 0.25);
    tft.drawCircle(cx, cy, rr, color);
  }

  // Draw main white eye ring
  tft.drawCircle(cx, cy, r / 2, TFT_WHITE);

  // Draw glowing yellow iris
  tft.fillCircle(cx, cy, r / 4, TFT_YELLOW);
  tft.drawCircle(cx, cy, r / 4 + 2, TFT_ORANGE);

  // Draw moving highlight
  tft.fillCircle(hx, hy, r / 10, TFT_WHITE);

  lastR = r;
}

// **************** Draw Functions - Helpers **************** //

void drawCenteredTextSprite(TFT_eSprite &sprite, const String &text, const uint8_t* font, int x, int y, uint8_t textDatum) {
  sprite.fillSprite(colorBackground);
  sprite.setTextColor(colorText, colorBackground);
  sprite.setTextDatum(textDatum);

  sprite.loadFont(font);
  sprite.drawString(text, sprite.width() / 2, sprite.height() / 2);
  sprite.unloadFont();

  sprite.pushSprite(x, y);
}

// **************** LED Functions **************** //

void HandleLED() {
  unsigned long now = millis();
  if (now - lastColorUpdate >= colorUpdateInterval) {
    lastColorUpdate = now;
    colorWheelPos += 1.0; // slow evolution, adjust for speed
    if (colorWheelPos >= 360.0) colorWheelPos -= 360.0;

    for (int i = 0; i < 4; i++) {
      float hue = fmod(colorWheelPos + i * colorStep, 360.0);
      uint32_t color = ColorFromHSV(hue, 1.0, 0.2); // Hue, Saturation, Brightness - 0.2 = 20% brightness
      strip.setPixelColor(i, color);
    }
    strip.show();
  }
}

uint32_t ColorFromHSV(float h, float s, float v) {
  // h: 0-360, s: 0-1, v: 0-1
  float r, g, b;
  int i = int(h / 60.0) % 6;
  float f = (h / 60.0) - i;
  float p = v * (1.0 - s);
  float q = v * (1.0 - f * s);
  float t = v * (1.0 - (1.0 - f) * s);
  switch (i) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5: r = v; g = p; b = q; break;
  }
  return strip.Color(int(r * 255), int(g * 255), int(b * 255));
}

// **************** Sensor Functions **************** //

// **************** Network Functions **************** //

// void SetupOTA() {
//   // Authentication details
//   ArduinoOTA.setHostname(OTA_HOST_NAME);
//   ArduinoOTA.setPassword(OTA_PASSWORD);
//
//   ArduinoOTA
//     .onStart([]() {
//       String type;
//       if (ArduinoOTA.getCommand() == U_FLASH) {
//         type = "sketch";
//       } else {  // U_SPIFFS
//         type = "filesystem";
//       }
//
//       // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
//       Serial.println("Start updating " + type);
//     })
//     .onEnd([]() {
//       Serial.println("\nEnd");
//     })
//     .onProgress([](unsigned int progress, unsigned int total) {
//       Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
//     })
//     .onError([](ota_error_t error) {
//       Serial.printf("Error[%u]: ", error);
//       if (error == OTA_AUTH_ERROR) {
//         Serial.println("Auth Failed");
//       } else if (error == OTA_BEGIN_ERROR) {
//         Serial.println("Begin Failed");
//       } else if (error == OTA_CONNECT_ERROR) {
//         Serial.println("Connect Failed");
//       } else if (error == OTA_RECEIVE_ERROR) {
//         Serial.println("Receive Failed");
//       } else if (error == OTA_END_ERROR) {
//         Serial.println("End Failed");
//       }
//     });
//
//   ArduinoOTA.begin();
//   Serial.println("OTA Initialized");
// }

// **************** User Interface Functions **************** //

void SetupLCD() {
  // Configure LCD initial state, tft can be used if the element is static, sprite should be used for dynamic objects
  // NOTE full screen sprite is not possible, memory will overflow without error. If the sprite can't create memory is the cause
  tft.init();
  tft.setRotation(3);
  tft.setSwapBytes(true);
  tft.fillScreen(colorCalmBlue);

  // This will dump creation errors to the serial port
  CreateAndFillSprite(centerText, 160, 60, colorCalmBlue, "centerText");

  CreateAndFillSprite(centerSubText, 80, 25, colorCalmBlue, "centerSubText");

  CreateAndFillSprite(upperSubText, 80, 25, colorCalmBlue, "upperSubText");

  // Backlight is defined in the tft_espi library
  analogWrite(BACKLIGHT, backlightLevel);

  touch.begin();
}

void CreateAndFillSprite(TFT_eSprite &sprite, int width, int height, uint16_t fillColor, const String &spriteName) {
  if (!sprite.createSprite(width, height)) {
    Serial.print("Sprite (");
    Serial.print(spriteName);
    Serial.println(") creation failed!");
  } else {
    sprite.fillSprite(fillColor);
  }
}

void NavigationDebounce() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillisTouchEvent >= touchNavigationDebounceTime) {
    previousMillisTouchEvent = currentMillis;

    if (touchNavDebounce) {
      touchNavEnabled = true;
    } else {
      touchNavEnabled = false;
      if (touch.available()) {}
    }

    touchNavDebounce = !touchNavDebounce;
  }
}

void DrawAnimatedProgressBar(int x, int y, int width, int height, int progress, int maxProgress, uint16_t barColor, uint16_t bgColor) {
  // Draw background
  tft.fillRoundRect(x, y, width, height, height / 2, bgColor);
  // Draw progress
  int barWidth = map(progress, 0, maxProgress, 0, width);
  tft.fillRoundRect(x, y, barWidth, height, height / 2, barColor);
  // Optional: Draw border
  tft.drawRoundRect(x, y, width, height, height / 2, TFT_WHITE);
}

// **************** Power Management Functions **************** //

void MonitorTask(void *pvParameters) {
  while (1) {
    bool buttonPinLow = digitalRead(BUTTON_PIN) == LOW;

    if (buttonPinLow) {
      if (buttonLow) {
        // Pin has just gone low, record the start time and reset the timer
        buttonLow = false;
        highStartTime = millis();

        // ResetSleepTimer();

      } else if (millis() - highStartTime >= SUSTAINED_HIGH_DURATION && !sustainedPress) {
        // Pin has remained low for the required duration, trigger sleep
        sustainedPress = true;

        GoToSleep();
      }
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);  // Check every 100ms
  }
}

void GoToSleep() {
  analogWrite(BACKLIGHT, 0);

  strip.setBrightness(0);

  delay(3000);

  digitalWrite(powerEnable, LOW);
  esp_deep_sleep_start();
}