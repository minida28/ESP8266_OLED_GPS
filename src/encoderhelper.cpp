#include "encoderhelper.h"

Encoder myEnc(ENC_CLK, ENC_DT);

Button2 b = Button2(ENC_SW);
bool leftPinFlag;
bool rightPinFlag;
bool switchPinToggled;

void showPosition(Button2 &btn)
{
    //   Serial.println(r.getPosition());
    switchPinToggled = true;
}

void resetPosition(Button2 &btn)
{
    //   r.resetPosition();
    //   Serial.println("Reset!");
    //   Serial.println(r.getPosition());
}

void ENCODERsetup()
{
    //   Serial.begin(9600);
    //   delay(50);
    // Serial.println("\n\nSimple Counter");

    // r.setChangedHandler(rotate);
    // r.setLeftRotationHandler(showDirection);
    // r.setRightRotationHandler(showDirection);

    // b.setClickHandler(showPosition);
    // b.setPressedHandler(showPosition);
    b.setReleasedHandler(showPosition);

    //     b.setLongClickHandler(resetPosition);
    //       buttonA.setLongClickHandler(handler);
    //   buttonA.setDoubleClickHandler(handler);
    //   buttonA.setTripleClickHandler(handler);
}

void ENCODERloop()
{
    b.loop();

    long encPos = myEnc.read();
    static long encPos_old = encPos;

    if (encPos != encPos_old)
    {
        if (encPos % 4 == 0)
        {
            if (encPos > encPos_old)
            {
                rightPinFlag = true;
            }
            else
            {
                leftPinFlag = true;
            }
            long encPos = myEnc.read();
            static long encPos_old = encPos;

            if (encPos != encPos_old)
            {
                if (encPos % 4 == 0)
                {
                    if (encPos > encPos_old)
                    {
                        rightPinFlag = true;
                    }
                    else
                    {
                        leftPinFlag = true;
                    }
                }

                encPos_old = encPos;
            }
        }

        encPos_old = encPos;
    }
}