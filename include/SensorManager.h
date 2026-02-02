#ifndef TEMP_SENSOR_SENSORMANAGER_H
#define TEMP_SENSOR_SENSORMANAGER_H
#include "DallasTemperature.h"
#include "DHT_U.h"
#include "OneWire.h"


/**
 * Orchestrates data acquisition from DHT (Humidity) and Dallas (Temperature) sensors.
 */
class SensorManager {
    // Unified DHT sensor instance for humidity
    DHT_Unified dht;

    // OneWire bus instance for DS18B20
    OneWire oneWire;

    // Dallas Temperature sensor instance
    DallasTemperature ds;

    // Minimum delay required between DHT reads
    unsigned long delayMs;

    // Timestamp (ms) when the last read happened
    unsigned long lastMeasurement = 0;

    // Last successfully cached temperature value
    float temp = -1;

    // Last successfully cached humidity value
    float hum = -1;

public:
    /**
     * Constructs the manager and initializes sensor hardware buses.
     * @param dsPin The GPIO pin connected to the DS18B20 data line.
     * @param dhtPin The GPIO pin connected to the DHT sensor data line.
     * @param dhtType The DHT model type (e.g., DHT11, DHT22).
     */
    SensorManager(byte dsPin, byte dhtPin, byte dhtType);

    /**
     * Performs a new measurement if enough time has passed.
     */
    void read();

    /**
     * Retrieves the most recent temperature reading.
     * @return Temperature in Celsius.
     */
    float getTemperature();

    /**
     * Retrieves the most recent humidity reading.
     * @return Relative humidity percentage.
     */
    float getHumidity();
};


#endif //TEMP_SENSOR_SENSORMANAGER_H