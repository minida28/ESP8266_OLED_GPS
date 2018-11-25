#include <Arduino.h>
#include "progmemmatrix.h"
#include "locationhelper.h"
#include "spiffshelper.h"
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include <ArduinoJson.h>

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

//*************************
// SAVE CONFIG LOCATION
//*************************
bool save_config_location()
{
  DEBUGLOG("%s\r\n", __PRETTY_FUNCTION__);

  StaticJsonBuffer<512> jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();

//   json[FPSTR(pgm_province)] = configLocation.province;
//   json[FPSTR(pgm_regency)] = configLocation.regency;
//   json[FPSTR(pgm_district)] = configLocation.district;
  json[FPSTR(pgm_timezonestring)] = configLocation.timezonestring;
  json[FPSTR(pgm_latitude)] = configLocation.latitude;
  json[FPSTR(pgm_longitude)] = configLocation.longitude;

  File file = SPIFFS.open(FPSTR(pgm_configfilelocation), "w");

#ifndef RELEASEASYNCWS
  json.prettyPrintTo(DEBUGPORT);
  DEBUGLOG("\r\n");
#endif

  json.prettyPrintTo(file);
  file.flush();
  file.close();
  return true;
}