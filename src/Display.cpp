#include "Display.h"

#include "Fonts/FreeSans12pt7b.h"

Display::Display(const short screenWidth, const short screenHeight, const byte screenAddress):
    Adafruit_SSD1306(screenWidth, screenHeight) {

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!begin(SSD1306_SWITCHCAPVCC, screenAddress)) {
        // ReSharper disable once CppDFAEndlessLoop
        for(;;) {
            Serial.println("SSD1306 allocation failed");
            delay(1000);
        }
    }

    setFont(&FreeSans12pt7b);
    setTextWrap(false);
    setTextSize(1);
    setTextColor(WHITE);
}

String Display::getTempString(const float temp) {
    return String(temp, 1);
}

String Display::getHumString(const float hum) {
    return String(hum, 0);
}

std::pair<unsigned short, unsigned short> Display::getBB(const String &str) {
    int16_t x1, y1;
    unsigned short w, h;
    // Get cursor position after the number
    getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    w += x1;

    return {w, h};
}

short Display::calculateYOffset(const float temp, const float hum) {
    const auto measureStr = getTempString(temp) + getHumString(hum) + " C%";

    const auto [w, h] = getBB(measureStr);

    return static_cast<short>((height() - h) / 2 + h);
}

void Display::printTemperature(const float temp, const short circleRadius) {
    const short x = getCursorX();
    const short y = getCursorY();
    const auto tempStr = getTempString(temp);

    print(tempStr);

    const auto [w, h] = getBB(tempStr);

    // Manually draw a "Degree" circle relative to the font size
    // For 9pt font, a 3x3 or 4x4 circle usually looks best
    const auto circleX = static_cast<short>(x + w + 2 + circleRadius);
    const auto circleY = static_cast<short>(y - h + circleRadius);
    drawCircle(circleX, circleY, circleRadius, WHITE);

    setCursor(static_cast<short>(circleX + 2), y);
    print("C");
}

void Display::printRightAlignedHumidity(const float hum) {
    const String humStr = getHumString(hum) + "%";

    const auto [w, h] = getBB(humStr);

    setCursor(static_cast<short>(width() - w), getCursorY());
    print(humStr);
}
