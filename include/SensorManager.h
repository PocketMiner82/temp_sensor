#ifndef TEMP_SENSOR_SENSORMANAGER_H
#define TEMP_SENSOR_SENSORMANAGER_H
#include "DallasTemperature.h"
#include "DHT_U.h"
#include "OneWire.h"
#include <FreeRTOS_SAMD21.h>


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

    // Handle for the background sensor reading task
    TaskHandle_t sensorTaskHandle = nullptr;

    // Minimum delay required between DHT reads
    unsigned long delayMs;

    // Timestamp (ms) when the last read happened
    unsigned long lastMeasurement = 0;

    // Last successfully cached temperature value
    float temp = -1;

    // Temperature offset to add to the read temperature
    float tempOffset = 0;

    // Last successfully cached humidity value
    float hum = -1;

    // Humidity offset to add to the read humidity
    float humOffset = 0;

    /**
     * A loop that performs a new measurement if enough time has passed.
     */
    static void readTask(void *params);

public:
    /**
     * Constructs the manager and initializes sensor hardware buses.
     * @param dsPin The GPIO pin connected to the DS18B20 data line.
     * @param dhtPin The GPIO pin connected to the DHT sensor data line.
     * @param dhtType The DHT model type (e.g., DHT11, DHT22).
     * @param tempOffset Temperature offset to add to the read temperature
     * @param humOffset Humidity offset to add to the read humidity
     */
    SensorManager(byte dsPin, byte dhtPin, byte dhtType, float tempOffset, float humOffset);

    /**
     * Retrieves the most recent temperature reading.
     * @return Temperature in Celsius.
     */
    [[nodiscard]] float getTemperature() const;

    /**
     * Retrieves the most recent humidity reading.
     * @return Relative humidity percentage.
     */
    [[nodiscard]] float getHumidity() const;
};


#endif //TEMP_SENSOR_SENSORMANAGER_H