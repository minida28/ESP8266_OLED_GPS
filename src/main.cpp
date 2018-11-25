#include <Arduino.h>
#include <Wire.h>
#include "pinouthelper.h"
#include "displayhelper.h"
#include "gpshelper.h"
#include "timehelper.h"
#include "encoderhelper.h"
#include "buzzerhelper.h"
#include "sholathelper.h"
#include "pageshelper.h"
#include "magnetometerhelper.h"
#include <Ticker.h>
#include "timezonehelper.h"
#include <FS.h>

//--------------------------

#define DEBUGPORT Serial

#define SKIP

// #define RELEASE

#ifndef RELEASE
#define DEBUGLOG(fmt, ...)                       \
    {                                            \
        static const char pfmt[] PROGMEM = fmt;  \
        DEBUGPORT.printf_P(pfmt, ##__VA_ARGS__); \
    }
#else
#define DEBUGLOG(...)
#endif

bool processSholatFlag = false;
bool processSholat2ndStageFlag = false;

// void ConstructClockPage(){}

// bool state1000ms;
bool ticker1000msFlag = false;

// void flip1000ms()
// {
//     state1000ms = !state1000ms;
// }

void flip500ms()
{
    state500ms = !state500ms;
}

void ticker1000ms()
{
    // utcTime++;
    // tick1000ms = true;
    // DEBUGLOG("\r\ntick %lu", millis());
    ticker1000msFlag = true;
    // utcTime++;
}

Ticker tickerFlip500ms;
Ticker tickerTick1000ms;

int I2C_ClearBus()
{
#if defined(TWCR) && defined(TWEN)
    TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
#endif

    pinMode(SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
    pinMode(SCL, INPUT_PULLUP);

    // Wait 2.5 secs, i.e. delay(2500). This is strictly only necessary on the first power
    // up of the DS3231 module to allow it to initialize properly,
    // but is also assists in reliable programming of FioV3 boards as it gives the
    // IDE a chance to start uploaded the program
    // before existing sketch confuses the IDE by sending Serial data.
    // Serial.println(F("Delay 2.5 secs to allow DS3231 module to initialize properly"));
    // DEBUGLOG("Delay 2.5 secs to allow DS3231 module to initialize properly");
    delay(2500);
    boolean SCL_LOW = (digitalRead(SCL) == LOW); // Check is SCL is Low.
    if (SCL_LOW)
    {             //If it is held low Arduno cannot become the I2C master.
        return 1; //I2C bus error. Could not clear SCL clock line held low
    }

    boolean SDA_LOW = (digitalRead(SDA) == LOW); // vi. Check SDA input.
    int clockCount = 20;                         // > 2x9 clock

    while (SDA_LOW && (clockCount > 0))
    { //  vii. If SDA is Low,
        clockCount--;
        // Note: I2C bus is open collector so do NOT drive SCL or SDA high.
        pinMode(SCL, INPUT);        // release SCL pullup so that when made output it will be LOW
        pinMode(SCL, OUTPUT);       // then clock SCL Low
        delayMicroseconds(10);      //  for >5uS
        pinMode(SCL, INPUT);        // release SCL LOW
        pinMode(SCL, INPUT_PULLUP); // turn on pullup resistors again
        // do not force high as slave may be holding it low for clock stretching.
        delayMicroseconds(10); //  for >5uS
        // The >5uS is so that even the slowest I2C devices are handled.
        SCL_LOW = (digitalRead(SCL) == LOW); // Check if SCL is Low.
        int counter = 20;
        while (SCL_LOW && (counter > 0))
        { //  loop waiting for SCL to become High only wait 2sec.
            counter--;
            delay(100);
            SCL_LOW = (digitalRead(SCL) == LOW);
        }
        if (SCL_LOW)
        {             // still low after 2 sec error
            return 2; // I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec
        }
        SDA_LOW = (digitalRead(SDA) == LOW); //   and check SDA input again and loop
    }
    if (SDA_LOW)
    {             // still low
        return 3; // I2C bus error. Could not clear. SDA data line held low
    }

    // else pull SDA line low for Start or Repeated Start
    pinMode(SDA, INPUT);  // remove pullup.
    pinMode(SDA, OUTPUT); // and then make it LOW i.e. send an I2C Start or Repeated start control.
    // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
    /// A Repeat Start is a Start occurring after a Start with no intervening Stop.
    delayMicroseconds(10);      // wait >5uS
    pinMode(SDA, INPUT);        // remove output low
    pinMode(SDA, INPUT_PULLUP); // and make SDA high i.e. send I2C STOP control.
    delayMicroseconds(10);      // x. wait >5uS
    pinMode(SDA, INPUT);        // and reset pins as tri-state inputs which is the default state on reset
    pinMode(SCL, INPUT);
    return 0; // all ok
}

void setup()
{
    // Serial.begin(115200);
    // Wire.begin();
    pinMode(LED_1, OUTPUT);
    digitalWrite(LED_1, HIGH);

    Serial.begin(115200);

    SPIFFS.begin();

    ENCODERsetup();

    // -------------------------------------------------------------------
    // Setup I2C stuffs
    // -------------------------------------------------------------------
    DEBUGLOG("\r\n\r\nClearing I2C Bus\r\n"); //http://www.forward.com.au/pfod/ArduinoProgramming/I2C_ClearBus/index.html
    int rtn = I2C_ClearBus();                 // clear the I2C bus first before calling Wire.begin()
    if (rtn != 0)
    {
        DEBUGLOG("I2C bus error. Could not clear\r\n");
        if (rtn == 1)
        {
            DEBUGLOG("SCL clock line held low\r\n");
        }
        else if (rtn == 2)
        {
            DEBUGLOG("SCL clock line held low by slave clock stretch\r\n");
        }
        else if (rtn == 3)
        {
            DEBUGLOG("SDA data line held low\r\n");
        }
    }
    else
    {
        DEBUGLOG("bus clear, re-enable Wire.begin()\r\n");
        Wire.begin(SDA, SCL);
        // Wire.begin();
    }

    DisplaySetup();

    MAGNETOMETERsetup();

    // GPSsetup();

    if (1)
    {
        char buf[17];
        snprintf_P(buf, sizeof(buf), PSTR("Waiting GPS"));
        // OzOled.drawFont8(buf, 0, 0);
        const uint8_t *font;
        font = u8g2_font_artossans8_8r;
        u8g2.setFont(font);
        u8g2.setCursor(0, 0);
        u8g2.print(buf);
        u8g2.sendBuffer();

        // GPSsetup();

        if (!gpsPort.isListening())
            gpsPort.begin(BAUD_GPS);

        // char c = 0;

        while (true)
        {
            if (gpsPort.available())
                if (gpsPort.find("$GN"))
                    break;
        }

        GPSsetup();

        unsigned long start = 0;
        uint8_t count = 0;
        while (true)
        {
            GPSloop();

            if (parseGPScompleted)
            {
                parseGPScompleted = false;

                count++;

                if (count >= 2)
                    if (fix.valid.status)
                        if (fix.valid.date && fix.valid.time)
                            break;
            }

            if (millis() - start >= 500)
            {
                // DEBUGLOG("\r\nstatus: %d, date: %d, time: %d",
                // fix.valid.status,
                // fix.valid.date,
                // fix.valid.time);

                doSomeWork();

                start = millis();

                static int8_t x = 80;

                if (x == 80)
                {
                    x = x + 8;
                    u8g2.setCursor(x, 0);
                    u8g2.print("   ");
                }
                else if (x < 112)
                {
                    // OzOled.drawFont8(".", x, 0);
                    u8g2.setCursor(x, 0);
                    u8g2.print(".");
                    x = x + 8;
                }
                else
                {
                    x = 88;
                    // OzOled.drawFont8("   ", x, 0);
                    u8g2.setCursor(x, 0);
                    u8g2.print("   ");
                }

                u8g2.sendBuffer();
            }

            // break;
        }

        // tickerTick1000ms.attach_ms(1000, ticker1000ms);

        //Initialize Ticker every 0.5s
        // timer1_isr_init();
        // timer1_attachInterrupt(ticker1000ms);
        // timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
        // timer1_write(312500); //1000000 us

        if (1)
        {
            uint32_t gpsTime = fix.dateTime;

            TimeSetup();

            SyncTime(gpsTime);

            TimeLoop();

            // ProcessTimestamp(utcTime);
            // ProcessTimestamp(gpsTime);

            DEBUGLOG("utcTime: %lu\r\n", utcTime);
            lastBoot = utcTime - millis() / 1000;
            lastSync = utcTime;

            // tickerTick1000ms.attach_ms(1000, ticker1000ms);

            // //Initialize Ticker every 0.5s
            // // timer1_isr_init();
            // timer1_attachInterrupt(ticker1000ms);
            // timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
            // timer1_write(312500); //1000000 us

            // RtcDateTime dt = utcTime;
            // uint32_t t = dt.Epoch32Time();
            // TimeSetup(t);

            // ProcessTimestamp(utcTime);

            DEBUGLOG("Sync! %s %s\r\n", getDateStr(localTime), getTimeStr(localTime));

            if (fix.valid.location)
            {
                // configLocation.latitude = fix.latitude();
                // configLocation.longitude = fix.longitude();
            }

            process_sholat();
            process_sholat_2nd_stage();
        }
    }

    // utcTime = 592953120 - 5; // simulate 5 seconds before subuh at Bekasi
    // utcTime = 592953120 - 305; // simulate 5 minutes before subuh at Bekasi

    // time_t _epoch = 2147483647 - 10;

    // timeval tv = {_epoch, 0};
    // timezone tz = {0, 0};
    // settimeofday(&tv, &tz);

    // gettimeofday(&tv, nullptr);
    // //   clock_gettime(0, &tp);

    // utcTime = time(nullptr);

    page = Clock;

    DEBUGLOG("Setup completed\r\n");

    // gpsPort.end();
}

void loop()
{
    parseGPScompleted = false;
    tick500ms = 0;
    tick1000ms = 0;
    // static byte page = TimezoneDB; // Clock
    // static byte page = Clock;
    // static byte page = SystemInfo;

    bool redrawPage = 0;
    static bool gpsPortEnabled = 0;

    if (gpsPortEnabled)
        GPSloop();

    TimeLoop();

    // if (ticker1000msFlag)
    // {

    //     ticker1000msFlag = false;
    //     utcTime++;
    //     // DEBUGLOG("\r\nFLAG\t%lu", utcTime);
    // }

    // if (!timer1_enabled())
    // {
    //     timer1_attachInterrupt(ticker1000ms);
    //     timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
    //     timer1_write(312500); //1000000 us
    // }

    if (PPS)
    {
        PPS = false;
        // gpsPort.end();
        // Tone0(buzzerPin, 4000, 10, true);
        // gpsPort.begin(BAUD_GPS);
        // Tone1(buzzerPin, 512);
        // DEBUGLOG("\r\nPPS\t%lu", millis());

        // timer1_detachInterrupt();
        // timer1_attachInterrupt(ticker1000ms);
        // timer1_enable(TIM_DIV256, TIM_EDGE, TIM_LOOP);
        // timer1_write(312500); //1000000 us

        // if (tickerTick1000ms.active())
        // {
        //     tickerTick1000ms.detach();
        //     tickerTick1000ms.attach_ms(1000, ticker1000ms);
        // }
    }

    if (RMCFlag)
    {
        RMCFlag = false;
        // gpsPort.end();
        // Tone0(buzzerPin, 4000, 10, true);
        // gpsPort.begin(BAUD_GPS);
        // Tone1(buzzerPin, 512);
        // DEBUGLOG("\r\nRMC\t%lu", millis());
    }

    if (parseGPScompleted)
    {
        if (fix.valid.location)
        {
            distanceToBaseKm = haversine(fix.latitude(), fix.longitude(), configLocation.latitude, configLocation.longitude) / 1000UL;

            // if ((int)distanceToBaseKm >= 10)
            // {
            //   configLocation.latitude = fix.latitude();
            //   configLocation.longitude = fix.longitude();

            //   updateEEPROM();

            //   process_sholat();
            // }
        }
    }

    static bool syncFlag = false;

    //*** SYNC INTERVAL
    if (!syncFlag && utcTime % 600 == 0)
    {
        syncFlag = true;
    }

    if (syncFlag ||
        page == Speedometer ||
        page == BaseLocation)
        gpsPortEnabled = true;
    else
        gpsPortEnabled = false;

    if (gpsPortEnabled)
    {
        if (!gpsPort.isListening())
        {
            gpsPort.begin(BAUD_GPS);
            DEBUGLOG("gpsPort turned On, baud: %lu\r\n", BAUD_GPS);
        }
    }
    else
    {
        // if (!parseGPScompleted)
        //     return;

        if (gpsPort.isListening())
        {
            gpsPort.end();
            gpsPort.flush();
            DEBUGLOG("gpsPort turned Off\r\n");
        }
    }

    if (parseGPScompleted)
    {
        if (fix.valid.time && fix.valid.date)
        {
            uint32_t gpsTime = fix.dateTime;

            // DEBUGLOG("\r\nFIX\t%u\r\n", gpsTime);

            if (syncFlag)
            {
                syncFlag = false;

                RtcDateTime dt = gpsTime;
                uint32_t t = dt.Epoch32Time();

                if (t == utcTime)
                {
                    DEBUGLOG("\r\nSYNC NOT REQUIRED\r\n");
                }
                else
                {
                    // gpsTime = 1200798847 + 10; // y2k38 + 10 seconds

                    SyncTime(gpsTime);

                    lastSync = utcTime;

                    DEBUGLOG("\r\nSYNC utcTime: %lu\r\n", utcTime);

                    processSholatFlag = true;
                }
            }
        }
    }

    static uint32_t utcTime_old = 0;

    if (utcTime != utcTime_old)
    {
        utcTime_old = utcTime;
        // prevMillis = currentMillis;

        // DEBUGLOG("\r\ntick\t%lu", utcTime);

        // ProcessTimestamp(utcTime);

        tick1000ms = true;
    }

    ENCODERloop();

    if (switchPinToggled)
    {
        DEBUGLOG("pin toggled\r\n");
        // switchPinToggled = false;

        // redrawPage = true;

        gpsPort.end();
        Tone0(buzzerPin, 4000, 30, true);
        gpsPort.begin(BAUD_GPS);

        if (page == Clock)
        {
            page = MainMenu;
        }
        else if (page == MainMenu)
        {
            // page = Clock;
        }
    }

    if (leftPinFlag || rightPinFlag)
    {
        if (gpsPort.isListening())
        {
            gpsPort.end();
            Tone0(buzzerPin, 4000, 30, true);
            gpsPort.begin(BAUD_GPS);
        }
        else
            Tone0(buzzerPin, 4000, 30, false);
    }

#ifndef SKIP

    static bool syncFlag = false;

    //*** SYNC INTERVAL
    if (!syncFlag && localTime % 300 == 0)
    {
        syncFlag = true;
    }

    if (syncFlag ||
        page == Speedometer ||
        page == BaseLocation)
        gpsPortEnabled = true;
    else
        gpsPortEnabled = false;

    if (gpsPortEnabled)
    {
        if (!gpsPort.isListening())
        {
            gpsPort.begin(BAUD_GPS);
            DEBUGLOG("\r\ngpsPort turned On, baud: %lu\r\n", BAUD_GPS);
        }
    }
    else
    {
        // if (!parseGPScompleted)
        //     return;

        if (gpsPort.isListening())
        {
            gpsPort.end();
            gpsPort.flush();
            DEBUGLOG("\r\ngpsPort turned Off");
        }
    }

    parseGPScompleted = false;
    GPSloop();
    if (parseGPScompleted)
        DEBUGLOG("\r\nparsed %lu", millis());

    if (PPS)
    {
        PPS = false;
        // gpsPort.end();
        // Tone0(buzzerPin, 4000, 10, true);
        // gpsPort.begin(BAUD_GPS);
        // Tone1(buzzerPin, 512);
        DEBUGLOG("\r\nPPS\t%lu", millis());
    }

#endif

#ifndef SKIP
    if (parseGPScompleted)
    {
        if (fix.valid.location)
        {
            distanceToBaseKm = haversine(fix.latitude(), fix.longitude(), configLocation.latitude, configLocation.longitude) / 1000UL;

            // if ((int)distanceToBaseKm >= 10)
            // {
            //   configLocation.latitude = fix.latitude();
            //   configLocation.longitude = fix.longitude();

            //   updateEEPROM();

            //   process_sholat();
            // }
        }
    }

    unsigned long currentMillis = millis();
    static unsigned long prevMillis = 0;

    static uint32_t utcTime_old = 0;

    if (parseGPScompleted && fix.valid.time && fix.valid.date)
    {
        utcTime = fix.dateTime;
        prevMillis = currentMillis;

        if (tickerTick1000ms.active())
        {
            tickerTick1000ms.detach();
        }

        if (syncFlag)
        {
            syncFlag = false;

            lastSync = utcTime;

            DEBUGLOG("\r\nSync! %d:%02d:%02d", h, m, s);
        }
    }
    else if (currentMillis - prevMillis >= 1000)
    {
        // utcTime++;
        prevMillis = currentMillis;
        if (!tickerTick1000ms.active())
        {
            utcTime++;
            tickerTick1000ms.attach_ms(1000, ticker1000ms);
        }
    }

    if (utcTime != utcTime_old)
    {
        utcTime_old = utcTime;
        // prevMillis = currentMillis;

        ProcessTimestamp(utcTime);

        tick1000ms = true;
    }

    if (tick1000ms)
    {
    }
#endif

    if (tick1000ms)
    {
        state500ms = false;
        tickerFlip500ms.detach();
        tickerFlip500ms.attach_ms(500, flip500ms);
    }

    static bool state500ms_old = false;
    if (state500ms != state500ms_old)
    {
        state500ms_old = state500ms;
        tick500ms = true;
    }

    digitalWrite(LED_1, HIGH);

    // process praytime
    static uint8_t monthDay_old = 254;
    uint8_t monthDay = dtLocal.Day();
    if (monthDay != monthDay_old)
    {
        monthDay_old = monthDay;
        processSholatFlag = true;

        // tickerFlip500ms.attach_ms(250, flip500ms);
    }

    if (processSholatFlag)
    {
        processSholatFlag = false;
        process_sholat();
        process_sholat_2nd_stage();
    }

    if (tick1000ms)
    {
        process_sholat_2nd_stage();
    }

    static uint8_t page_old = PageCount;
    static unsigned long timerPageChanged = 0;
    // static bool pageChangeFlag = false;
    if (page != page_old)
    {
        page_old = page;

        mode = pageMode;

        timerPageChanged = millis();
        pageChangeFlag = true;

        DEBUGLOG("page: %d\r\n", page);

        u8g2.clearBuffer();

        redrawPage = true;
    }

    if (pageChangeFlag && millis() - timerPageChanged >= 1000)
    {
        pageChangeFlag = false;

        if (!gpsPort.isListening())
        {
            gpsPort.begin(BAUD_GPS);
            DEBUGLOG("gpsPort turned On, baud: %lu\r\n", BAUD_GPS);
        }

        if (page == Speedometer)
        {
            DEBUGLOG("send gps config, page:%d\r\n", page);

            sendUBX(ubxEnableVTG, sizeof(ubxEnableVTG));
            sendUBX(ubxDisableRMC, sizeof(ubxDisableRMC));
            sendUBX(ubxDisableGGA, sizeof(ubxDisableGGA));
            sendUBX(ubxDisableTimTP, sizeof(ubxDisableTimTP));
            LastSentenceInInterval = NMEAGPS::NMEA_VTG;
            sendUBX(ubxRate10Hz, sizeof(ubxRate10Hz));
        }
        // else if (page == Clock)
        // {
        //     sendUBX(ubxRate1Hz, sizeof(ubxRate1Hz));
        //     sendUBX(ubxDisableRMC, sizeof(ubxDisableRMC));
        //     sendUBX(ubxDisableGGA, sizeof(ubxDisableGGA));
        //     LastSentenceInInterval = NMEAGPS::NMEA_RMC;
        // }
        else
        {
            DEBUGLOG("send gps config, page:%d\r\n", page);

            sendUBX(ubxRate1Hz, sizeof(ubxRate1Hz));

            sendUBX(ubxDisableVTG, sizeof(ubxDisableVTG));
            // sendUBX(ubxEnableTimTP, sizeof(ubxEnableTimTP));
            sendUBX(ubxEnableRMC, sizeof(ubxEnableRMC));

            // sendUBX(ubxEnableGGA, sizeof(ubxEnableGGA));
            sendUBX(ubxDisableGGA, sizeof(ubxDisableGGA));

            LastSentenceInInterval = NMEAGPS::NMEA_RMC;
            // LastSentenceInInterval = NMEAGPS::NMEA_GGA;
        }
    }

    if (page == Clock)
    {
        if (processSholatFlag)
            redrawPage = true;

        static uint8_t m_old = 254;
        if (m_old != minLocal)
        {
            m_old = minLocal;
            redrawPage = true;
        }

        if (redrawPage)
        {
            u8g2.clearBuffer();
            ConstructClockPage();
        }

        const uint8_t *font;
        char buf[36];

        //**** CONTENT ****//

        uint8_t s_next;
        s_next = secLocal + 1;
        if (s_next == 60)
            s_next = 0;

        char bufSec[3];
        snprintf_P(bufSec, sizeof(bufSec), PSTR("%02d"), s_next);

        snprintf_P(buf, sizeof(buf), PSTR("%c"), bufSec[1]);

        font = u8g2_font_inb19_mn;
        // font = u8g2_font_profont29_mf;
        // font = u8g2_font_freedoomr25_mn;
        // font = u8g2_font_inb21_mn;
        font = bigNumbers16x24;

        u8g2.setFont(font);

        int y_start = 15;

        int x;

        x = (u8g2.getMaxCharWidth() * 6) - 1;
        // x = u8g2.getStrWidth(PSTR("00:00:")) - 1;

        static int y_next = y_start - u8g2.getMaxCharHeight();
        static int y_now = y_start;
        static int counter = 0;
        counter++;

        // good combination:
        // step 1; counter 1;
        // step 2; counter 12;
        // step 3; counter 16;

        // static bool scroll = false;
        // if (counter == 20)
        // {
        //     scroll = true;
        //     y_next = y_start - u8g2.getMaxCharHeight();
        //     y_now = y_start;
        // }

        static bool scroll = false;
        static unsigned long prevMillis = 0;

        if (tick1000ms)
        {
            prevMillis = millis();
        }

        if (scroll == false && millis() - prevMillis >= 725)
        {
            scroll = true;
            y_next = y_start - u8g2.getMaxCharHeight();
            y_now = y_start;
        }

        if (scroll)
        {
            int step = 3;

            y_next = y_next + step;
            if (y_next >= y_start)
                y_next = y_start;

            y_now = y_now + step;
            if (y_now >= y_start + u8g2.getMaxCharHeight())
                y_now = y_start + u8g2.getMaxCharHeight();
        }

        if (tick1000ms)
        {
            counter = 0;
            scroll = false;
            y_next = y_start - u8g2.getMaxCharHeight();
            y_now = y_start;
        }

        u8g2.setFontPosTop();

        u8g2.setCursor(x + u8g2.getMaxCharWidth(), y_next);
        u8g2.print(buf);

        snprintf_P(bufSec, sizeof(bufSec), PSTR("%02d"), secLocal);
        snprintf_P(buf, sizeof(buf), PSTR("%c"), bufSec[1]);

        u8g2.setCursor(x + u8g2.getMaxCharWidth(), y_now);
        u8g2.print(buf);

        int y0 = y_next;

        if (bufSec[1] == '9')
        {
            y0 = y_next;
        }

        snprintf_P(bufSec, sizeof(bufSec), PSTR("%02d"), s_next);
        snprintf_P(buf, sizeof(buf), PSTR("%c"), bufSec[0]);

        u8g2.setCursor(x, y0);
        u8g2.print(buf);

        y0 = 15;
        snprintf_P(bufSec, sizeof(bufSec), PSTR("%02d"), secLocal);
        snprintf_P(buf, sizeof(buf), PSTR("%c"), bufSec[0]);
        if (bufSec[1] == '9')
        {
            y0 = y_now;
        }
        u8g2.setCursor(x, y0);
        u8g2.print(buf);

        u8g2.setDrawColor(0);
        u8g2.drawBox(x, y_start - u8g2.getMaxCharHeight(), 2 * u8g2.getMaxCharWidth(), u8g2.getMaxCharHeight());
        u8g2.drawBox(x, y_start + u8g2.getMaxCharHeight() + u8g2.getFontDescent(), 2 * u8g2.getMaxCharWidth(), u8g2.getMaxCharHeight());
        u8g2.setDrawColor(1);

        // u8g2.drawFrame(x, 0, u8g2.getMaxCharWidth(), 15 + u8g2.getMaxCharHeight() - u8g2.getFontAscent() + u8g2.getFontDescent());
        // u8g2.drawFrame(x, 15 + u8g2.getMaxCharHeight() + u8g2.getFontDescent(), u8g2.getMaxCharWidth(), u8g2.getMaxCharHeight());

        //***** HEADER
        static bool state500ms_old = false;
        if (state500ms_old != state500ms)
        {
            state500ms_old = state500ms;

            // ConstructClockPage();
        }

        font = u8g2_font_chroma48medium8_8r;
        // font = u8g2_font_roentgen_nbp_tf;

        u8g2.setFont(font);

        char mon[4];
        strcpy(mon, monthShortStr(monthLocal));

        /*            
            snprintf_P(buf, sizeof(buf), PSTR("%s,%d-%s SAT%2d"),
                    //    dayShortStr(dtLocal.DayOfWeek()),
                       dtLocal.Day(),
                       mon,
                       fix.satellites);
                       */

        snprintf_P(buf, sizeof(buf), PSTR("%s %d %s %d"),
                   dayShortStr(wdayLocal),
                   mdayLocal,
                   mon,
                   yearLocal);

        uint16_t len = u8g2.getStrWidth(buf);
        x = (u8g2.getDisplayWidth() - len) / 2;
        y0 = 0;

        u8g2.setCursor(x, y0);
        u8g2.setFontMode(0);

        // u8g2.setDrawColor(state500ms);
        u8g2.print(buf);

        u8g2.setDrawColor(1);

        //**** FOOTER
        font = u8g2_font_artossans8_8r;
        font = u8g2_font_chroma48medium8_8r;
        u8g2.setFont(font);

        static int start_x = u8g2.getDisplayWidth(); //128

        static uint16_t count = 0;
        count++;
        if (count >= 0) // default: 600
        {
            // x--;
            count = 0;
            if (!ceilHOUR)
            {
                snprintf_P(buf, sizeof(buf), PSTR("%d menit"), ceilMINUTE);
            }
            else
            {
                snprintf_P(buf, sizeof(buf), PSTR("%d jam %d menit"), ceilHOUR, ceilMINUTE);
            }

            int len = u8g2.getStrWidth(buf);
            len = (u8g2.getDisplayWidth() - len) / 2;
            u8g2.setCursor(start_x + len, 43);
            u8g2.print(buf);

            snprintf_P(buf, sizeof(buf), PSTR("menuju %s"), sholatNameStr(NEXTTIMEID));
            len = u8g2.getStrWidth(buf);
            len = (u8g2.getDisplayWidth() - len) / 2;

            bool scroll = false;

            if (!scroll)
                start_x = 0;

            u8g2.setCursor(start_x + len, 55);

            u8g2.print(buf);

            start_x--;
            if (start_x <= -1 * u8g2.getDisplayWidth())
                start_x = u8g2.getDisplayWidth();

            // u8g2.sendBuffer();

            redrawPage = true;
        }
    }

    else if (page == Speedometer)
    {
        if (parseGPScompleted)
        {
            ConstructSpeedometerPage();
        }

        if (redrawPage)
        {
            ConstructSpeedometerPage();
        }
        else if (switchPinToggled)
        {
            page = MainMenu;
        }
    }

    else if (page == Compass)
    {
        MAGNETOMETERloop();

        static int headingDeg_old = 0;
        if (headingDeg_old != headingDegInt)
        {
            headingDeg_old = headingDegInt;
            ConstructCompassPage();
            redrawPage = true;
        }

        if (redrawPage)
        {
            ConstructCompassPage();
        }

        if (switchPinToggled)
        {
            page = MainMenu;
        }
    }

    else if (page == SholatTime)
    {
        if (redrawPage)
        {
            ConstructSholatTimePage();
        }
        else if (switchPinToggled)
        {
            page = MainMenu;
        }
    }

    else if (page == BaseLocation)
    {
        if (parseGPScompleted)
        {
            redrawPage = true;
        }

        if (redrawPage)
        {
            ConstructBaseLocationPage();
        }

        if (switchPinToggled)
        {
            page = MainMenu;
        }
    }
    // else if (page == Timezone)
    // {
    //     if (redrawPage)
    //     {
    //         ConstructTimezonePage();
    //     }
    //     else if (leftPinFlag || rightPinFlag)
    //     {
    //         if (rightPinFlag)
    //         {
    //             configLocation.timezone++;
    //             if (configLocation.timezone > 14)
    //                 configLocation.timezone = 14;
    //         }

    //         if (leftPinFlag)
    //         {
    //             configLocation.timezone--;
    //             if (configLocation.timezone < -12)
    //                 configLocation.timezone = -12;
    //         }

    //         int len;

    //         char temp[7];
    //         // dtostrf(configLocation.timezone, 0, 0, temp);
    //         // len = strlen(temp);

    //         const uint8_t *font;
    //         font = u8g2_font_inb19_mn;
    //         font = bigNumbers21x26;
    //         u8g2.setFont(font);
    //         len = u8g2.getMaxCharWidth();
    //         len = 3 * len; // max 3 chars
    //         uint8_t x = (128 - len) / 2;
    //         u8g2.setCursor(x, 23);
    //         u8g2.print("   ");

    //         dtostrf(configLocation.timezone, 0, 0, temp);
    //         len = u8g2.getStrWidth(temp);
    //         x = (128 - len) / 2;
    //         u8g2.setCursor(x, 23);
    //         u8g2.print(temp);

    //         redrawPage = true;
    //     }
    //     else if (switchPinToggled)
    //     {
    //         page = Settings;
    //     }
    // }
    else if (page == TimezoneDB)
    {
        if (redrawPage)
        {
            // create_timezone_start_position("/zones.csv");
            ConstructTimezoneDBPage();
        }
        else if (tick500ms || rightPinFlag || leftPinFlag || switchPinToggled)
        {
            u8g2.clearBuffer();

            ConstructTimezoneDBPage();

            redrawPage = true;
        }
    }
    else if (page == Uptime)
    {
        if (tick1000ms)
        {
            ConstructUptimePage();
        }
    }
    else if (page == Alarm)
    {
        static bool state500ms_old = false;
        if (state500ms_old != state500ms)
        {
            state500ms_old = state500ms;
            ConstructAlarmPage();
            redrawPage = true;
        }
    }
    else if (page == SystemInfo)
    {
        static int16_t selected = 0;

        if (redrawPage)
        {
            ConstructSystemInfoPage(3);
        }

        if (mode == editMode)
        {
            if (leftPinFlag)
            {
                selected--;

                if (selected < 0)
                    selected = 10;

                u8g2.clearBuffer();
                ConstructSystemInfoPage(0);
                redrawPage = true;
            }
            else if (rightPinFlag)
            {
                selected++;

                if (selected > 10)
                    selected = 0;

                u8g2.clearBuffer();
                ConstructSystemInfoPage(1);
                redrawPage = true;
            }
        }
    }
    else if (page == MainMenu)
    {
        static int8_t pos = 0;

        if (redrawPage)
        {
            // switchPinToggled = false;
            ConstructMainMenuPage(pos);
        }
        else if (leftPinFlag)
        {
            pos--;

            if (pos < 0)
                pos = 0;

            u8g2.clearBuffer();
            ConstructMainMenuPage(pos);
            redrawPage = true;
        }
        else if (rightPinFlag)
        {
            pos++;

            if (pos >= MainMenuCount)
                pos = MainMenuCount - 1;

            u8g2.clearBuffer();
            ConstructMainMenuPage(pos);
            redrawPage = true;
        }
        else if (switchPinToggled)
        {
            if (pos == EntrySpeedometer)
            {
                page = Speedometer;
            }
            if (pos == EntryCompass)
            {
                page = Compass;
            }
            if (pos == EntryBaseLocation)
            {
                page = BaseLocation;
            }
            if (pos == EntrySholatTime)
            {
                page = SholatTime;
            }
            if (pos == EntrySettings)
            {
                page = Settings;
            }
            if (pos == MainMenuCount - 1) // Exit menu
            {
                page = Clock;
            }
        }
    }
    else if (page == Settings)
    {
        static int8_t pos = 0;

        if (redrawPage)
        {
            // switchPinToggled = false;
            ConstructSettingsPage(pos);
        }
        else if (leftPinFlag)
        {
            pos--;

            if (pos < 0)
                pos = 0;

            u8g2.clearBuffer();
            ConstructSettingsPage(pos);
            redrawPage = true;
        }
        else if (rightPinFlag)
        {
            pos++;

            if (pos >= SettingsMenuCount)
                pos = SettingsMenuCount - 1;

            u8g2.clearBuffer();
            ConstructSettingsPage(pos);
            redrawPage = true;
        }
        else if (switchPinToggled)
        {
            if (pos == EntrySetDateTime)
            {
                page = DateTime;
            }
            else if (pos == EntryTimezone)
            {
                page = Timezone;
            }
            else if (pos == EntryTimezoneDB)
            {
                page = TimezoneDB;
            }
            else if (pos == EntrySetGPS)
            {
                page = SetGPS;
            }
            else if (pos == SettingsMenuCount - 1) // Exit menu
            {
                page = MainMenu;
            }
        }
    }
    else if (page == DateTime)
    {
        static int16_t pos = 0;
        static uint32_t moment = utcTime;

        int8_t maxPos = SetDateTimeCount;

        if (redrawPage)
        {
            pos = 0;
            // mode = editMode;
            moment = utcTime;
            ConstructDateTimePage(pos, moment);
        }
        else if (tick500ms)
        {
            u8g2.clearBuffer();
            ConstructDateTimePage(pos, moment);
            redrawPage = true;
        }
        else if (rightPinFlag)
        {
            if (mode == pageMode)
            {
                pos++;

                if (pos >= maxPos)
                    pos = maxPos - 1;
            }

            u8g2.clearBuffer();
            ConstructDateTimePage(pos, moment);
            redrawPage = true;
        }
        else if (leftPinFlag)
        {
            if (mode == pageMode)
            {
                pos--;

                if (pos < 0)
                    pos = 0;
            }

            u8g2.clearBuffer();
            ConstructDateTimePage(pos, moment);
            redrawPage = true;
        }
        else if (switchPinToggled)
        {
            if (pos == SaveDateTime)
            {
                ConstructDateTimePage(pos, moment);
                processSholatFlag = true;
            }
            else if (pos == maxPos - 1)
            {
                page = Settings;
            }
            else
            {
                if (mode == pageMode)
                    mode = editMode;
                else
                    mode = pageMode;
            }
        }
    }
    else if (page == SetGPS)
    {
        static int8_t pos = 0;

        if (redrawPage)
        {
            // switchPinToggled = false;
            ConstructSetGPSPage(pos);
        }
        else if (rightPinFlag)
        {
            pos++;

            if (pos >= SetGPSCount)
                pos = SetGPSCount - 1;

            u8g2.clearBuffer();
            ConstructSetGPSPage(pos);
            redrawPage = true;
        }
        else if (leftPinFlag)
        {
            pos--;

            if (pos < 0)
                pos = 0;

            u8g2.clearBuffer();
            ConstructSetGPSPage(pos);
            redrawPage = true;
        }
        else if (switchPinToggled)
        {
            if (pos == SetGPSCount - 1)
            {
                page = Settings;
            }
        }
    }

    if (page == Speedometer)
    {
        if (parseGPScompleted)
        {
            redrawPage = true;
        }
    }
    if (page == Clock || page == DateTime)
    {
        if (tick1000ms)
        {
            redrawPage = true;
        }
    }

    if (redrawPage)
    {
        // DEBUGLOG("\r\nRedraw PAGE");
        // redrawPage = false;
        u8g2.sendBuffer();
    }

    static uint8_t mode_old = 254;
    if (mode != mode_old)
    {
        mode_old = mode;
        DEBUGLOG("MODE: %d\r\n", mode);
    }

    // buzzer
    if (tick1000ms)
    {
        static bool alarmPreSholat = false;
        if (utcTime == (nextSholatTime - 10 * 60))
        {
            alarmPreSholat = true;
        }

        if (utcTime == (nextSholatTime - 5 * 60))
        {
            alarmPreSholat = true;
        }

        if (alarmPreSholat)
        {
            static uint8_t count = 0;

            count++;
            if (count == 3)
            {
                count = 0;
                alarmPreSholat = false;
            }

            buzzer(buzzerPin, 500);
        }

        static bool alarmSholat = false;
        if (utcTime == nextSholatTime)
        {
            alarmSholat = true;
        }

        if (alarmSholat)
        {
            static uint8_t count = 0;
            int dur = 500;

            count++;
            if (count == 5)
            {
                dur = 2000;
                count = 0;
                alarmSholat = false;
            }

            Tone0(buzzerPin, 2375, dur, false);
        }
    }

    if (leftPinFlag)
    {
        leftPinFlag = false;
    }

    if (rightPinFlag)
    {
        rightPinFlag = false;
    }

    if (switchPinToggled)
    {
        switchPinToggled = false;
    }

    // #endif
}
