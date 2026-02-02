#ifndef TEMP_SENSOR_DISPLAYMANAGER_H
#define TEMP_SENSOR_DISPLAYMANAGER_H
#include "Adafruit_SSD1306.h"


/**
 * Extension of Adafruit_SSD1306 to handle formatted sensor data rendering.
 */
class Display : public Adafruit_SSD1306 {
public:
    /**
     * Initializes the OLED display and sets default font styles.
     * @param screenWidth Width of the display in pixels.
     * @param screenHeight Height of the display in pixels.
     * @param screenAddress I2C address of the display (usually 0x3D for 128x64 or 0x3C for 128x32).
     * @note If allocation fails, the constructor enters an infinite loop.
     */
    explicit Display(short screenWidth, short screenHeight, byte screenAddress);

    /**
     * Formats a temperature value to one decimal place.
     * @param temp The temperature value.
     * @return String representation (e.g., "23.5").
     */
    static String getTempString(float temp);

    /**
     * Formats a humidity value to zero decimal places.
     * @param hum The humidity value.
     * @return String representation (e.g., "45").
     */
    static String getHumString(float hum);

    /**
     * Calculates the width and height of a string based on the current font.
     * @param str The string to measure.
     * @return A pair containing {width, height} in pixels.
     */
    std::pair<unsigned short, unsigned short> getBB(const String &str);

    /**
     * Calculates the Y-coordinate required to vertically center text.
     * @param temp Representative temperature for measurement.
     * @param hum Representative humidity for measurement.
     * @return The Y-coordinate for the baseline of the text.
     */
    short calculateYOffset(float temp, float hum);

    /**
     * Prints temperature text followed by a degree symbol and 'C'.
     * @param temp The temperature value to print.
     * @param circleRadius Radius of the degree symbol circle in pixels.
     */
    void printTemperature(float temp, short circleRadius);

    /**
     * Aligns and prints the humidity percentage at the right edge of the screen.
     * @param hum The humidity value to print.
     */
    void printRightAlignedHumidity(float hum);
};


#endif //TEMP_SENSOR_DISPLAYMANAGER_H