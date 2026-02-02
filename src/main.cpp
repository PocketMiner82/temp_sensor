#include <Adafruit_FreeTouch.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <DHT.h>

#include "Display.h"
#include "SensorManager.h"


// digital pin of the DS18B20 sensor
#define DS_PIN D6

// digital pin of the DHT sensor
#define DHT_PIN D2

// the used dht sensor type
#define DHT_TYPE DHT11

// width/height of used OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// Address of OLED
#define SCREEN_ADDRESS 0x3C

// the threshold value at which the touch pin should get triggered
#define TOUCH_THRESHOLD 750

// the used QTouch pin
#define TOUCH_PIN A1

// the time in ms which the OLED should show temp+humidity
#define TIME_SCREEN_ON 10000


auto touch = Adafruit_FreeTouch(TOUCH_PIN, OVERSAMPLE_4, RESISTOR_50K, FREQ_MODE_NONE);

SensorManager *sensors;

Display *display;

// the timestamp (ms) of the last registered touch event
unsigned long lastTouchTime = 0;


void setup() {
    Serial.begin(9600);

    if (!touch.begin()) {
        // ReSharper disable once CppDFAEndlessLoop
        for(;;) {
            Serial.println("Failed to begin qt on pin A1");
            delay(1000);
        }
    }

    sensors = new SensorManager(DS_PIN, DHT_PIN, DHT_TYPE);

    delay(500);
    display = new Display(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ADDRESS);
}

void loop() {
    display->clearDisplay();

    unsigned long currentMillis = millis();

    // show temp/hum if screen should still be on
    if (currentMillis - lastTouchTime < TIME_SCREEN_ON) {
        const float temp = sensors->getTemperature();
        const float hum = sensors->getHumidity();

        display->setCursor(0, display->calculateYOffset(temp, hum));
        display->printTemperature(temp, 2);
        display->printRightAlignedHumidity(hum);
    }

    // touch registered?
    if (touch.measure() > TOUCH_THRESHOLD) {
        lastTouchTime = currentMillis;
    }

    display->display();
    delay(100);
}