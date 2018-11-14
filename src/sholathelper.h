#ifndef sholathelper_h
#define sholathelper_h

#include <Arduino.h>
#include "sholat.h"
#include "pgmstring.h"
#include "locationhelper.h"
#include "timehelper.h"
// #include "progmemmatrix.h"
// #include <time.h>


extern PrayerTimes sholat;

// typedef struct
// {
//   char city[48] = "KOTA BEKASI";
//   int8_t timezone = 70;
//   double latitude = -6.263718;
//   double longitude = 106.981958;
// } strConfigLocation;
// extern strConfigLocation _configLocation;

extern uint8_t HOUR;
extern uint8_t MINUTE;
extern uint8_t SECOND;
extern uint8_t ceilHOUR;
extern uint8_t ceilMINUTE;
extern uint8_t CURRENTTIMEID, NEXTTIMEID;

extern unsigned long nextSholatTime;

extern unsigned long timestampSholatTimesToday[TimesCount];

// extern char bufHOUR[3];
// extern char bufMINUTE[3];
// extern char bufSECOND[3];

// extern char sholatTimeYesterdayArray[TimesCount][6];
// extern char sholatTimeArray[TimesCount][6];
// extern char sholatTimeTomorrowArray[TimesCount][6];



char *sholatNameStr(uint8_t id);
void process_sholat();
void process_sholat_2nd_stage();
void ProcessSholatEverySecond();

void SholatLoop();

#endif