#ifdef __AVR__
#include <avr/pgmspace.h>
#include <util/delay.h>
#endif
#if defined(ESP8266)
#include "mySPI.h"
#endif
#include <stdlib.h>
#include "SPI.h"

#include "Adafruit_GFX.h"
#include "Adafruit_SPFD54124B.h"

Adafruit_SPFD54124B::Adafruit_SPFD54124B(int8_t SID, int8_t SCLK, int8_t RST, int8_t CS) : Adafruit_GFX(SPFD54124B_LCDWIDTH, SPFD54124B_LCDHEIGHT)
{
    sid = SID;
    sclk = SCLK;
    rst = RST;
    cs = CS;
    hwSPI = false;
}

Adafruit_SPFD54124B::Adafruit_SPFD54124B(int8_t RST, int8_t CS) : Adafruit_GFX(SPFD54124B_LCDWIDTH, SPFD54124B_LCDHEIGHT)
{
    sid = -1;
    sclk = -1;
    rst = RST;
    cs = CS;
    hwSPI = true;
}

void Adafruit_SPFD54124B::writeCommand(uint8_t c)
{
    // Prepend leading bit instead of using D/C pin
    if (hwSPI)
    {
#ifdef __AVR__
        SPCR = 0;                  // Disable SPI temporarily
        *dataport &= ~datapinmask; // Clear 9th bit
        *clkport |= clkpinmask;    // Clock tick
        *clkport &= ~clkpinmask;   //       tock
        SPCR = spi_save;           // Re-enable SPI
        SPDR = c;                  // Issue remaining 8 bits

        while (!(SPSR & _BV(SPIF)))
            ; // Await completion
#endif

#if defined(ESP8266)

        uint16_t data = 0 | c;
        uint16_t data_2 = 0x0000;

        data_2 = data >> 1;
        if (data & 0x01)
            data_2 = data_2 | 0x8000;
        mSPI.write9(data_2);
#endif
    }
    else
    {
#ifdef __AVR__
        *dataport &= ~datapinmask;
        *clkport |= clkpinmask;
        *clkport &= ~clkpinmask;
        for (uint8_t bit = 0x80; bit; bit >>= 1)
        {
            if (c & bit)
                *dataport |= datapinmask;
            else
                *dataport &= ~datapinmask;
            *clkport |= clkpinmask;
            *clkport &= ~clkpinmask;
        }
#elif defined(ESP8266)

        GPOC = (1 << sid);
        GPOS = (1 << sclk); //set
        GPOC = (1 << sclk); //reset

        for (uint8_t bit = 0x80; bit; bit >>= 1)
        {
            if (c & bit)
                GPOS = (1 << sid);
            else
                GPOC = (1 << sid);
            GPOS = (1 << sclk);
            GPOC = (1 << sclk);
        }
#else
        digitalWrite(sid, LOW);
        digitalWrite(sclk, HIGH);
        digitalWrite(sclk, LOW);

        for (uint8_t bit = 0x80; bit; bit >>= 1)
        {
            if (c & bit)
                digitalWrite(sid, HIGH);
            else
                digitalWrite(sid, LOW);
            digitalWrite(sclk, HIGH);
            digitalWrite(sclk, LOW);
        }

#endif
    }
}

void Adafruit_SPFD54124B::writeData(uint8_t c)
{
    //c=0x00;
    // Prepend leading bit instead of using D/C pin
    if (hwSPI)
    {
#ifdef __AVR__
        SPCR = 0;                 // Disable SPI temporarily
        *dataport |= datapinmask; // Set 9th bit
        *clkport |= clkpinmask;   // Clock tick
        *clkport &= ~clkpinmask;  //       tock
        SPCR = spi_save;          // Re-enable SPI
        SPDR = c;                 // Issue remaining 8 bits
        while (!(SPSR & _BV(SPIF)))
            ; // Await completion
#endif

#if defined(ESP8266)

        uint16_t data = 0x100 | c;
        uint16_t data_2 = 0x0000;

        data_2 = data >> 1;
        if (data & 0x01)
            data_2 = data_2 | 0x8000;
        mSPI.write9(data_2);

#endif
    }
    else
    {

#ifdef __AVR__
        *dataport |= datapinmask;
        *clkport |= clkpinmask;
        *clkport &= ~clkpinmask;
        for (uint8_t bit = 0x80; bit; bit >>= 1)
        {
            if (c & bit)
                *dataport |= datapinmask;
            else
                *dataport &= ~datapinmask;
            *clkport |= clkpinmask;
            *clkport &= ~clkpinmask;
        }
#elif defined(ESP8266)
        GPOS = (1 << sid);
        GPOS = (1 << sclk); //set
        GPOC = (1 << sclk); //reset

        for (uint8_t bit = 0x80; bit; bit >>= 1)
        {
            if (c & bit)
                GPOS = (1 << sid);
            else
                GPOC = (1 << sid);
            GPOS = (1 << sclk);
            GPOC = (1 << sclk);
        }
#else

        digitalWrite(sid, HIGH);
        digitalWrite(sclk, HIGH);
        digitalWrite(sclk, LOW);

        for (uint8_t bit = 0x80; bit; bit >>= 1)
        {
            if (c & bit)
                digitalWrite(sid, HIGH);
            else
                digitalWrite(sid, LOW);
            digitalWrite(sclk, HIGH);
            digitalWrite(sclk, LOW);
        }

#endif
    }
}

// Idea swiped from 1.8" TFT code: rather than a bazillion writeCommand()
// and writeData() calls, screen initialization commands and arguments are
// organized in this table in PROGMEM.  It may look bulky, but that's
// mostly the formatting -- storage-wise this is considerably more compact
// than the equiv code.  Command sequence is from TFT datasheet.
#define DELAY 0x80
PROGMEM static const unsigned char
    initCmd[] = {
        14,                        // 14 commands in list:
        SPFD54124B_N_SETEXTCMD, 3, //  1: ???, 3 args, no delay
        0xFF, 0x83, 0x40,
        SPFD54124B_N_SPLOUT, DELAY, //  2: No args, delay follows
        150,                        //     150 ms delay
        0xCA, 3,                    //  3: Undoc'd register?  3 args, no delay
        0x70, 0x00, 0xD9,
        0xB0, 2, //  4: Undoc'd register?  2 args, no delay
        0x01, 0x11,
        0xC9, 8 + DELAY, //  5: Drive ability, 8 args + delay
        0x90, 0x49, 0x10, 0x28,
        0x28, 0x10, 0x00, 0x06,
        20,                        //     20 ms delay
        SPFD54124B_N_SETGAMMAP, 9, //  6: Positive gamma control, 9 args
        0x60, 0x71, 0x01,          //     2.2
        0x0E, 0x05, 0x02,
        0x09, 0x31, 0x0A,
        SPFD54124B_N_SETGAMMAN, 8 + DELAY, //  7: Negative gamma, 8 args + delay
        0x67, 0x30, 0x61, 0x17,            //     2.2
        0x48, 0x07, 0x05, 0x33,
        10,                        //     10 ms delay
        SPFD54124B_N_SETPWCTR5, 3, //  8: Power Control 5, 3 args
        0x35, 0x20, 0x45,
        SPFD54124B_N_SETPWCTR4, 3 + DELAY, //  9: Power control 4, 3 args + delay
        0x33, 0x25, 0x4c,
        10,                         //     10 ms delay
        SPFD54124B_N_COLMOD, 1,     // 10: Color Mode, 1 arg
        0x05,                       //     0x05 = 16bpp, 0x06 = 18bpp
        SPFD54124B_N_DISPON, DELAY, // 11: Display on, no args, w/delay
        10,                         //     10 ms delay
        SPFD54124B_N_CASET, 4,      // 12: Physical column pointer, 4 args
        0x00, 0x00, 0x00, 0xaf,     //     175 (max X)
        SPFD54124B_N_PASET, 4,      // 13: Physical page pointer, 4 args
        0x00, 0x00, 0x00, 0xdb,     //     219 (max Y)
        SPFD54124B_N_RAMWR, 0       // 14: Start GRAM write
};

void Adafruit_SPFD54124B::begin()
{
    // set pin directions, set pins low by default (except reset)

#ifdef __AVR__
    csport = portOutputRegister(digitalPinToPort(cs));
    cspinmask = digitalPinToBitMask(cs);

#endif
    pinMode(rst, OUTPUT);
    pinMode(cs, OUTPUT);
    digitalWrite(cs, LOW);
    digitalWrite(rst, HIGH);
    if (hwSPI)
    {
#ifdef __AVR__
        // Even if using hardware SPI, the clock and data pins are still
        // looked up and set to outputs, because the 9-bit SPI output code
        // temporarily disables the SPI bus to issue the extra bit.
        clkport = portOutputRegister(digitalPinToPort(SCK));
        clkpinmask = digitalPinToBitMask(SCK);
        dataport = portOutputRegister(digitalPinToPort(MOSI));
        datapinmask = digitalPinToBitMask(MOSI);
        pinMode(SCK, OUTPUT);
        pinMode(MOSI, OUTPUT);
        pinMode(SS, OUTPUT);
        SPI.begin();
        SPI.setClockDivider(SPI_CLOCK_DIV8); // 4 MHz (half speed)
        SPI.setBitOrder(MSBFIRST);
        SPI.setDataMode(SPI_MODE0);
        spi_save = SPCR; // Save SPI config bits for later
#endif

#if defined(ESP8266)
        mSPI.begin();
        mSPI.setFrequency(80000000L);
#endif
    }
    else
    {
#ifdef __AVR__
        clkport = portOutputRegister(digitalPinToPort(sclk));
        clkpinmask = digitalPinToBitMask(sclk);
        dataport = portOutputRegister(digitalPinToPort(sid));
        datapinmask = digitalPinToBitMask(sid);

#endif
        pinMode(sclk, OUTPUT);
        pinMode(sid, OUTPUT);
        digitalWrite(sclk, LOW);
        digitalWrite(sid, LOW);
    }

    // Reset the LCD
    digitalWrite(rst, HIGH);
    delay(100);
    digitalWrite(rst, LOW);
    delay(50);
    digitalWrite(rst, HIGH);
    delay(50);

    _cs_clr();

    // Companion code to the above tables.  Reads and issues
    // a series of LCD commands stored in PROGMEM byte array.
    uint8_t numCommands, numArgs;
    uint16_t ms;
    const unsigned char *addr = initCmd;
    numCommands = pgm_read_byte(addr++); // Number of commands to follow
    while (numCommands--)
    {                                        // For each command...
        writeCommand(pgm_read_byte(addr++)); //   Read, issue command
        numArgs = pgm_read_byte(addr++);     //   Number of args to follow
        ms = numArgs & DELAY;                //   If hibit set, delay follows args
        numArgs &= ~DELAY;                   //   Mask out delay bit
        while (numArgs--)
        {                                     //   For each argument...
            writeData(pgm_read_byte(addr++)); //     Read, issue argument
        }

        if (ms)
            delay(pgm_read_byte(addr++)); // Read post-command delay time (ms)
    }

    _cs_set();
}

void Adafruit_SPFD54124B::setWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    uint8_t t0, t1;
    _cs_clr();

    switch (rotation)
    {
    case 1:
        t0 = WIDTH - 1 - y1;
        t1 = WIDTH - 1 - y0;
        y0 = x0;
        x0 = t0;
        y1 = x1;
        x1 = t1;
        break;
    case 2:
        t0 = x0;
        x0 = WIDTH - 1 - x1;
        x1 = WIDTH - 1 - t0;
        t0 = y0;
        y0 = HEIGHT - 1 - y1;
        y1 = HEIGHT - 1 - t0;
        break;
    case 3:
        t0 = HEIGHT - 1 - x1;
        t1 = HEIGHT - 1 - x0;
        x0 = y0;
        y0 = t0;
        x1 = y1;
        y1 = t1;
        break;
    }

    writeCommand(SPFD54124B_N_CASET); // Column addr set
    writeData(0);
    writeData(x0); // X start
    writeData(0);
    writeData(x1); // X end

    writeCommand(SPFD54124B_N_PASET); // Page addr set
    writeData(0);
    writeData(y0); // Y start
    writeData(0);
    writeData(y1); // Y end

    writeCommand(SPFD54124B_N_RAMWR);

    _cs_set();
}

// clear everything
void Adafruit_SPFD54124B::fillScreen(uint16_t c)
{
    uint8_t x, y, hi = c >> 8, lo = c;

    setWindow(0, 0, _width - 1, _height - 1);

    _cs_clr();

    for (y = _height; y > 0; y--)
    {
        for (x = _width; x > 0; x--)
        {
            writeData(hi);
            writeData(lo);
        }
    }

    _cs_set();
}

// Used by BMP-reading sketch
void Adafruit_SPFD54124B::pushColor(uint16_t color)
{
    _cs_clr();

    writeData(color >> 8);
    writeData(color);

    _cs_set();
}

// the most basic function, set a single pixel
void Adafruit_SPFD54124B::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height))
        return;

    setWindow(x, y, x, y);
    _cs_clr();
    writeData(color >> 8);
    writeData(color);
    _cs_set();
}

void Adafruit_SPFD54124B::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{

    if ((x < 0) || (x >= _width) // Fully off left or right
        || (y >= _height))
        return; // Fully off bottom
    int16_t y2 = y + h - 1;
    if (y2 < 0)
        return; // Fully off top
    if (y2 >= _height)
        h = _height - y; // Clip bottom
    if (y < 0)
    {
        h += y;
        y = 0;
    } // Clip top
    setWindow(x, y, x, y + h - 1);

    uint8_t hi = color >> 8, lo = color;
    _cs_clr();
    while (h--)
    {
        writeData(hi);
        writeData(lo);
    }
    _cs_set();
}

void Adafruit_SPFD54124B::drawFastHLine(int16_t x, int16_t y, int16_t w,
                                        uint16_t color)
{

    if ((y < 0) || (y >= _height) // Fully off top or bottom
        || (x >= _width))
        return; // Fully off right
    int16_t x2 = x + w - 1;
    if (x2 < 0)
        return; // Fully off left
    if (x2 >= _width)
        w = _width - x; // Clip right
    if (x < 0)
    {
        w += x;
        x = 0;
    } // Clip left
    setWindow(x, y, x + w - 1, y);

    uint8_t hi = color >> 8, lo = color;
    _cs_clr();
    while (w--)
    {
        writeData(hi);
        writeData(lo);
    }
    _cs_set();
}

void Adafruit_SPFD54124B::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    // rudimentary clipping (drawChar w/big text requires this)
    if ((x >= _width) || (y >= _height))
        return; // Fully off right or bottom
    int16_t x2, y2;
    if (((x2 = x + w - 1) < 0) ||
        ((y2 = y + h - 1) < 0))
        return; // Fully off left or top
    if (x2 >= _width)
        w = _width - x; // Clip right
    if (x < 0)
    {
        w += x;
        x = 0;
    } // Clip left
    if (y2 >= _height)
        h = _height - y; // Clip bottom
    if (y < 0)
    {
        h += y;
        y = 0;
    } // Clip top

    setWindow(x, y, x + w - 1, y + h - 1);

    uint8_t hi = color >> 8, lo = color;
    int32_t i = (int32_t)w * (int32_t)h;

    _cs_clr();

    while (i--)
    {
        writeData(hi);
        writeData(lo);
    }

    _cs_set();
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t Adafruit_SPFD54124B::Color565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
