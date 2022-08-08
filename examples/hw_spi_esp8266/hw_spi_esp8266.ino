#include <Adafruit_GFX.h>
#include <Adafruit_SPFD54124B.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSansBoldOblique24pt7b.h>

#define TFT_CS          D0
#define TFT_RESET       D1

#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

Adafruit_SPFD54124B display(TFT_RESET, TFT_CS);

void setup(void)
{
    display.begin();
    Serial.begin(115200);
    display.setRotation(1);
    unsigned long time;
    time = millis();

    display.fillScreen(WHITE);
    display.fillScreen(BLACK);

    Serial.println(millis() - time);

    display.fillRect(100, 10, 20, 20, CYAN);

    display.setTextColor(WHITE);

    display.setCursor(5, 30);
    display.setFont();
    display.print("Hello World");

    display.setTextColor(BLUE);

    display.setCursor(5, 50);
    display.setFont(&FreeSerif9pt7b);
    display.print("Hello World");

    display.setTextColor(YELLOW);
    display.setCursor(5, 110);
    display.setFont(&FreeSansBoldOblique24pt7b);
    display.print("Hello");
}

void loop() {}