#include "pageshelper.h"
#include "resources.h"
#include "timezonehelper.h"

#define DEBUGPORT Serial

#define PRINT(fmt, ...)                          \
    {                                            \
        static const char pfmt[] PROGMEM = fmt;  \
        DEBUGPORT.printf_P(pfmt, ##__VA_ARGS__); \
    }

// #define RELEASE

#ifndef RELEASE

#define DEBUGLOG(fmt, ...)                      \
    {                                           \
        static const char pfmt[] PROGMEM = fmt; \
        printf_P(pfmt, ##__VA_ARGS__);          \
    }
#else
#define DEBUGLOG(...)
#endif

uint8_t mode = 0;
uint8_t page = 0;
bool pageChangeFlag = false;

void ConstructClockPage()
{
    // char buf[17]; // MAGHRIB 18hr 36min

    // char mon[4];
    // strcpy(mon, monthShortStr(dtLocal.Month()));
    // snprintf_P(buf, sizeof(buf), PSTR("%s,%d-%s SAT%2d"),
    //            dayShortStr(dtLocal.DayOfWeek()),
    //            dtLocal.Day(),
    //            mon,
    //            fix.satellites);

    // snprintf_P(buf, sizeof(buf), PSTR("%d-%d-%d SAT%2d"),
    //            dtLocal.Day(),
    //            dtLocal.Month(),
    //            dtLocal.Year(),
    //            fix.satellites);

    // OzOled.drawFont8(buf, 0, 0);

    // snprintf_P(buf, sizeof(buf), PSTR("%2d:%02d:%02d"), h, m, s);
    // OzOled.printBigNumber(buf, 0, 2);

    // snprintf_P(buf, sizeof(buf), PSTR("%-7s   %2dh%2dm"), sholatNameStr(NEXTTIMEID), ceilHOUR, ceilMINUTE);
    // OzOled.drawFont8(buf, 0, 7);

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
    // font = u8g2_font_helvR10_tf;
    // font = u8g2_font_inb19_mn;
    // font = u8g2_font_inb24_mn;
    font = u8g2_font_artossans8_8r;
    font = u8g2_font_chroma48medium8_8r;
    // font = u8g2_font_roentgen_nbp_tf;

    u8g2.setFont(font);

    // u8g2.setFontMode(0);

    char buf[36];

    char mon[4];
    strcpy(mon, monthShortStr(dtLocal.Month()));

    /*
    snprintf_P(buf, sizeof(buf), PSTR("%s,%d-%s SAT%2d"),
               dayShortStr(dtLocal.DayOfWeek()),
               dtLocal.Day(),
               mon,
               fix.satellites);
               */

    // snprintf_P(buf, sizeof(buf), PSTR("%s %d %s %d"),
    //            dayShortStr(dtLocal.DayOfWeek()),
    //            dtLocal.Day(),
    //            mon,
    //            dtLocal.Year());

    int16_t x, y;

    // uint16_t len = u8g2.getStrWidth(buf);
    // x = (u8g2.getDisplayWidth() - len) / 2;
    // y = 0;

    // u8g2.setCursor(x, y);
    // u8g2.setFontMode(0);
    // static bool color = 1;
    // if (tick1000ms)
    //     color = !color;
    // u8g2.setDrawColor(color);
    u8g2.print(buf);

    u8g2.setDrawColor(1);

    char bufSec[3];
    snprintf_P(bufSec, sizeof(bufSec), PSTR("%02d"), secLocal);

    // snprintf_P(buf, sizeof(buf), PSTR("%2d:%02d:%c "), h, m, bufSec[0]);
    snprintf_P(buf, sizeof(buf), PSTR("%2d:%02d:  "), hourLocal, minLocal);

    x = 0;
    y = 15;

    font = u8g2_font_inb19_mn;
    // font = u8g2_font_profont29_mf;
    // font = u8g2_font_freedoomr25_mn;
    // font = u8g2_font_inb21_mn;
    font = bigNumbers16x24;

    u8g2.setFont(font);

    u8g2.setCursor(x, y);

    u8g2.setFontPosTop();

    u8g2.print(buf);

    // u8g2.print("88:88:00");

    // snprintf_P(buf, sizeof(buf), PSTR("%-7s   %2dh%2dm"), sholatNameStr(NEXTTIMEID), ceilHOUR, ceilMINUTE);
    // // snprintf_P(buf, sizeof(buf), PSTR("%-7s   %2dh%2dm"), "TEST", ceilHOUR, ceilMINUTE);
    // // OzOled.drawFont8(buf, 0, 7);

    // font = u8g2_font_artossans8_8r;
    // u8g2.setFont(font);

    // static int start_x = 128;

    // u8g2.setCursor(start_x, 55);
    // start_x--;
    // if (start_x <= -128)
    //     start_x = 128;

    // u8g2.print(buf);
}

void ConstructSpeedometerPage()
{
    static float rate = 0.0;

    static uint8_t count = 0;
    if (parseGPScompleted)
        count++;

    if (count == 3)
    {
        static unsigned long prevMillis = 0;
        unsigned long now = millis();
        unsigned long diff = now - prevMillis;
        prevMillis = now;

        // lastDiff = diff;

        rate = count * 1000.0 / (float)diff;
        count = 0;
    }

    char buf[17];

    if (fix.valid.speed)
        snprintf_P(buf, sizeof(buf), PSTR("VALID     %4.1fHz"), rate);
    else
        snprintf_P(buf, sizeof(buf), PSTR("NOSIG     %4.1fHz"), rate);

    const uint8_t *font;
    u8g2.setFontPosTop();

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
    // font = u8g2_font_helvR10_tf;
    // font = u8g2_font_inb19_mn;
    // font = u8g2_font_inb24_mn;
    font = u8g2_font_artossans8_8r;

    u8g2.setFont(font);
    u8g2.setCursor(0, 0);
    u8g2.print(buf);

    bool showFraction = 1;

    if (fix.valid.speed)
    {
        int tmpInt1 = fix.speed_kph(); // Get the integer (678).

        if (showFraction)
        {
            float tmpFrac = fix.speed_kph() - tmpInt1; // Get fraction (0.0123).
            int tmpInt2 = trunc(tmpFrac * 10);         // Turn into integer (123).
            snprintf_P(buf, sizeof(buf), PSTR("%03d.%d"), tmpInt1, tmpInt2);
        }
        else
        {
            snprintf_P(buf, sizeof(buf), PSTR("%03d"), tmpInt1);
        }
    }
    else
    {
        if (showFraction)
        {
            snprintf_P(buf, sizeof(buf), PSTR("---.-"));
        }
        else
        {
            snprintf_P(buf, sizeof(buf), PSTR("---"));
        }
    }

    font = u8g2_font_inb19_mn;
    font = bigNumbers21x26;
    u8g2.setFont(font);
    u8g2.setCursor(0, 15);
    u8g2.print(buf);
    font = u8g2_font_artossans8_8r;
    u8g2.setFont(font);
    u8g2.print("kph");

    // if (fix.valid.location)
    // {
    //     dtostrf(distanceToBaseKm, 6, 1, buf);
    // }
    // else
    // {
    //     snprintf_P(buf, sizeof(buf), PSTR("----.-"));
    // }
    // font = u8g2_font_inb19_mn;
    // u8g2.setFont(font);
    // u8g2.setCursor(0, 40);
    // u8g2.print(buf);
    // font = u8g2_font_artossans8_8r;
    // u8g2.setFont(font);
    // u8g2.print("km");

    // OzOled.drawFont8(dtostrf(GPSlatitude, 0, 7, buf), 0, 6);
    // OzOled.drawFont8(dtostrf(GPSlongitude, 0, 6, buf), 0, 7);
}

void ConstructBaseLocationPage()
{
    //title
    char buf[17];

    snprintf_P(buf, sizeof(buf), PSTR("BASE LOC."));

    const uint8_t *font;
    font = u8g2_font_artossans8_8r;
    u8g2.setFont(font);
    u8g2.setCursor(0, 0);
    u8g2.print(buf);

    u8g2.setCursor(0, 8);
    u8g2.print(dtostrf(configLocation.latitude, 11, 7, buf));
    u8g2.setCursor(0, 17);
    u8g2.print(dtostrf(configLocation.longitude, 11, 6, buf));

    u8g2.setCursor(0, 26);
    u8g2.print(PSTR("CURRENT LOC."));
    u8g2.setCursor(0, 35);
    u8g2.print(dtostrf(fix.latitude(), 11, 7, buf));
    u8g2.setCursor(0, 44);
    u8g2.print(dtostrf(fix.longitude(), 11, 6, buf));
    u8g2.setCursor(0, 53);
    u8g2.print(dtostrf(distanceToBaseKm, 11, 3, buf));
}

// void ConstructTimezonePage()
// {
//     //title
//     const char *buf = "TIMEZONE";
//     int len = strlen(buf);
//     len = len * 8;
//     uint8_t x = (128 - len) / 2;

//     const uint8_t *font;
//     font = u8g2_font_artossans8_8r;
//     u8g2.setFont(font);
//     u8g2.setCursor(x, 0);
//     u8g2.print(buf);

//     char temp[7];
//     dtostrf(configLocation.timezone, 0, 0, temp);
//     len = strlen(temp);

//     font = u8g2_font_inb19_mn;
//     font = bigNumbers21x26;
//     u8g2.setFont(font);
//     len = u8g2.getStrWidth(temp);
//     x = (128 - len) / 2;

//     font = u8g2_font_inb19_mn;
//     font = bigNumbers21x26;
//     u8g2.setFont(font);
//     u8g2.setCursor(x, 23);
//     u8g2.print(temp);

//     //menu text
//     // OzOled.drawFont8("Alarm     ", 0, 7);
// }

void ConstructTimezoneDBPage()
{
    enum enumTzDB
    {
        Tz,
        Save,
        Exit,
        EntryCount
    };

    static uint16_t pos = 0;
    uint8_t maxPos = EntryCount;

    static char tempBufZoneName[sizeof(configLocation.timezonename)];
    static char tempBufZoneString[sizeof(configLocation.timezonestring)];

    if (pageChangeFlag)
    {
        pos = 0;

        get_zones_info(tempBufZoneName, tempBufZoneString);

        // strlcpy(tempBufZoneName, configLocation.timezonename, sizeof(configLocation.timezonename));
        // strlcpy(tempBufZoneString, configLocation.timezonestring, sizeof(configLocation.timezonestring));
    }

    if (rightPinFlag)
    {
        if (mode == pageMode)
        {
            if (pos < maxPos - 1)
                pos++;

            DEBUGLOG("pos: %d\r\n", pos);
        }
    }
    else if (leftPinFlag)
    {
        if (mode == pageMode)
        {
            if (pos > 0)
                pos--;

            DEBUGLOG("pos: %d\r\n", pos);
        }
    }

    else if (switchPinToggled)
    {
        if (pos == Save)
        {
            // ConstructDateTimePage(pos, moment);
            // processSholatFlag = true;
            DEBUGLOG("Save\r\n");
        }
        else if (pos == maxPos - 1)
        {
            page = Settings;
            DEBUGLOG("Exit\r\n");
            // return;
        }
        else
        {
            if (mode == pageMode)
                mode = editMode;
            else
                mode = pageMode;
        }
    }

    // char buf[17];
    const uint8_t *font;

    uint16_t dispWidth = u8g2.getDisplayWidth();

    //title
    font = u8g2_font_artossans8_8r;
    // font = u8g2_font_profont11_tf;
    // font = u8g2_font_mozart_nbp_tr;
    // font = u8g2_font_artossans8_8u;

    u8g2.setFont(font);

    const char *text = "TIMEZONE DB";
    int len = u8g2.getStrWidth(text);
    uint16_t x = (dispWidth - len) / 2;

    uint8_t y = 0;

    u8g2.setCursor(x, y);
    u8g2.print(text);

    y = y + 9;

    u8g2.drawHLine(0, y, u8g2.getDisplayWidth());

    // bool mask = 1;
    bool color = mask;
    bool blinkColor = state500ms;

    //*** DB ***//
    char bufContinent[sizeof(configLocation.timezonename)];
    char bufCity[sizeof(configLocation.timezonename)];

    if (pos == Tz)
    {
        if (mode == editMode)
            color = blinkColor;
        else
            color = !mask;

        if (mode == editMode)
        {
            if (rightPinFlag || leftPinFlag)
            {
                get_zones_info(tempBufZoneName, tempBufZoneString);
            }
        }
    }
    else
        color = mask;

    u8g2.setDrawColor(color);

    u8g2.setFontMode(0);

    char *token; // this is used by strtok() as an index

    // DEBUGLOG("tempBufZoneName: %s\r\n", tempBufZoneName);
    char buf[sizeof(configLocation.timezonename)];
    strlcpy(buf, tempBufZoneName, sizeof(buf));

    token = strtok(buf, "/");
    strlcpy(bufContinent, token, strlen(token) + 1);
    token = strtok(NULL, "\0");
    strlcpy(bufCity, token, strlen(token) + 1);

    // DEBUGLOG("%s %s %s\r\n", bufContinent, bufCity, tempBufZoneString);

    x = u8g2.getStrWidth(bufContinent);
    x = (dispWidth - x) / 2;
    y = y + (1 * (u8g2.getMaxCharHeight() + 2));
    u8g2.setCursor(x, y);
    u8g2.print(bufContinent);

    x = u8g2.getStrWidth(bufCity);
    x = (dispWidth - x) / 2;
    y = y + (1 * (u8g2.getMaxCharHeight() + 2));
    u8g2.setCursor(x, y);

    u8g2.print(bufCity);



    int count = 0;
    bool skip = false;
    len = strlen(tempBufZoneString);
    uint8_t _case = 99;
    // int startChar = 0;
    char bufTzOffset[17];
    uint8_t tempI = 0;

    int offset = 0;

    for (int i = 0; i < len + 1; i++)
    {
        char c = tempBufZoneString[i];

        if (skip)
        {
            if (_case == 0)
            {
                if (offset == 0)
                {
                    if (c == '-')
                        c = '+';
                    else
                    {
                        bufTzOffset[offset] = '-';
                        offset++;
                    }
                }

                bufTzOffset[offset] = c;

                if (i == len)
                {
                    bufTzOffset[offset + 1] = '\0';
                    char buf[sizeof(bufTzOffset)];
                    sprintf(buf, "UTC%s", bufTzOffset);
                    strlcpy(bufTzOffset, buf, sizeof(bufTzOffset));
                    break;
                }

                offset++;
            }
            else if (_case == 1)
            {
                if (offset == 0)
                {
                    if (c == '-')
                        c = '+';
                    else
                    {
                        bufTzOffset[offset] = '-';
                        offset++;
                    }
                }

                bufTzOffset[offset] = c;

                if (c == '<')
                {
                    bufTzOffset[offset] = '\0';
                    char buf[sizeof(bufTzOffset)];
                    sprintf(buf, "UTC%s", bufTzOffset);
                    strlcpy(bufTzOffset, buf, sizeof(bufTzOffset));
                    break;
                }

                offset++;
            }
        }
        else if (!skip)
        {
            bufTzOffset[offset] = c;

            if (c == ',')
            {
                bufTzOffset[offset] = '\0';
                break;
            }
            else if (i == len)
            {
                skip = true;

                i = tempI;
                offset = 0;

                // DEBUGLOG("RESTART !!!\r\n");
            }
            else
                offset++;
        }

        if (c == '>' && skip == false)
        {
            count++;

            if (count == 1 && tempI == 0)
            {
                _case = 0;
                tempI = i;
            }
            else if (count == 2)
            {
                _case = 1;
                i = tempI;
                offset = 0;
                skip = true;
            }
        }
    }

    // DEBUGLOG("CASE: %d, tempI: %d\r\n", _case, tempI);

    x = u8g2.getStrWidth(bufTzOffset);
    x = (dispWidth - x) / 2;
    y = y + (1 * (u8g2.getMaxCharHeight() + 2));
    u8g2.setCursor(x, y);

    u8g2.print(bufTzOffset);

    u8g2.setDrawColor(mask);

    //*** SAVE ***//
    if (pos == Save)
    {
        color = !mask;

        if (switchPinToggled)
        {
            strlcpy(configLocation.timezonename, tempBufZoneName, sizeof(configLocation.timezonename));
            strlcpy(configLocation.timezonestring, tempBufZoneString, sizeof(configLocation.timezonestring));

            setenv("TZ", configLocation.timezonestring, 1 /*overwrite*/);
            // tzset();
        }
    }
    else
        color = mask;

    x = 0;
    y = y + (1 * (u8g2.getMaxCharHeight() + 2));
    u8g2.setCursor(x, y);
    u8g2.setDrawColor(color);
    u8g2.print(PSTR("Save"));
    u8g2.setDrawColor(mask);

    //*** EXIT ***//
    if (pos == Exit)
        color = !mask;
    else
        color = mask;

    x = 0;
    y = y + (1 * (u8g2.getMaxCharHeight() + 2));
    u8g2.setCursor(x, y);
    u8g2.setCursor(x, y);
    u8g2.setDrawColor(color);
    u8g2.print(PSTR("Back"));
    u8g2.setDrawColor(mask);
}

void ConstructSholatTimePage()
{
    char buf[17];
    const uint8_t *font;

    uint16_t dispWidth = u8g2.getDisplayWidth();

    //title
    font = u8g2_font_artossans8_8r;
    font = u8g2_font_profont11_tf;
    font = u8g2_font_mozart_nbp_tf;
    font = u8g2_font_mozart_nbp_tf;

    u8g2.setFont(font);

    const char *text = "JADWAL SHOLAT";
    int len = u8g2.getStrWidth(text);
    int16_t x = (dispWidth - len) / 2;

    u8g2.setCursor(x, 0);
    u8g2.print(text);

    u8g2.drawHLine(0, 9, u8g2.getDisplayWidth());

    int selected = CURRENTTIMEID;

    for (uint8_t i = 0; i < TimesCount; i++)
    {
        if (i != Sunset)
        {
            uint8_t hr, mnt;

            unsigned long tempTimestamp = timestampSholatTimesToday[i] + TimezoneSeconds();

            RtcDateTime tm;
            tm = RtcDateTime(tempTimestamp);

            hr = tm.Hour();
            mnt = tm.Minute();

            snprintf_P(buf, sizeof(buf), PSTR("%-7s : %02d:%02d"), sholatNameStr(i), hr, mnt);

            int offset = i;
            if (i > 4)
                offset--;

            int x = 0;
            int y = 10 + (offset * (8 + 1));

            if (i == selected)
            {
                u8g2.drawBox(x, y, u8g2.getDisplayWidth(), 9);
                u8g2.setDrawColor(0);
            }

            u8g2.setCursor(x, y);
            u8g2.print(buf);

            // reset draw color
            u8g2.setDrawColor(1);
        }
    }
}

void ConstructUptimePage()
{
    //title
    char buf[17];
    snprintf_P(buf, sizeof(buf), PSTR("UPTIME"));

    int len = strlen(buf);
    len = len * 8;
    uint8_t x = (128 - len) / 2;

    const uint8_t *font;
    font = u8g2_font_artossans8_8r;
    u8g2.setFont(font);
    u8g2.setCursor(x, 0);
    u8g2.print(buf);

    if (1)
    {
        // uptime strings
        unsigned long uptime = utcTime - lastBoot;

        uint16_t days;
        uint8_t hours;
        uint8_t minutes;
        uint8_t seconds;

        days = elapsedDays(uptime);
        hours = numberOfHours(uptime);
        minutes = numberOfMinutes(uptime);
        seconds = numberOfSeconds(uptime);

        snprintf_P(buf, sizeof(buf), PSTR("%u days"), days);
        len = strlen(buf);
        len = len * 8;
        x = (128 - len) / 2;
        u8g2.setCursor(x, 23);
        u8g2.print(buf);

        snprintf_P(buf, sizeof(buf), PSTR("%02d:%02d:%02d"), hours, minutes, seconds);
        len = strlen(buf);
        len = len * 8;
        x = (128 - len) / 2;
        u8g2.setCursor(x, 36);
        u8g2.print(buf);

        //menu text
        // OzOled.drawFont8("Alarm     ", 0, 7);
    }
}

void ConstructCompassPage()
{
    //title
    const char *buf = "COMPASS";
    int len = strlen(buf);
    len = len * 8;
    uint8_t x = (128 - len) / 2;

    const uint8_t *font;
    font = u8g2_font_artossans8_8r;
    u8g2.setFont(font);
    u8g2.setCursor(x, 0);
    u8g2.print(buf);

    // int len;
    char temp[7];
    // dtostrf(configLocation.timezone, 0, 0, temp);
    // len = strlen(temp);

    // const uint8_t *font;
    font = u8g2_font_inb19_mn;
    u8g2.setFont(font);
    len = u8g2.getMaxCharWidth();
    len = 3 * len; // max 3 chars
    x = (128 - len) / 2;
    u8g2.setCursor(x, 23);
    u8g2.print("   ");

    dtostrf(headingDegInt, 0, 0, temp);
    len = u8g2.getStrWidth(temp);
    x = (128 - len) / 2;
    u8g2.setCursor(x, 23);
    u8g2.print(temp);
}

void ConstructAlarmPage()
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
    // font = u8g2_font_helvR10_tf;
    // font = u8g2_font_inb19_mn;
    // font = u8g2_font_inb24_mn;
    font = u8g2_font_artossans8_8r;
    font = u8g2_font_chroma48medium8_8r;

    u8g2.setFont(font);

    // u8g2.setFontMode(0);

    char buf[36];

    if (state500ms)
    {
        snprintf_P(buf, sizeof(buf), PSTR("%d:%02d"), hourLocal, minLocal);
    }
    else
        snprintf_P(buf, sizeof(buf), PSTR("%d %02d"), hourLocal, minLocal);

    int x, y;

    x = 0;
    y = 8;

    font = u8g2_font_inb30_mf;

    u8g2.setFont(font);
    x = u8g2.getStrWidth(buf);

    x = (u8g2.getDisplayWidth() - x) / 2;

    u8g2.setCursor(x, y);

    u8g2.setFontPosTop();

    u8g2.print(buf);
}

void ConstructSystemInfoPage(int16_t _selected)
{
    char buf[17];
    const uint8_t *font;

    //title
    font = u8g2_font_artossans8_8r;
    font = u8g2_font_profont11_tf;
    font = u8g2_font_mozart_nbp_tr;
    // font = u8g2_font_artossans8_8u;

    u8g2.setFont(font);

    const char *text = "SYSTEM INFO";
    int len = u8g2.getStrWidth(text);
    uint8_t x = (128 - len) / 2;

    u8g2.setCursor(x, 0);
    u8g2.print(text);

    u8g2.drawHLine(0, 9, u8g2.getDisplayWidth());

    // time_t lastSyncLocal = lastSync + TimezoneSeconds();
    // RtcDateTime dt;
    // dt = RtcDateTime(lastSyncLocal);

    // s = dt.Second();
    // h = dt.Hour();
    // m = dt.Minute();

    // snprintf_P(buf, sizeof(buf), PSTR("%d:%02d:%02d "),
    //            h,
    //            m,
    //            s);

    // u8g2.setCursor(0, 9 + (1 * (u8g2.getMaxCharHeight() + 2)));
    // u8g2.print(buf);

    const char *menu[] =
        {
            "HEAP",
            "LAST SYNC",
            "MENU_2",
            "MENU_3",
            "MENU_4",
            "MENU_5",
            "MENU_6",
            "MENU_7",
            "MENU_8",
            "MENU_9",
            "MENU_10"};

    uint16_t yStart = 11;

    uint16_t rowHeight = u8g2.getMaxCharHeight() + 1;
    uint16_t maxMenuToDisplay = (u8g2.getDisplayHeight() - yStart) / rowHeight;

    uint16_t menuCount = sizeof(menu) / sizeof(const char *);

    static int16_t iStart = 0;
    static int16_t iEnd = iStart + maxMenuToDisplay;

    // int selected = _selected;

    static int selected = 0;

    // bool menuIdIncrease = false;
    // bool menuIdDecrease = false;

    if (mode == editMode)
    {
    }

    if (_selected == 1)
        selected++;
    else if (_selected == 0)
        selected--;

    if (selected < 0)
        selected = 0;
    else if (selected >= menuCount)
        selected = menuCount - 1;

    if (_selected == 1 && selected >= iEnd)
    {
        iStart++;
        iEnd++;
    }
    else if (_selected == 0 && selected < iStart)
    {
        iStart--;
        iEnd--;
    }

    if (iStart < 0)
        iStart = 0;
    if (iEnd > menuCount)
        iEnd = menuCount;

    // if(selected + maxMenuToDisplay > )

    // if (selected == iEnd + 1)
    // {
    //     iEnd = selected;
    //     iStart = (iEnd - maxMenuToDisplay) + 1;
    // }

    uint8_t z = 0;

    for (uint8_t i = iStart; i < iEnd; i++)
    {
        snprintf_P(buf, sizeof(buf), PSTR("%s"), menu[i]);

        int x;
        int y;

        x = u8g2.getStrWidth(buf);
        x = (128 - x) / 2;

        y = yStart + (z * (rowHeight));

        if (i == selected)
        {
            u8g2.drawBox(0, y, u8g2.getDisplayWidth(), rowHeight + 1);
            u8g2.setDrawColor(0);
        }

        u8g2.setCursor(x, y);
        u8g2.print(buf);

        // reset draw color
        u8g2.setDrawColor(1);

        z++;
    }
}

// void ConstructMainMenuPage()
// {
//     char buf[17];
//     const uint8_t *font;

//     //title
//     font = u8g2_font_artossans8_8r;
//     font = u8g2_font_profont11_tf;
//     font = u8g2_font_mozart_nbp_tr;
//     // font = u8g2_font_artossans8_8u;

//     u8g2.setFont(font);

//     const char *text = "SYSTEM INFO";
//     int len = u8g2.getStrWidth(text);
//     uint8_t x = (128 - len) / 2;

//     u8g2.setCursor(x, 0);
//     // u8g2.print(text);

//     // u8g2.drawBitmap(0,0,4,32, menu_default);
//     u8g2.drawXBMP(0,0,8,64, menu_alarm);
// }

void ConstructMainMenuPage(int16_t _selected)
{
    char buf[17];
    const uint8_t *font;

    //title
    font = u8g2_font_artossans8_8r;
    font = u8g2_font_profont11_tf;
    font = u8g2_font_mozart_nbp_tr;
    // font = u8g2_font_artossans8_8u;

    u8g2.setFont(font);

    const char *text = "MAIN MENU";
    int len = u8g2.getStrWidth(text);
    uint8_t x = (128 - len) / 2;

    u8g2.setCursor(x, 0);
    u8g2.print(text);

    u8g2.drawHLine(0, 9, u8g2.getDisplayWidth());

    // time_t lastSyncLocal = lastSync + TimezoneSeconds();
    // RtcDateTime dt;
    // dt = RtcDateTime(lastSyncLocal);

    // s = dt.Second();
    // h = dt.Hour();
    // m = dt.Minute();

    // snprintf_P(buf, sizeof(buf), PSTR("%d:%02d:%02d "),
    //            h,
    //            m,
    //            s);

    // u8g2.setCursor(0, 9 + (1 * (u8g2.getMaxCharHeight() + 2)));
    // u8g2.print(buf);

    const char *menu[] =
        {
            "SPEEDOMETER",
            "COMPASS",
            "BASE LOCATION",
            "SHOLAT TIME",
            "SETTINGS",
            "DIAGNOSTIC",
            "EXIT"};

    uint16_t yStart = 11;

    uint16_t rowHeight = u8g2.getMaxCharHeight() + 1;
    uint16_t maxMenuToDisplay = (u8g2.getDisplayHeight() - yStart) / rowHeight;

    // uint16_t menuCount = sizeof(menu) / sizeof(const char *);
    uint16_t menuCount = MainMenuCount;

    static int16_t iStart = 0;
    static int16_t iEnd = iStart + maxMenuToDisplay;

    // int selected = _selected;

    static int selected = 0;

    bool dirUP = false;
    bool dirDown = false;

    if (_selected > selected)
        dirUP = true;
    else if (_selected < selected)
        dirDown = true;

    selected = _selected;

    if (selected < 0)
        selected = 0;
    else if (selected >= menuCount)
        selected = menuCount - 1;

    if (dirUP && selected >= iEnd)
    {
        iStart++;
        iEnd++;
    }
    else if (dirDown && selected < iStart)
    {
        iStart--;
        iEnd--;
    }

    if (iStart < 0)
        iStart = 0;
    if (iEnd > menuCount)
        iEnd = menuCount;

    // if(selected + maxMenuToDisplay > )

    // if (selected == iEnd + 1)
    // {
    //     iEnd = selected;
    //     iStart = (iEnd - maxMenuToDisplay) + 1;
    // }

    uint8_t z = 0;

    for (uint8_t i = iStart; i < iEnd; i++)
    {
        snprintf_P(buf, sizeof(buf), PSTR("%s"), menu[i]);

        int x;
        int y;

        x = u8g2.getStrWidth(buf);
        x = (128 - x) / 2;

        y = yStart + (z * (rowHeight));

        if (i == selected)
        {
            u8g2.drawBox(0, y, u8g2.getDisplayWidth(), rowHeight + 1);
            u8g2.setDrawColor(0);
        }

        u8g2.setCursor(x, y);
        u8g2.print(buf);

        // reset draw color
        u8g2.setDrawColor(1);

        z++;
    }
}

void ConstructSetGPSPage(int16_t _selected)
{
    char buf[17];
    const uint8_t *font;

    //title
    font = u8g2_font_artossans8_8r;
    font = u8g2_font_profont11_tf;
    font = u8g2_font_mozart_nbp_tr;
    // font = u8g2_font_artossans8_8u;

    u8g2.setFont(font);

    const char *text = "GPS SETTINGS";
    int len = u8g2.getStrWidth(text);
    uint8_t x = (128 - len) / 2;

    u8g2.setCursor(x, 0);
    u8g2.print(text);

    u8g2.drawHLine(0, 9, u8g2.getDisplayWidth());

    // time_t lastSyncLocal = lastSync + TimezoneSeconds();
    // RtcDateTime dt;
    // dt = RtcDateTime(lastSyncLocal);

    // s = dt.Second();
    // h = dt.Hour();
    // m = dt.Minute();

    // snprintf_P(buf, sizeof(buf), PSTR("%d:%02d:%02d "),
    //            h,
    //            m,
    //            s);

    // u8g2.setCursor(0, 9 + (1 * (u8g2.getMaxCharHeight() + 2)));
    // u8g2.print(buf);

    const char *menu[] =
        {
            "ENABLE RMC",
            "ENABLE GGA",
            "ENABLE TimTP",
            "ENABLE VTG",
            "RATE",
            "BAUD",
            "EXIT"};

    uint16_t yStart = 11;

    uint16_t rowHeight = u8g2.getMaxCharHeight() + 1;
    uint16_t maxMenuToDisplay = (u8g2.getDisplayHeight() - yStart) / rowHeight;

    // uint16_t menuCount = sizeof(menu) / sizeof(const char *);
    uint16_t menuCount = SetGPSCount;

    static int16_t iStart = 0;
    static int16_t iEnd = iStart + maxMenuToDisplay;

    // int selected = _selected;

    static int selected = 0;

    bool dirUP = false;
    bool dirDown = false;

    if (_selected > selected)
        dirUP = true;
    else if (_selected < selected)
        dirDown = true;

    selected = _selected;

    if (selected < 0)
        selected = 0;
    else if (selected >= menuCount)
        selected = menuCount - 1;

    if (dirUP && selected >= iEnd)
    {
        iStart++;
        iEnd++;
    }
    else if (dirDown && selected < iStart)
    {
        iStart--;
        iEnd--;
    }

    if (iStart < 0)
        iStart = 0;
    if (iEnd > menuCount)
        iEnd = menuCount;

    // if(selected + maxMenuToDisplay > )

    // if (selected == iEnd + 1)
    // {
    //     iEnd = selected;
    //     iStart = (iEnd - maxMenuToDisplay) + 1;
    // }

    uint8_t z = 0;

    for (uint8_t i = iStart; i < iEnd; i++)
    {
        snprintf_P(buf, sizeof(buf), PSTR("%s"), menu[i]);

        int x;
        int y;

        x = u8g2.getStrWidth(buf);
        x = (128 - x) / 2;

        y = yStart + (z * (rowHeight));

        if (i == selected)
        {
            u8g2.drawBox(0, y, u8g2.getDisplayWidth(), rowHeight + 1);
            u8g2.setDrawColor(0);
        }

        u8g2.setCursor(x, y);
        u8g2.print(buf);

        // reset draw color
        u8g2.setDrawColor(1);

        z++;
    }
}

void ConstructSettingsPage(int16_t _selected)
{
    char buf[17];
    const uint8_t *font;

    //title
    font = u8g2_font_artossans8_8r;
    font = u8g2_font_profont11_tf;
    font = u8g2_font_mozart_nbp_tr;
    // font = u8g2_font_artossans8_8u;

    u8g2.setFont(font);

    const char *text = "SETTINGS";
    int len = u8g2.getStrWidth(text);
    uint8_t x = (128 - len) / 2;

    u8g2.setCursor(x, 0);
    u8g2.print(text);

    u8g2.drawHLine(0, 9, u8g2.getDisplayWidth());

    // time_t lastSyncLocal = lastSync + TimezoneSeconds();
    // RtcDateTime dt;
    // dt = RtcDateTime(lastSyncLocal);

    // s = dt.Second();
    // h = dt.Hour();
    // m = dt.Minute();

    // snprintf_P(buf, sizeof(buf), PSTR("%d:%02d:%02d "),
    //            h,
    //            m,
    //            s);

    // u8g2.setCursor(0, 9 + (1 * (u8g2.getMaxCharHeight() + 2)));
    // u8g2.print(buf);

    const char *menu[] =
        {
            "DATE/TIME",
            "TIMEZONE",
            "TIMEZONE DB",
            "GPS",
            "MENU_3",
            "EXIT"};

    uint16_t yStart = 11;

    uint16_t rowHeight = u8g2.getMaxCharHeight() + 1;
    uint16_t maxMenuToDisplay = (u8g2.getDisplayHeight() - yStart) / rowHeight;

    // uint16_t menuCount = sizeof(menu) / sizeof(const char *);
    uint16_t menuCount = SettingsMenuCount;

    static int16_t iStart = 0;
    static int16_t iEnd = iStart + maxMenuToDisplay;

    // int selected = _selected;

    static int selected = 0;

    bool dirUP = false;
    bool dirDown = false;

    if (_selected > selected)
        dirUP = true;
    else if (_selected < selected)
        dirDown = true;

    selected = _selected;

    if (selected < 0)
        selected = 0;
    else if (selected >= menuCount)
        selected = menuCount - 1;

    if (dirUP && selected >= iEnd)
    {
        iStart++;
        iEnd++;
    }
    else if (dirDown && selected < iStart)
    {
        iStart--;
        iEnd--;
    }

    if (iStart < 0)
        iStart = 0;
    if (iEnd > menuCount)
        iEnd = menuCount;

    // if(selected + maxMenuToDisplay > )

    // if (selected == iEnd + 1)
    // {
    //     iEnd = selected;
    //     iStart = (iEnd - maxMenuToDisplay) + 1;
    // }

    uint8_t z = 0;

    for (uint8_t i = iStart; i < iEnd; i++)
    {
        snprintf_P(buf, sizeof(buf), PSTR("%s"), menu[i]);

        int x;
        int y;

        x = u8g2.getStrWidth(buf);
        x = (128 - x) / 2;

        y = yStart + (z * (rowHeight));

        if (i == selected)
        {
            u8g2.drawBox(0, y, u8g2.getDisplayWidth(), rowHeight + 1);
            u8g2.setDrawColor(0);
        }

        u8g2.setCursor(x, y);
        u8g2.print(buf);

        // reset draw color
        u8g2.setDrawColor(1);

        z++;
    }
}

void ConstructDateTimePage(int16_t _pos, uint32_t _moment)
{
    char buf[17];
    const uint8_t *font;

    //title
    font = u8g2_font_artossans8_8r;
    // font = u8g2_font_profont11_tf;
    // font = u8g2_font_mozart_nbp_tr;
    // font = u8g2_font_artossans8_8u;

    u8g2.setFont(font);

    const char *text = "SET DATE & TIME";
    int len = u8g2.getStrWidth(text);
    uint8_t x = (128 - len) / 2;

    uint8_t y = 0;

    u8g2.setCursor(x, y);
    u8g2.print(text);

    y = y + 9;

    u8g2.drawHLine(0, y, u8g2.getDisplayWidth());

    static uint32_t moment = 0;

    static int8_t day = 0;
    static int8_t month = 0;
    static int16_t year = 0;
    static int8_t hour = 0;
    static int8_t minute = 0;
    static int8_t second = 0;

    if (moment != _moment)
    {
        moment = _moment;

        uint32_t timestamp = moment + TimezoneSeconds();
        RtcDateTime dt;
        // dt = RtcDateTime(timestamp);
        dt.InitWithEpoch32Time(timestamp);

        day = dt.Day();
        month = dt.Month();
        year = dt.Year();
        hour = dt.Hour();
        minute = dt.Minute();
        second = 0;
    }

    int16_t pos = _pos;

    // bool mask = 1;
    bool color = mask;
    bool blinkColor = state500ms;

    //*** DATE ***//
    char monStr[4];
    strcpy(monStr, monthShortStr(month));

    /*
    snprintf_P(buf, sizeof(buf), PSTR("%s,%d-%s SAT%2d"),
               dayShortStr(dtLocal.DayOfWeek()),
               dtLocal.Day(),
               mon,
               fix.satellites);
               */

    snprintf_P(buf, sizeof(buf), PSTR("%02d %s %04d"),
               //    dayShortStr(dt.DayOfWeek()),
               day,
               monStr,
               year);

    x = u8g2.getStrWidth(buf);
    x = (128 - x) / 2;
    y = y + (1 * (u8g2.getMaxCharHeight() + 2));

    u8g2.setCursor(x, y);

    u8g2.setFontMode(0);

    //*** YEAR ***//
    if (pos == 0)
    {
        if (mode == editMode)
            color = blinkColor;
        else
            color = !mask;

        if (mode == editMode)
        {
            if (rightPinFlag)
            {
                year++;
                if (year > 2100)
                    year = 2018;
            }

            if (leftPinFlag)
            {
                year--;
                if (year < 2018)
                    year = 2100;
            }
        }
    }
    else
        color = mask;

    u8g2.setDrawColor(color);
    u8g2.print(year);
    u8g2.setDrawColor(1);

    // space
    u8g2.print(PSTR(" "));

    //*** Month ***//
    if (pos == 1)
    {
        if (mode == editMode)
            color = blinkColor;
        else
            color = !mask;

        if (mode == editMode)
        {
            if (rightPinFlag)
            {
                month++;
                if (month > 12)
                    month = 1;
            }

            if (leftPinFlag)
            {
                month--;
                if (month < 1)
                    month = 12;
            }
        }
    }
    else
        color = mask;

    strcpy(monStr, monthShortStr(month));
    u8g2.setDrawColor(color);
    u8g2.print(monStr);
    u8g2.setDrawColor(mask);

    // space
    u8g2.print(PSTR(" "));

    //*** DAY ***//

    if (pos == 2)
    {
        if (mode == editMode)
            color = blinkColor;
        else
            color = !mask;

        if (mode == editMode)
        {
            // January - 31 days
            // February - 28 days in a common year and 29 days in leap years
            // March - 31 days
            // April - 30 days
            // May - 31 days
            // June - 30 days
            // July - 31 days
            // August - 31 days
            // September - 30 days
            // October - 31 days
            // November - 30 days
            // December - 31 days

            uint8_t maxDays;

            if (month == 2)
                if (year % 4 == 0)
                    maxDays = 28;
                else
                    maxDays = 29;
            else if (month == 4 || month == 6 || month == 9 || month == 11)
                maxDays = 30;
            else
                maxDays = 31;

            if (rightPinFlag)
            {
                day++;
                if (day > maxDays)
                    day = 1;
            }

            if (leftPinFlag)
            {
                day--;
                if (day < 1)
                    day = maxDays;
            }
        }
    }
    else
        color = mask;

    u8g2.setDrawColor(color);
    u8g2.print(day);
    u8g2.setDrawColor(mask);

    //*** HOUR ***//
    if (pos == 3)
    {
        if (mode == editMode)
            color = blinkColor;
        else
            color = !mask;

        if (mode == editMode)
        {
            if (rightPinFlag)
            {
                hour++;
                if (hour > 23)
                    hour = 0;
            }

            if (leftPinFlag)
            {
                hour--;
                if (hour < 0)
                    hour = 23;
            }
        }
    }
    else
        color = mask;

    snprintf_P(buf, sizeof(buf), PSTR("%02d:%02d:%02d"), hour, minute, second);

    x = u8g2.getStrWidth(buf);
    x = (128 - x) / 2;
    y = y + (2 * (u8g2.getMaxCharHeight() + 2));

    snprintf_P(buf, sizeof(buf), PSTR("%02d"), hour);

    u8g2.setCursor(x, y);
    u8g2.setDrawColor(color);
    u8g2.print(buf);
    u8g2.setDrawColor(mask);

    u8g2.print(":");

    //*** MINUTE ***//
    if (pos == 4)
    {
        if (mode == editMode)
            color = blinkColor;
        else
            color = !mask;

        if (mode == editMode)
        {
            if (rightPinFlag)
            {
                minute++;
                if (minute > 59)
                    minute = 0;
            }

            if (leftPinFlag)
            {
                minute--;
                if (minute < 0)
                    minute = 59;
            }
        }
    }
    else
        color = mask;

    snprintf_P(buf, sizeof(buf), PSTR("%02d"), minute);
    u8g2.setDrawColor(color);
    u8g2.print(buf);
    u8g2.setDrawColor(mask);

    // colon
    u8g2.print(":");

    //*** SECOND ***//
    if (pos == 5)
    {
        if (mode == editMode)
            color = blinkColor;
        else
            color = !mask;

        if (mode == editMode)
        {
            if (rightPinFlag)
            {
                second++;
                if (second > 59)
                    second = 0;
            }

            if (leftPinFlag)
            {
                second--;
                if (second < 0)
                    second = 59;
            }
        }
    }
    else
        color = mask;

    snprintf_P(buf, sizeof(buf), PSTR("%02d"), second);
    u8g2.setDrawColor(color);
    u8g2.print(buf);
    u8g2.setDrawColor(mask);

    //*** Text Save ***//
    if (pos == 6)
    {
        color = !mask;

        if (switchPinToggled)
        {
            // RtcDateTime dt;

            RtcDateTime dt = RtcDateTime(year, month, day, hour, minute, second);

            // convert t to utc
            dt -= TimezoneSeconds();
            // PRINT("\r\nTimestamp to set: %u\r\n", dt.Epoch32Time());

            // // finally, set rtc time;
            // Rtc.SetDateTime(dt);
            // PRINT("RTC has been updated!\r\n");

            // sync system time
            // utcTime = dt.Epoch32Time(); // use this if you need EPOCH from 1970
            uint32_t newtime = dt.TotalSeconds(); // use this if you need EPOCH from 2000
            SyncTime(newtime);
            Serial.print("\r\nSystem time has been updated!\r\n");

            // sync system time
            // timeval tv = {dt.Epoch32Time(), 0};
            // timezone tz = {0, 0};
            // settimeofday(&tv, &tz);
            // PRINT("System time has been updated!\r\n\r\n");

            // update last sync
            lastSync = utcTime;

            // processSholatFlag = true;

            // // convert t to utc
            // dt -= TimezoneSeconds();
            // PRINT("\r\nTimestamp to set: %u\r\n", dt.Epoch32Time());

            // // finally, set rtc time;
            // Rtc.SetDateTime(dt);
            // PRINT("RTC has been updated!\r\n");

            // // sync system time
            // timeval tv = {dt.Epoch32Time(), 0};
            // timezone tz = {0, 0};
            // settimeofday(&tv, &tz);
            // PRINT("System time has been updated!\r\n\r\n");

            // // update last sync
            // lastSync = dt.Epoch32Time();

            // // reset to no
            // yesNo = false;

            // tone1 = HIGH;
        }
    }
    else
        color = mask;

    x = 0;
    y = y + (1 * (u8g2.getMaxCharHeight() + 2));
    u8g2.setCursor(x, y);
    u8g2.setDrawColor(color);
    u8g2.print(PSTR("Save"));
    u8g2.setDrawColor(mask);

    //*** Text Back ***//
    if (pos == 7)
        color = !mask;
    else
        color = mask;

    x = 0;
    y = y + (1 * (u8g2.getMaxCharHeight() + 2));
    u8g2.setCursor(x, y);
    u8g2.setCursor(x, y);
    u8g2.setDrawColor(color);
    u8g2.print(PSTR("Back"));
    u8g2.setDrawColor(mask);
}