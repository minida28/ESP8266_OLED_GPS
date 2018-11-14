#ifndef pageshelper_h
#define pageshelper_h

#include <Arduino.h>
#include "gpshelper.h"
#include "displayhelper.h"
#include "timehelper.h"
#include "sholathelper.h"
#include "magnetometerhelper.h"
#include "encoderhelper.h"

enum enumMode
{
    pageMode,
    editMode,
    // level_0,
    // level_1,
    // level_2,

    modeCount
};

enum enumPage
{
  Clock,
  Speedometer,
  Timezone,
  BaseLocation,
  SholatTime,
  Uptime,
  Compass,
  Alarm,
  SystemInfo,
  MainMenu,
  Settings,
  DateTime,
  SetGPS,
  PageCount
};

enum enumMainMenu
{
  EntrySpeedometer,
  EntryCompass,
  EntryBaseLocation,
  EntrySholatTime,
  EntrySettings,
  EntryDianostic,
  ExitMainMenu,
  MainMenuCount
};

enum enumSettingsMenu
{
  EntrySetDateTime,
  EntryTimezone,
  EntrySetGPS,
  EntryMenu3,
  ExitSettingsMenu,
  SettingsMenuCount
};

enum enumSetDateTime
{
  SetDate,
  SetMonth,
  SetYear,
  SetHour,
  SetMinute,
  SetSecond,
  SaveDateTime,
  ExitDateTime,
  SetDateTimeCount
};

enum enumSetGPS
{
  SetRMC,
  SetGGA,
  SetTimTP,
  SetVTG,
  SetRate,
  SetBaud,
  ExitSetGPS,
  SetGPSCount
};

enum enumSetTimeDate
{
  EnableRMC,
  EnableGGA,
  EnableTimTP,
  EnableVTG,
  MenuGPSSettingsCount
};

extern uint8_t mode;

void ConstructClockPage();
void ConstructSpeedometerPage();
void ConstructTimezonePage();
void ConstructBaseLocationPage();
void ConstructSholatTimePage();
void ConstructUptimePage();
void ConstructCompassPage();
void ConstructAlarmPage();
void ConstructSystemInfoPage(int16_t _pos);
void ConstructMainMenuPage(int16_t _pos);
void ConstructSettingsPage(int16_t _pos);
void ConstructSetGPSPage(int16_t _pos);
void ConstructDateTimePage(int16_t _pos, uint32_t _moment);



#endif