#include <Arduino.h>
#include <time.h>
#include "timehelper.h"
// #include "sntphelper.h"
// #include "rtchelper.h"

#define DEBUGPORT Serial

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

bool tick1000ms = false;
bool state500ms = false;
bool tick500ms = false;

bool NTP_OK = false;
unsigned long utcTime = 0;
unsigned long localTime = 0;

RtcDateTime dtUtc;
RtcDateTime dtLocal;
unsigned long _lastSyncd = 0; ///< Stored time of last successful sync
unsigned long _firstSync = 0; ///< Stored time of first successful sync after boot
unsigned long lastBoot = 0;
unsigned long lastSync = 0;

uint16_t syncInterval = 0;      ///< Interval to set periodic time sync
uint16_t shortSyncInterval = 0; ///< Interval to set periodic time sync until first synchronization.
uint16_t longSyncInterval = 0;  ///< Interval to set periodic time sync

uint8_t h, m, s;

// strConfigTime configTime;
// TIMESOURCE _timeSource;

void ProcessTimestamp(unsigned long _utc)
{
  dtUtc = RtcDateTime(_utc);
  localTime = _utc + TimezoneSeconds();
  dtLocal = RtcDateTime(localTime);

  s = dtLocal.Second();
  h = dtLocal.Hour();
  m = dtLocal.Minute();
}

long TimezoneMinutes()
{
    return configLocation.timezone * 60;
}

long TimezoneSeconds()
{
    return TimezoneMinutes() * 60;
}

char *getDateStr(time_t rawtime) // Thu Aug 23 2001
{
    DEBUGLOG("%s\r\n", __PRETTY_FUNCTION__);
    static char buf[16];

    RtcDateTime dt;
    dt = RtcDateTime(rawtime);
    snprintf_P(buf, sizeof(buf), PSTR("%02d %02d %02d %d"), dt.DayOfWeek(), dt.Month(), dt.Day(), dt.Year());

    return buf;
}

char *getTimeStr(time_t rawtime)
{
    DEBUGLOG("%s\r\n", __PRETTY_FUNCTION__);
    static char buf[12];

    RtcDateTime dt;
    dt = RtcDateTime(rawtime);
    snprintf_P(buf, sizeof(buf), PSTR("%02d:%02d:%02d"), dt.Hour(), dt.Minute(), dt.Second()); //02:55:02

    return buf;
}

char *getUptimeStr()
{
    DEBUGLOG("%s\r\n", __PRETTY_FUNCTION__);
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
    DEBUGLOG("%s\r\n", __PRETTY_FUNCTION__);

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
    DEBUGLOG("%s\r\n", __PRETTY_FUNCTION__);

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

void TimeSetup()
{
}