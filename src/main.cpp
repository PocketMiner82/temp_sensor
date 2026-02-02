#include <Arduino.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "Adafruit_FreeTouch.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <utility>

#include "Fonts/FreeSans12pt7b.h"

#define DHT_PIN 2
#define DHT_TYPE DHT11

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

/// See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define SCREEN_ADDRESS 0x3C

// the threshold value at which the touch pin should get triggered
#define TOUCH_THRESHOLD 750
#define TOUCH_PIN A1

// the time in ms which the OLED should show temp+humidity
#define TIME_SCREEN_ON 10000

DHT_Unified dht(DHT_PIN, DHT_TYPE);

auto touch = Adafruit_FreeTouch(TOUCH_PIN, OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);

Adafruit_SSD1306 *display;

unsigned long sensorDelayMS = 0;
unsigned long offAfter = 0;
unsigned long nextMeasurement = 0;

float temp = -1;
float hum = -1;

String getTempString() {
    return String(temp, 1);
}

String getHumString() {
    return String(hum, 0);
}

std::pair<unsigned short, unsigned short> getBB(const String &str) {
    int16_t x1, y1;
    unsigned short w, h;
    // Get cursor position after the number
    display->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    w += x1;

    return {w, h};
}

short calculateYOffset() {
    const auto measureStr = getTempString() + getHumString() + " C%";

    const auto [w, h] = getBB(measureStr);

    return static_cast<short>((SCREEN_HEIGHT - h) / 2 + h);
}

void printTemperature(const short circleSize) {
    const short x = display->getCursorX();
    const short y = display->getCursorY();
    const auto tempStr = getTempString();

    display->print(tempStr);

    const auto [w, h] = getBB(tempStr);

    // Manually draw a "Degree" circle relative to the font size
    // For 9pt font, a 3x3 or 4x4 circle usually looks best
    const auto circleX = static_cast<short>(x + w + 2 + circleSize);
    const auto circleY = static_cast<short>(y - h + circleSize);
    display->drawCircle(circleX, circleY, circleSize, WHITE);

    display->setCursor(static_cast<short>(circleX + 2), y);
    display->print("C");
}

void printRightAlignedHumidity() {
    const String humStr = getHumString() + "%";

    const auto [w, h] = getBB(humStr);

    display->setCursor(static_cast<short>(display->width() - w), display->getCursorY());
    display->print(humStr);
}

void readSensor() {
    sensors_event_t tempEvent;
    sensors_event_t humEvent;
    dht.temperature().getEvent(&tempEvent);
    dht.humidity().getEvent(&humEvent);

    if (isnan(tempEvent.temperature)) {
        Serial.println(F("Error reading temperature!"));
    } else {
        temp = tempEvent.temperature;
    }

    if (isnan(humEvent.relative_humidity)) {
        Serial.println(F("Error reading humidity!"));
    } else {
        hum = humEvent.relative_humidity;
    }
}

void setup() {
    Serial.begin(9600);
    dht.begin();

    sensor_t sensor;
    dht.humidity().getSensor(&sensor);
    sensorDelayMS = sensor.min_delay / 1000;

    if (!touch.begin()) {
        // ReSharper disable once CppDFAEndlessLoop
        for(;;) {
            Serial.println("Failed to begin qt on pin A1");
            delay(1000);
        }
    }

    display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT);

    delay(500);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        // ReSharper disable once CppDFAEndlessLoop
        for(;;) {
            Serial.println("SSD1306 allocation failed");
            delay(1000);
        }
    }

    display->setFont(&FreeSans12pt7b);
    display->setTextWrap(false);
    display->setTextSize(1);
    display->setTextColor(WHITE);
}

void loop() {
    display->clearDisplay();

    if (offAfter > millis()) {
        if (nextMeasurement < millis()) {
            nextMeasurement = millis() + sensorDelayMS;
            readSensor();
        }

        display->setCursor(0, calculateYOffset());
        printTemperature(2);
        printRightAlignedHumidity();
    }

    if (touch.measure() > TOUCH_THRESHOLD) {
        offAfter = millis() + TIME_SCREEN_ON;
    }

    display->display();
    delay(100);
}