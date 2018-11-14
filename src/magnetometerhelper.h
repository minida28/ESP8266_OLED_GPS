#ifndef magnetometerhelper_h
#define magnetometerhelper_h

#include <Arduino.h>
#include <Wire.h>
#include <HMC5883L.h>

extern HMC5883L compass;

extern int headingDegInt;

void MAGNETOMETERsetup();
void MAGNETOMETERloop();

#endif