#include "SensorManager.h"

#include <FreeRTOS_SAMD21.h>

#include "MovingMedianCollection.h"

SensorManager::SensorManager(const byte dsPin, const byte dhtPin, const byte dhtType,
                             const float tempOffset, const float humOffset) :
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
        1024,
        this,
        tskIDLE_PRIORITY + 1,
        &sensorTaskHandle
    );
}

void SensorManager::readTask(void *params) {
    const auto instance = static_cast<SensorManager*>(params);

    // last humidities
    MovingMedianCollection<float, 7> humidities;

    // last temps
    MovingMedianCollection<float, 7> temps;

    DeviceAddress dsAddress;
    instance->ds.getAddress(dsAddress, 0);

    while (true) {
        sensors_event_t humEvent;
        instance->dht.humidity().getEvent(&humEvent);
        if (isnan(humEvent.relative_humidity)) {
            Serial.println("Error reading humidity!");
            Serial.flush();
        } else {
            humidities.add(humEvent.relative_humidity);
            instance->hum = humidities.get_median();
        }

        instance->ds.requestTemperaturesByAddress(dsAddress);
        float newTemp = instance->ds.getTempC(dsAddress);
        if (newTemp <= DEVICE_DISCONNECTED_C) {
            Serial.println("Error reading temperature!");
            Serial.flush();

            // try to get new address
            instance->ds.getAddress(dsAddress, 0);
        } else {
            temps.add(newTemp);
            instance->temp = temps.get_median();
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
