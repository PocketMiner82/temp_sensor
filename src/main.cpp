#include <Adafruit_FreeTouch.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <DHT.h>

#include "Display.h"
#include "SensorManager.h"


// digital pin of the DS18B20 sensor
#define DS_PIN D2

// digital pin of the DHT sensor
#define DHT_PIN D3

// the used PIR sensor pin
#define PIR_PIN D10

// the used photoresistor pin
#define PR_PIN A9

// the used dht sensor type
#define DHT_TYPE DHT11

// width of OLED
#define SCREEN_WIDTH 128

// height of OLED
#define SCREEN_HEIGHT 32

// Address of OLED
#define SCREEN_ADDRESS 0x3C

// the threshold at which the screen should be off by default (0-4095)
#define SCREEN_OFF_THRESHOLD 512

// the threshold at which the screen should always be on (0-4095)
#define SCREEN_ALWAYS_ON_THRESHOLD 1024

// the threshold at which to dim the screen (0-4095)
#define SCREEN_DIM_THRESHOLD 1536

// the threshold at which to undim the screen (0-4095)
#define SCREEN_UNDIM_THRESHOLD 2048

// the time in ms which the OLED should show temp+humidity
#define TIME_SCREEN_ON 5000

// temperature offset (for calibration)
#define TEMP_OFFSET (-1)

// humidity offset (for calibration)
#define HUM_OFFSET (-5)

// uncomment if all display pixels should be turned on
//#define DEBUG_ALL_ON


SensorManager *sensors;

Display *display;


[[noreturn]] void displayTask([[maybe_unused]] void *params) {
    // the timestamp (ms) of the last registered touch event. also make sure screen is off initially
    unsigned long lastTouchTime = 0 - TIME_SCREEN_ON;

    bool dimmed = false;
    bool alwaysOn = false;
    bool off = false;

    float oldTemp = -999.0f;
    float oldHum = -999.0f;

    while (true) {
        const unsigned long currentMillis = millis();
        const unsigned long prValue = analogRead(PR_PIN);

        Serial.println(prValue);

        // motion detected?
        if (digitalRead(PIR_PIN) == HIGH) {
            lastTouchTime = currentMillis;
        }

        if (prValue < SCREEN_OFF_THRESHOLD && alwaysOn) {
            // only allow screen to be on when motion detected
            alwaysOn = false;
        } else if (prValue > SCREEN_ALWAYS_ON_THRESHOLD && !alwaysOn) {
            // leave screen on if above threshold
            alwaysOn = true;
        }

        // dim screen if below threshold
        if (prValue < SCREEN_DIM_THRESHOLD && !dimmed) {
            dimmed = true;
            display->dim(true);
        } else if (prValue > SCREEN_UNDIM_THRESHOLD && dimmed) {
            dimmed = false;
            display->dim(false);
        }

        // show temp/hum if screen should still be on
        if (currentMillis - lastTouchTime < TIME_SCREEN_ON || alwaysOn) {
            off = false;

            const float temp = sensors->getTemperature();
            const float hum = sensors->getHumidity();

            if (abs(oldTemp - temp) > 0.01f || abs(oldHum - hum) > 0.01f) {
                oldTemp = temp;
                oldHum = hum;

                display->clearDisplay();
                display->setCursor(0, display->calculateYOffset(temp, hum));
                display->printTemperature(temp, 2);
                display->printRightAlignedHumidity(hum);
                display->display();
            }
        } else if (!off) {
            oldTemp = -999.0f;
            oldHum = -999.0f;
            display->clearDisplay();
            display->display();
            off = true;
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // ReSharper disable once CppDFAUnreachableCode
    vTaskDelete(nullptr);
}

void setup() {
    Serial.begin(9600);

    pinMode(PIR_PIN, INPUT);

    // ADC supports 12-bit resolution
    analogReadResolution(12);

    sensors = new SensorManager(DS_PIN, DHT_PIN, DHT_TYPE, TEMP_OFFSET, HUM_OFFSET);

    // wait for pin to go low
    do {
        delay(100);
    } while (digitalRead(PIR_PIN) == HIGH);

    display = new Display(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ADDRESS);

#ifdef DEBUG_ALL_ON
    display->clearDisplay();
    display->display();
    display->invertDisplay(true);
    return;
#endif

    xTaskCreate(displayTask,
        "displayTask",
        2048,
        nullptr,
        tskIDLE_PRIORITY + 2,
        nullptr
    );

    vSetErrorSerial(&Serial);
    vSetErrorLed(LED_BUILTIN, LOW);

    // Start the RTOS, this function will never return and will schedule the tasks.
    vTaskStartScheduler();

    // error scheduler failed to start
    // should never get here
    // ReSharper disable once CppDFAEndlessLoop
    while(true)
    {
        Serial.println("Scheduler Failed! \n");
        Serial.flush();
        delay(1000);
    }
}

void loop() {
    delay(100);
}