/*

Desktop widget

Read and display temperature and humidity on a waveshare 1.8" round LCD with touch. 

Target microcontroller is an ESP-32-WROOM

Battery voltage is available on hardware.

Burn-in may be an issue, multiple display pages will be created and will cycle 
automatically to lessen negative effect.

*/

#include <WiFi.h>
#include <ArduinoOTA.h>

#include <Defines.h>
#include <Secrets.h>

// Graphics
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

TFT_eSprite centerText = TFT_eSprite(&tft);
TFT_eSprite centerSubText = TFT_eSprite(&tft);
TFT_eSprite upperSubText = TFT_eSprite(&tft);

// Touchscreen
#include <CST816S.h>
CST816S touch(SDA, SCL, RST, INT);

#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);

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

  // Enable wireless
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  SetupOTA();

  // Init NeoPixel strip, 4 px ring in this case
  strip.begin(); 
  strip.show();
  strip.setBrightness(ledBrightness);
}

void loop() {
  // Network items
  ArduinoOTA.handle();

  // Handle touch events - TODO orientation is not implemented yet
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
  }
}

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
  float pulse = (sin((now - startTime) / 400.0) + 1.0) * 0.5; // 0..1

  tft.fillScreen(colorMossGreen);

  int cx = centerX;
  int cy = centerY;
  int maxR = maxRadius - 10;
  int minR = 30;
  int r = minR + (int)((maxR - minR) * pulse);

  tft.drawCircle(cx, cy, r, TFT_GREEN);
  tft.drawCircle(cx, cy, r / 2, TFT_WHITE);
  tft.fillCircle(cx, cy, r / 4, TFT_YELLOW);
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

void SetupOTA() {
  // Authentication details
  ArduinoOTA.setHostname(OTA_HOST_NAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });

  ArduinoOTA.begin();
  Serial.println("OTA Initialized");
}

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