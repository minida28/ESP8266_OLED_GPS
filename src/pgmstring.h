#ifndef _PGMSTRING_H
#define _PGMSTRING_H

const char fajrStr[] PROGMEM = "SUBUH";
const char syuruqStr[] PROGMEM = "SYURUQ";
const char dhuhrStr[] PROGMEM = "DHUHR";
const char ashrStr[] PROGMEM = "ASHAR";
const char sunsetStr[] PROGMEM = "SUNSET";
const char maghribStr[] PROGMEM = "MAGHRIB";
const char isyaStr[] PROGMEM = "ISYA";
const char sholatNameCountStr[] PROGMEM = "TIMESCOUNT";

const char *const sholatName_P[8] PROGMEM =
    {
        fajrStr,
        syuruqStr,
        dhuhrStr,
        ashrStr,
        sunsetStr,
        maghribStr,
        isyaStr,

        sholatNameCountStr};

const char pgm_invalid[] PROGMEM = "invalid";

#endif