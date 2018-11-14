// #include <U8g2lib.h>
#include "displayhelper.h"
#include "pinouthelper.h"
// #include "timehelper.h"
// #include "sholathelper.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define DEBUGPORT Serial

// #define RELEASE

#ifndef RELEASE
#define DEBUGLOG(fmt, ...)                       \
    {                                            \
        static const char pfmt[] PROGMEM = fmt;  \
        DEBUGPORT.printf_P(pfmt, ##__VA_ARGS__); \
    }
#define DEBUGLOGLN(fmt, ...)                     \
    {                                            \
        static const char pfmt[] PROGMEM = fmt;  \
        static const char rn[] PROGMEM = "\r\n"; \
        DEBUGPORT.printf_P(pfmt, ##__VA_ARGS__); \
        DEBUGPORT.printf_P(rn);                  \
    }
#else
#define DEBUGLOG(...)
#define DEBUGLOGLN(...)
#endif

// U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2_2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
// U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2_2(U8G2_R0, SCL, SDA, /* reset=*/U8X8_PIN_NONE);
// U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, SCL, SDA, /* reset=*/U8X8_PIN_NONE);

uint8_t mask = 0xFF;

void u8g2_string8(uint16_t a)
{
    const uint8_t *font;

    // font = u8g2_font_6x10_mn;
    // font = u8g2_font_smart_patrol_nbp_tn;
    // font = u8g2_font_profont15_mn;
    // font = u8g2_font_px437wyse700b_mn;
    // font = u8g2_font_profont17_mn;
    // font = u8g2_font_t0_18_me;
    // font = u8g2_font_balthasar_titling_nbp_tn;
    // font = u8g2_font_logisoso16_tn;
    // font = u8g2_font_logisoso42_tn;
    // font = u8g2_font_logisoso46_tn;
    // font = u8g2_font_inb33_mn;
    // font = u8g2_font_inr33_mn;
    // font = u8g2_font_freedoomr10_mu;
    // font = u8g2_font_helvR12_tf;
    // font = u8g2_font_t0_17_tf;
    font = u8g2_font_helvR10_tf;

    u8g2.setFont(font);

    char buf[36];

    snprintf(buf, sizeof(buf), "Waiting GPS...");

    int x, y;

    x = 0;
    y = 0;

    u8g2.setCursor(x, y);

    u8g2.setFontPosTop();

    u8g2.print(buf);
}

void DisplaySetup()
{
    // u8g2_2.begin();
    // u8g2_2.clear();
    u8g2.begin();
    // Wire.begin(); // SDA, SCL

#ifdef ESP8266
    // Wire.setClock(1600000L);
    Wire.setClock(100000L);
#endif

    // u8g2.setBusClock(400);
    u8g2.clear();

    u8g2.setFontPosTop();
}

void DisplayLoop()
{

    // picture loop
    // must be after sholat time has been updated
    // u8g2.clearBuffer();
    // u8g2_2.clearBuffer();

    static uint16_t x = 0;
    // u8g2_prepare();
    // u8g2_string(x);
    // u8g2_string2(x);
    // u8g2_string3(x);
    // u8g2_string4(x);
    // u8g2_string5(x);
    // u8g2_string6(0);
    // u8g2_string7(0);
    u8g2_string8(0);
    // u8g2_string9(0);
    x = x - 84;
    if (x <= -127)
    {
        x = 0;
    }

    // u8g2_2.sendBuffer();
}