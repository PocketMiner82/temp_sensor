#include "SensorManager.h"
#include <FreeRTOS_SAMD21.h>

SensorManager::SensorManager(const byte dsPin, const byte dhtPin, const byte dhtType,
    float tempOffset, float humOffset) :
        dht(dhtPin, dhtType), oneWire(dsPin), ds(&oneWire), tempOffset(tempOffset), humOffset(humOffset) {
    dht.begin();
    sensor_t sensor;
    dht.humidity().getSensor(&sensor);
    delayMs = sensor.min_delay / 1000;

    ds.begin();

    // run the sensor reading in another task so the main loop is only drawing the display
    xTaskCreate(
        readTask,
        "readTask",
        512,
        this,
        tskIDLE_PRIORITY + 1,
        &sensorTaskHandle
    );
}

void SensorManager::readTask(void *params) {
    while (true) {
        const auto instance = static_cast<SensorManager*>(params);

        // const unsigned long currentMillis = millis();
        // if (currentMillis - instance->lastMeasurement < instance->delayMs) {
        //     return;
        // }
        // instance->lastMeasurement = currentMillis;

        sensors_event_t humEvent;
        instance->dht.humidity().getEvent(&humEvent);
        if (isnan(humEvent.relative_humidity)) {
            Serial.println("Error reading humidity!");
            Serial.flush();
        } else {
            instance->hum = humEvent.relative_humidity;
        }

        instance->ds.requestTemperaturesByIndex(0);
        if (isnan(instance->ds.getTempCByIndex(0))) {
            Serial.println("Error reading temperature!");
            Serial.flush();
        } else {
            instance->temp = instance->ds.getTempCByIndex(0);
        }

        vTaskDelay(pdMS_TO_TICKS(instance->delayMs));
    }

    // Have to call this or the system crashes when you reach the end bracket and then get scheduled.
    vTaskDelete(nullptr);
}

float SensorManager::getTemperature() const {
    return temp + tempOffset;
}

float SensorManager::getHumidity() const {
    return hum + humOffset;
}
