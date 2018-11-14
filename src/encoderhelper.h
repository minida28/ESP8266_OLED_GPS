#ifndef encoderhelper_h
#define encoderhelper_h

#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
#include "Button2.h"
#include "pinouthelper.h"

extern Encoder myEnc;

extern bool leftPinFlag;
extern bool rightPinFlag;
extern bool switchPinToggled;

void ENCODERsetup();
void ENCODERloop();



#endif