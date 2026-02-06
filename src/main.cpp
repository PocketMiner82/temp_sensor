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
#define DHT_PIN D3

// the used PIR sensor pin
#define PIR_PIN D10

// the used photoresistor pin
#define PR_PIN A0

// the used dht sensor type
#define DHT_TYPE DHT11

// width of OLED
#define SCREEN_WIDTH 128

// height of OLED
#define SCREEN_HEIGHT 32

// Address of OLED
#define SCREEN_ADDRESS 0x3C

// the threshold at which the screen should be off by default (0-4095)
#define SCREEN_OFF_THRESHOLD 1536

// the threshold at which the screen should always be on (0-4095)
#define SCREEN_ALWAYS_ON_THRESHOLD 2048

// the threshold at which to dim the screen (0-4095)
#define SCREEN_DIM_THRESHOLD 2560

// the threshold at which to undim the screen (0-4095)
#define SCREEN_UNDIM_THRESHOLD 3072

// the time in ms which the OLED should show temp+humidity
#define TIME_SCREEN_ON 5000

// temperature offset (for calibration)
#define TEMP_OFFSET (-1)

// humidity offset (for calibration)
#define HUM_OFFSET (-5)


SensorManager *sensors;

Display *display;

// the timestamp (ms) of the last registered touch event
unsigned long lastTouchTime = 0;


[[noreturn]] void displayTask([[maybe_unused]] void *params) {
    // make sure screen is off initially
    lastTouchTime = 0 - TIME_SCREEN_ON;

    bool dimmed = false;
    bool alwaysOn = false;

    while (true) {
        display->clearDisplay();

        const unsigned long currentMillis = millis();
        const unsigned long prValue = analogRead(PR_PIN);

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
            const float temp = sensors->getTemperature();
            const float hum = sensors->getHumidity();

            display->setCursor(0, display->calculateYOffset(temp, hum));
            display->printTemperature(temp, 2);
            display->printRightAlignedHumidity(hum);
        }

        display->display();
        vTaskDelay(pdMS_TO_TICKS(100));
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