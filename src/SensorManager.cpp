#include "SensorManager.h"

SensorManager::SensorManager(const byte dsPin, const byte dhtPin, const byte dhtType) :
        dht(dhtPin, dhtType), oneWire(dsPin), ds(&oneWire) {
    dht.begin();
    sensor_t sensor;
    dht.humidity().getSensor(&sensor);
    delayMs = sensor.min_delay / 1000;

    ds.begin();
}

void SensorManager::read() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastMeasurement < delayMs) {
        return;
    }
    lastMeasurement = currentMillis;

    sensors_event_t humEvent;
    dht.humidity().getEvent(&humEvent);
    if (isnan(humEvent.relative_humidity)) {
        Serial.println("Error reading humidity!");
    } else {
        hum = humEvent.relative_humidity;
    }

    ds.requestTemperatures();
    if (isnan(ds.getTempCByIndex(0))) {
        Serial.println("Error reading temperature!");
    } else {
        temp = ds.getTempCByIndex(0);
    }
}

float SensorManager::getTemperature() {
    read();
    return temp;
}

float SensorManager::getHumidity() {
    read();
    return hum;
}
