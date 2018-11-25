#ifndef timezonehelper_h
#define timezonehelper_h

#include "Espgoodies.h"
#include <ESP8266WiFi.h>
#include <FS.h>


uint16_t create_timezone_start_position(const char *fileName);
uint8_t get_zones_info(char *bufZonesName, char *bufZonesString);



#endif // timezonehelper_h