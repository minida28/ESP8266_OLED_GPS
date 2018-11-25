#include <Arduino.h>
#include <time.h>
#include "timehelper.h"
// #include "sntphelper.h"
// #include "rtchelper.h"
// #include "Espgoodies.h"
// #include <ESP8266WiFi.h>
#include "timezonehelper.h"

#define DEBUGPORT Serial

#define PRINT(fmt, ...)                          \
    {                                            \
        static const char pfmt[] PROGMEM = fmt;  \
        DEBUGPORT.printf_P(pfmt, ##__VA_ARGS__); \
    }

#define RELEASE

#ifndef RELEASE

#define DEBUGLOG(fmt, ...)                      \
    {                                           \
        static const char pfmt[] PROGMEM = fmt; \
        printf_P(pfmt, ##__VA_ARGS__);          \
    }
#else
#define DEBUGLOG(...)
#endif

#define PTM(w)                \
    Serial.print(":" #w "="); \
    Serial.print(tm->tm_##w);

void printTm(const char *what, const tm *tm)
{
    Serial.print(what);
    PTM(isdst);
    PTM(yday);
    PTM(wday);
    PTM(year);
    PTM(mon);
    PTM(mday);
    PTM(hour);
    PTM(min);
    PTM(sec);
}

bool state1000ms = false;
bool tick1000ms = false;
bool state500ms = false;
bool tick500ms = false;
bool tick3000ms = false;

uint32_t utcTime_ms, utcTime_us;
timeval tv;
timespec tp;
timeval cbtime; // time set in callback
bool timeSetFlag;
bool y2k38mode = false;

bool NTP_OK = false;
unsigned long utcTime = 0;
unsigned long localTime = 0;
// uint32_t espTime = 0;

RtcDateTime dtUtc;
RtcDateTime dtLocal;
unsigned long _lastSyncd = 0; ///< Stored time of last successful sync
unsigned long _firstSync = 0; ///< Stored time of first successful sync after boot
unsigned long lastBoot = 0;
unsigned long lastSync = 0;
unsigned long uptime = 0;

uint16_t syncInterval = 0;      ///< Interval to set periodic time sync
uint16_t shortSyncInterval = 0; ///< Interval to set periodic time sync until first synchronization.
uint16_t longSyncInterval = 0;  ///< Interval to set periodic time sync

// uint8_t h, m, s;

uint16_t yearLocal;
uint8_t monthLocal;
uint8_t mdayLocal;
uint8_t wdayLocal;
uint8_t hourLocal;
uint8_t minLocal;
uint8_t secLocal;

// strConfigTime configTime;
// TIMESOURCE _timeSource;

void ProcessTimestamp(unsigned long _utc)
{
    bool EPOCH2000 = true;

    if (EPOCH2000)
    {
        dtUtc = RtcDateTime(_utc);
        localTime = _utc + TimezoneSeconds();
        dtLocal = RtcDateTime(localTime);
    }
    else
    {
        dtUtc.InitWithEpoch32Time(_utc);
        localTime = _utc + TimezoneSeconds();
        dtLocal.InitWithEpoch32Time(localTime);
    }

    yearLocal = dtLocal.Year();
    monthLocal = dtLocal.Month();
    mdayLocal = dtLocal.Day();
    wdayLocal = dtLocal.DayOfWeek();
    hourLocal = dtLocal.Hour();
    minLocal = dtLocal.Minute();
    secLocal = dtLocal.Second();
}

float TimezoneFloat()
{
    time_t rawtime;
    char buffer[6];

    if (!y2k38mode)
    {
        rawtime = time(nullptr);
    }
    else if (y2k38mode)
    {
        rawtime = time(nullptr) + 2145916800 + 3133696;
    }

    struct tm * timeinfo;
    timeinfo = localtime(&rawtime);

    strftime(buffer, 6, "%z", timeinfo);

    char bufTzHour[4];
    strlcpy(bufTzHour, buffer, sizeof(bufTzHour));
    int8_t hour = atoi(bufTzHour);

    char bufTzMin[4];
    bufTzMin[0] = buffer[0]; // sign
    bufTzMin[1] = buffer[3];
    bufTzMin[2] = buffer[4];
    bufTzMin[3] = 0;
    float min = atoi(bufTzMin) / 60.0;

    float TZ_FLOAT = hour + min;
    return TZ_FLOAT;
}

long TimezoneMinutes()
{
    return TimezoneFloat() * 60;
}

long TimezoneSeconds()
{
    return TimezoneMinutes() * 60;
}

char *getDateStr(uint32_t rawtime) // Thu Aug 23 2001
{
    // DEBUGLOG("%s\r\n", __PRETTY_FUNCTION__);
    static char buf[16];

    RtcDateTime dt;
    // dt = RtcDateTime(rawtime);
    dt.InitWithEpoch32Time(rawtime);
    snprintf_P(buf, sizeof(buf), PSTR("%02d %02d %02d %d"), dt.DayOfWeek(), dt.Month(), dt.Day(), dt.Year());

    return buf;
}

char *getTimeStr(uint32_t rawtime)
{
    // DEBUGLOG("%s\r\n", __PRETTY_FUNCTION__);
    static char buf[12];

    RtcDateTime dt;
    // dt = RtcDateTime(rawtime);
    dt.InitWithEpoch32Time(rawtime);
    snprintf_P(buf, sizeof(buf), PSTR("%02d:%02d:%02d"), dt.Hour(), dt.Minute(), dt.Second()); //02:55:02

    return buf;
}

char *getDateTimeStr(uint32_t moment)
{
    // output: Tue Jul 24 2018 16:46:44 GMT
    static char buf[29];

    RtcDateTime dt;
    dt.InitWithEpoch32Time(moment);

    char bufDay[4];
    strcpy(bufDay, dayShortStr(dt.DayOfWeek()));

    char bufMon[4];
    strcpy(bufMon, monthShortStr(dt.Month()));

    snprintf_P(
        buf,
        (sizeof(buf)),
        PSTR("%s %s %2d %02d:%02d:%02d %d"),
        bufDay,
        bufMon,
        dt.Day(),
        dt.Hour(),
        dt.Minute(),
        dt.Second(),
        dt.Year());

    return buf;
}

char *getUptimeStr()
{
    // DEBUGLOG("%s\r\n", __PRETTY_FUNCTION__);
    //time_t uptime = utcTime - _lastBoot;
    time_t uptime = millis() / 1000;

    uint16_t days;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;

    days = elapsedDays(uptime);
    hours = numberOfHours(uptime);
    minutes = numberOfMinutes(uptime);
    seconds = numberOfSeconds(uptime);

    static char buf[30];
    snprintf_P(buf, sizeof(buf), PSTR("%u day %02d:%02d:%02d"), days, hours, minutes, seconds);

    return buf;
}

char *getLastSyncStr()
{
    // DEBUGLOG("%s\r\n", __PRETTY_FUNCTION__);

    unsigned long diff = utcTime - _lastSyncd;

    uint16_t days;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;

    days = elapsedDays(diff);
    hours = numberOfHours(diff);
    minutes = numberOfMinutes(diff);
    seconds = numberOfSeconds(diff);

    static char buf[30];
    if (days > 0)
    {
        snprintf_P(buf, sizeof(buf), PSTR("%u day %d hr ago"), days, hours);
    }
    else if (hours > 0)
    {
        snprintf_P(buf, sizeof(buf), PSTR("%d hr %d min ago"), hours, minutes);
    }
    else if (minutes > 0)
    {
        snprintf_P(buf, sizeof(buf), PSTR("%d min ago"), minutes);
    }
    else
    {
        snprintf_P(buf, sizeof(buf), PSTR("%d sec ago"), seconds);
    }

    return buf;
}

char *getNextSyncStr()
{
    // DEBUGLOG("%s\r\n", __PRETTY_FUNCTION__);

    unsigned long nextsync;

    nextsync = _lastSyncd - utcTime + syncInterval;

    uint16_t days;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;

    RtcDateTime tm;
    tm = RtcDateTime(nextsync); // convert to broken down time
    days = tm.Day();
    hours = tm.Hour();
    minutes = tm.Minute();
    seconds = tm.Second();

    static char buf[30];
    snprintf_P(buf, sizeof(buf), PSTR("%d days %02d:%02d:%02d"), days, hours, minutes, seconds);
    // snprintf_P(buf, sizeof(buf), PSTR("xx days %02d:%02d:%02d"), hours, minutes, seconds);

    return buf;
}
// char *dayShortStr(uint8_t day)
// {
//     static char buf[30];
//     uint8_t index = day * dt_SHORT_STR_LEN;
//     for (int i = 0; i < dt_SHORT_STR_LEN; i++)
//         buf[i] = pgm_read_byte(&(dayShortNames_P[index + i]));
//     buf[dt_SHORT_STR_LEN] = 0;
//     return buf;
// }

void time_is_set()
{
    gettimeofday(&cbtime, NULL);
    // SetRtcTime(time(nullptr));
    timeSetFlag = true;
    y2k38mode = false;
    PRINT("\r\n\r\n------------------ settimeofday() was called ------------------\r\n");

    //   clock_gettime(0, &tp);
    // now = time(nullptr);
    DEBUGLOG("time(nullptr): %li, (uint32_t)tv.tv_sec: %u", time(nullptr), (uint32_t)tv.tv_sec)
}

void SyncTime(uint32_t epoch2000)
{
    setenv("TZ", configLocation.timezonestring, 1/*overwrite*/);   
    // setenv("TZ", "WIB-7", 1 /*overwrite*/);
    tzset();

    // configTZ(TZ_Australia_Melbourne);
    // configTZ(TZ_Asia_Jakarta);

    RtcDateTime dt = epoch2000;
    uint32_t t = dt.Epoch32Time();

    time_t _epoch = t;

    DEBUGLOG("\r\nrawtime: %u, EPOCH32: %u, _epoch: %li", epoch2000, dt.Epoch32Time(), _epoch);

    timeval tv = {_epoch, 0};
    timezone tz = {0, 0};
    settimeofday(&tv, &tz);

    // gettimeofday(&tv, nullptr);
    // //   clock_gettime(0, &tp);

    // utcTime = time(nullptr);
}

void TimeSetup()
{
    //   state250msTimer.attach(0.25, FlipState250ms);

    settimeofday_cb(time_is_set);

    // configTZ(TZ_Asia_Jakarta);
    // configTZ(_configLocation.timezonestring);
    // configTZ("WIB-7");

    setenv("TZ", configLocation.timezonestring, 1/*overwrite*/);
    // setenv("TZ", "WIB-7", 1 /*overwrite*/);
    tzset();

    // configTZ(TZ_Australia_Melbourne);
    // configTZ(TZ_Asia_Jakarta);

    // if (_configTime.enablentp)
    // {
    //     configTime(0, 0, _configTime.ntpserver_0, _configTime.ntpserver_1, _configTime.ntpserver_2);
    // }

    // if (_configTime.enablertc)
    // {
    //     uint32_t rtc = get_time_from_rtc();
    //     timeval tv = {rtc, 0};
    //     timezone tz = {0, 0};
    //     settimeofday(&tv, &tz);
    // }

    // time_t _gpsTime = gpsTime;

    // if (1)
    // {
    //     // uint32_t gpsTime = _gpsTime;
    //     timeval tv = {_gpsTime, 0};
    //     timezone tz = {0, 0};
    //     settimeofday(&tv, &tz);
    // }

    // time_t _epoch = 2147483647 - 10;

    // timeval tv = {_epoch, 0};
    // timezone tz = {0, 0};
    // settimeofday(&tv, &tz);

    // gettimeofday(&tv, nullptr);
    // //   clock_gettime(0, &tp);

    // utcTime = time(nullptr);
}

void TimeLoop()
{
    gettimeofday(&tv, nullptr);
    //   clock_gettime(0, &tp);

    utcTime = time(nullptr);

    if (utcTime < 0)
    {
        y2k38mode = true;
    }

    uptime = tp.tv_sec;
    utcTime_ms = millis();
    utcTime_us = micros();

    // localtime / gmtime every second change
    static uint32_t lastv = 0;
    if (lastv != utcTime)
    {
        lastv = utcTime;

        localTime = utcTime + TimezoneSeconds();

        RtcDateTime dt;
        dt.InitWithEpoch32Time(localTime);

        yearLocal = dt.Year();
        monthLocal = dt.Month();
        mdayLocal = dt.Day();
        wdayLocal = dt.DayOfWeek();
        hourLocal = dt.Hour();
        minLocal = dt.Minute();
        secLocal = dt.Second();

        DEBUGLOG("\r\ntv.tv_sec:%li", tv.tv_sec);

        state500ms = true;
        // state500msTimer.once(0.5, FlipState500ms);

        // tick1000ms = true;

        if (!(utcTime % 2))
        {
            // do something even
            state1000ms = true;
        }
        else
        {
            state1000ms = false;
        }

        static uint8_t counter3 = 0;
        counter3++;
        if (counter3 >= 3)
        {
            counter3 = 0;
            tick3000ms = true;
        }

#ifndef RELEASE
        time_t test;
        if (!y2k38mode)
        {
            test = time(nullptr);
        }
        else if (y2k38mode)
        {
            test = time(nullptr) + 2145916800 + 3133696;
        }

        // Serial.println();
        printTm("\r\nlocaltime", localtime(&test));
        printTm("\r\ngmtime   ", gmtime(&test));

        // time from boot
        Serial.print("\r\nclock:");
        Serial.print((uint32_t)tp.tv_sec);
        Serial.print("/");
        Serial.print((uint32_t)tp.tv_nsec);
        Serial.print("ns");

        // time from boot
        Serial.print(" millis:");
        Serial.print(utcTime_ms);
        Serial.print(" micros:");
        Serial.print(utcTime_us);

        // EPOCH+tz+dst
        Serial.print(" gtod:");
        Serial.print((uint32_t)tv.tv_sec);
        Serial.print("/");
        Serial.print((uint32_t)tv.tv_usec);
        Serial.print("us");

        // EPOCH+tz+dst
        Serial.print(" time_t:");
        Serial.print(utcTime);
        Serial.print(" time uint32_t:");
        Serial.print((uint32_t)utcTime);

        // RtcDateTime timeToSetToRTC;
        // timeToSetToRTC.InitWithEpoch32Time(now);
        // Rtc.SetDateTime(timeToSetToRTC);

        // human readable
        // Printed format: Wed Oct 05 2011 16:48:00 GMT+0200 (CEST)
        char buf[50];
        // output: Wed Jan 01 1902 07:02:04 GMT+0700 (WIB)
        // strftime(buf, sizeof(buf), "%a %b %d %Y %X GMT", gmtime(&test));
        strftime(buf, sizeof(buf), "%c GMT", gmtime(&test));
        DEBUGLOG("\r\nNTP GMT   date/time: %s", buf);
        // strftime(buf, sizeof(buf), "%a %b %d %Y %X GMT%z (%Z)", localtime(&test));
        strftime(buf, sizeof(buf), "%c GMT%z (%Z)", localtime(&test));
        DEBUGLOG("\r\nNTP LOCAL date/time: %s", buf);

        // RtcDateTime dt;
        dt.InitWithEpoch32Time(utcTime);
        DEBUGLOG("\r\nNTP GMT   date/time: %s GMT", getDateTimeStr(utcTime));
        dt.InitWithEpoch32Time(localTime);
        strftime(buf, sizeof(buf), "GMT%z (%Z)", localtime(&test));
        DEBUGLOG("\r\nNTP LOCAL date/time: %s %s", getDateTimeStr(localTime), buf);

        // dt = Rtc.GetDateTime();
        // DEBUGLOG("\r\nRTC GMT   date/time: %s GMT\r\n", GetRtcDateTimeStr(dt));
#endif
    }
}