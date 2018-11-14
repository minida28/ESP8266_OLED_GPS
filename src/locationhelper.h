#ifndef locationhelper_h
#define locationhelper_h

#include <Arduino.h>

typedef struct
{
//   char city[48] = "KOTA BEKASI";
  float timezone = 7.0;
  double latitude = -6.2653246;
  double longitude = 106.972939;
} strConfigLocation;
extern strConfigLocation configLocation;

#endif