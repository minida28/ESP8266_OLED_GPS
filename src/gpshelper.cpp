#include "gpshelper.h"

#include <NeoGPS_cfg.h>
// #include <ublox/ubxGPS.h>
#include <NeoTeeStream.h>

//======================================================================
//  Program: ubloxRate.ino
//
//  Description:  This program sends ublox commands to enable and disable
//    NMEA sentences, set the update rate, and set the
//    baud rate to 9600 or 115200.
//
//    Enter the following commands through the Serial Monitor window:
//
const char help[] PROGMEM = R"(
  '?'  - dislay this help message
  '1'  - send NMEA PUBX text command to enable all sentences
  '0'  - send NMEA PUBX text command to disable all sentences except GLL
  'd'  - send UBX binary command to disable all sentences except GLL
  'r1' - send UBX binary command to set update rate to  1Hz
  'r5' - send UBX binary command to set update rate to  5Hz
  'r0' - send UBX binary command to set update rate to 10Hz
  'r6' - send UBX binary command to set update rate to 16Hz
  're' - send UBX binary command to reset the GPS device (cold start)
  '5'  - send NMEA PUBX text command to set baud rate to 115200
  '7'  - send NMEA PUBX text command to set baud rate to 57600
  '3'  - send NMEA PUBX text command to set baud rate to 38400
  '9'  - send NMEA PUBX text command to set baud rate to 9600
  'e'  - toggle echo of all characters received from GPS device.
  't'  - toggle tracing of parsed GPS fields.
)";
//    CAUTION:   If your Serial Monitor window baud rate is less
//       than the GPS baud rate, turning echo ON will cause the
//        sketch to lose some or all GPS data and/or fixes.
//
//    NOTE:  All NMEA PUBX text commands are also echoed to the debug port.
//
//  Prerequisites:
//     1) Your GPS device has been correctly powered.
//          Be careful when connecting 3.3V devices.
//     2) Your GPS device is correctly connected to an Arduino serial port.
//          See GPSport.h for the default connections.
//     3) You know the default baud rate of your GPS device.
//          If 9600 does not work, use NMEAdiagnostic.ino to
//          scan for the correct baud rate.
//     4) LAST_SENTENCE_IN_INTERVAL is defined to be
//          the following in NMEAGPS_cfg.h:
//
//          #include <stdint.h>
//          extern uint8_t LastSentenceInInterval; // a variable!
//          #define LAST_SENTENCE_IN_INTERVAL ((NMEAGPS::nmea_msg_t) LastSentenceInInterval)
//
//
//        This is a replacement for the typical
//
//          #define LAST_SENTENCE_IN_INTERVAL NMEAGPS::NMEA_GLL
//
//        This allows the sketch to choose the last sentence *at run time*, not
//        compile time.  This is necessary because this sketch can send
//        configuration commands that change which sentences are enabled.
//        The storage for the "externed" variable is below.
//     5) ublox.ino builds correctly (see its prequisites).
//
//  'Serial' is for debug output to the Serial Monitor window.
//
//  License:
//    Copyright (C) 2014-2018, SlashDevin
//
//    This file is part of NeoGPS
//
//    NeoGPS is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    NeoGPS is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with NeoGPS.  If not, see <http://www.gnu.org/licenses/>.
//
//======================================================================

// #include <GPSport.h>
#include <Streamers.h>

#include <SoftwareSerial.h>
#include "pinouthelper.h"

#define DEBUGPORT Serial

// #define SKIP

// #define RELEASE

#ifndef RELEASE
#define DEBUGLOG(fmt, ...)                       \
    {                                            \
        static const char pfmt[] PROGMEM = fmt;  \
        DEBUGPORT.printf_P(pfmt, ##__VA_ARGS__); \
    }
#else
#define DEBUGLOG(...)
#endif

SoftwareSerial gpsPort(SOFTSER_RX, SOFTSER_TX, false, 256); // Rx = D2 ,Tx = D1
// SoftwareSerial gpsPort(13,15, false, 256); // Rx = D7 ,Tx = D8
#define GPS_PORT_NAME "gpsPort"
#define DEBUG_PORT Serial

//------------------------------------------------------------
// Check that the config files are set up properly

#ifdef NMEAGPS_INTERRUPT_PROCESSING
#error You must *not* enable NMEAGPS_INTERRUPT_PROCESSING in NMEAGPS_cfg.h!
#endif

#ifndef NMEAGPS_DERIVED_TYPES
#error You must enable NMEAGPS_DERIVED_TYPES in NMEAGPS_cfg.h!
#endif

#if !defined(NMEAGPS_PARSE_GLL)
#error You must disable NMEAGPS_PARSE_GLL in NMEAGPS_cfg.h!
#endif

#if !defined(UBLOX_PARSE_STATUS) & !defined(UBLOX_PARSE_TIMEGPS) & \
    !defined(UBLOX_PARSE_TIMEUTC) & !defined(UBLOX_PARSE_POSLLH) & \
    !defined(UBLOX_PARSE_DOP) & !defined(UBLOX_PARSE_PVT) &        \
    !defined(UBLOX_PARSE_VELNED) & !defined(UBLOX_PARSE_SVINFO) &  \
    !defined(UBLOX_PARSE_HNR_PVT)

#error No UBX binary messages enabled: no fix data available.

#endif

//-----------------------------------------------------------------

ubloxGPS gps(&gpsPort);
// NMEAGPS gps;
gps_fix fix;
uint8_t LastSentenceInInterval = 0xFF; // storage for the run-time selection
unsigned long BAUD_GPS = 9600UL;

static char lastChar; // last command char
// static bool echoing = true;
static bool tracing = false;
uint8_t ubloxClassID = 0;
bool PPS = false;

bool parseGPScompleted;
double distanceToBaseKm;

//  Use NeoTee to echo the NMEA text commands to the Serial Monitor window
Stream *both[2] = {&DEBUG_PORT, &gpsPort};
NeoTeeStream tee(both, sizeof(both) / sizeof(both[0]));

//-------------------------------------------

static const uint8_t ubxReset[] __PROGMEM =
    {
        // ublox::UBX_CFG, ublox::UBX_CFG_RST,
        // UBX_MSG_LEN(ublox::cfg_reset_t), 0,               // word length MSB is 0
        // 0, 0,                                             // clear bbr section
        // ublox::cfg_reset_t::CONTROLLED_SW_RESET_GPS_ONLY, // reset mode
        // 0x00                                              // reserved
};

//--------------------------

void sendUBX(const unsigned char *progmemBytes, size_t len)
{
    gpsPort.write(0xB5); // SYNC1
    gpsPort.write(0x62); // SYNC2

    uint8_t a = 0, b = 0;
    while (len-- > 0)
    {
        uint8_t c = pgm_read_byte(progmemBytes++);
        a += c;
        b += a;
        gpsPort.write(c);

        // DEBUG_PORT.print(c, HEX);
        // DEBUG_PORT.print(" ");
    }

    // DEBUG_PORT.println();

    gpsPort.write(a); // CHECKSUM A
    gpsPort.write(b); // CHECKSUM B

} // sendUBX

//-------------------------------------------
// U-blox NMEA text commands

const char disableRMC[] PROGMEM = "PUBX,40,RMC,0,0,0,0,0,0";
const char disableGLL[] PROGMEM = "PUBX,40,GLL,0,0,0,0,0,0";
const char disableGSV[] PROGMEM = "PUBX,40,GSV,0,0,0,0,0,0";
const char disableGSA[] PROGMEM = "PUBX,40,GSA,0,0,0,0,0,0";
const char disableGGA[] PROGMEM = "PUBX,40,GGA,0,0,0,0,0,0";
const char disableVTG[] PROGMEM = "PUBX,40,VTG,0,0,0,0,0,0";
const char disableZDA[] PROGMEM = "PUBX,40,ZDA,0,0,0,0,0,0";

const char enableRMC[] PROGMEM = "PUBX,40,RMC,0,1,0,0,0,0";
const char enableGLL[] PROGMEM = "PUBX,40,GLL,0,1,0,0,0,0";
const char enableGSV[] PROGMEM = "PUBX,40,GSV,0,1,0,0,0,0";
const char enableGSA[] PROGMEM = "PUBX,40,GSA,0,1,0,0,0,0";
const char enableGGA[] PROGMEM = "PUBX,40,GGA,0,1,0,0,0,0";
const char enableVTG[] PROGMEM = "PUBX,40,VTG,0,1,0,0,0,0";
const char enableZDA[] PROGMEM = "PUBX,40,ZDA,0,1,0,0,0,0";

const char baud9600[] PROGMEM = "PUBX,41,1,0007,00,03,9600,0";
const char baud38400[] PROGMEM = "PUBX,41,1,0007,00,03,38400,0";
const char baud57600[] PROGMEM = "PUBX,41,1,0007,00,03,57600,0";
const char baud115200[] PROGMEM = "PUBX,41,1,0007,00,03,115200,0";

//--------------------------

const uint32_t COMMAND_DELAY = 250;

void changeBaud(const char *textCommand, unsigned long baud)
{
    gps.send_P(&tee, FPSTR(disableRMC));
    delay(COMMAND_DELAY);
    gps.send_P(&tee, FPSTR(disableGLL));
    delay(COMMAND_DELAY);
    gps.send_P(&tee, FPSTR(disableGSV));
    delay(COMMAND_DELAY);
    gps.send_P(&tee, FPSTR(disableGSA));
    delay(COMMAND_DELAY);
    gps.send_P(&tee, FPSTR(disableGGA));
    delay(COMMAND_DELAY);
    gps.send_P(&tee, FPSTR(disableVTG));
    delay(COMMAND_DELAY);
    gps.send_P(&tee, FPSTR(disableZDA));
    delay(500);
    gps.send_P(&tee, FPSTR(textCommand));
    gpsPort.flush();
    gpsPort.end();

    DEBUG_PORT.print(F("All sentences disabled for baud rate "));
    DEBUG_PORT.print(baud);
    DEBUG_PORT.println(F(" change.  Enter '1' to reenable sentences."));
    delay(500);
    gpsPort.begin(baud);

} // changeBaud

//------------------------------------

static void doSomeWork()
{
    // Print all the things!

    if (tracing)
        trace_all(DEBUG_PORT, gps, fix);

} // doSomeWork

// https://community.particle.io/t/tinygps-using-distancebetween/28233/3
// Returns the great-circle distance (in meters) between two points on a sphere
// lat1, lat2, lon1, lon2 must be provided in Degrees.  (Radians = Degrees * PI / 180, Degrees = Radians / PI * 180)
double haversine(double lat1, double lon1, double lat2, double lon2)
{
    const double rEarth = 6371000.0; // in meters
    double x = pow(sin(((lat2 - lat1) * PI / 180.0) / 2.0), 2.0);
    double y = cos(lat1 * PI / 180.0) * cos(lat2 * PI / 180.0);
    double z = pow(sin(((lon2 - lon1) * PI / 180.0) / 2.0), 2.0);
    double a = x + y * z;
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    double d = rEarth * c;
    // Serial.printlnf("%12.9f, %12.9f, %12.9f, %12.9f, %12.9f, %12.9f", x, y, z, a, c, d);
    return d; // in meters
}

void GPSsetup()
{
    if (!DEBUG_PORT)
    {
        DEBUG_PORT.begin(115200);
        while (!DEBUG_PORT)
        {
        }
        DEBUG_PORT.println(F("ubloxRate.INO: started\r\n"
                             "Looking for GPS device on " GPS_PORT_NAME "\r\n"
                             "Enter '?' for help."));

#ifdef NMEAGPS_INTERRUPT_PROCESSING
#error You must *NOT* define NMEAGPS_INTERRUPT_PROCESSING in NMEAGPS_cfg.h!
#endif

#if !defined(NMEAGPS_PARSE_GGA) & !defined(NMEAGPS_PARSE_GLL) & \
    !defined(NMEAGPS_PARSE_GSA) & !defined(NMEAGPS_PARSE_GSV) & \
    !defined(NMEAGPS_PARSE_RMC) & !defined(NMEAGPS_PARSE_VTG) & \
    !defined(NMEAGPS_PARSE_ZDA) & !defined(NMEAGPS_PARSE_GST)

        DEBUG_PORT.println(F("\nWARNING: No NMEA sentences are enabled: no fix data will be displayed."));

#else
        if (gps.merging == NMEAGPS::NO_MERGING)
        {
            DEBUG_PORT.print(F("\nWARNING: displaying data from "));
            DEBUG_PORT.print(gps.string_for(LAST_SENTENCE_IN_INTERVAL));
            DEBUG_PORT.print(F(" sentences ONLY, and only if "));
            DEBUG_PORT.print(gps.string_for(LAST_SENTENCE_IN_INTERVAL));
            DEBUG_PORT.println(F(" is enabled.\n"
                                 "  Other sentences may be parsed, but their data will not be displayed."));
        }
#endif

        // Make sure the run-time selectable LAST_SENTENCE is
        //   configured correctly in NMEAGPS_cfg.h
        for (uint8_t i = 0; i < 1; i++)
        {
            if (LastSentenceInInterval != LAST_SENTENCE_IN_INTERVAL)
            {
                DEBUG_PORT.println(
                    F("LAST_SENTENCE_IN_INTERVAL is not properly defined in NMEAGPS_cfg.h!\n"
                      "   See Prerequisite 4 above"));
                for (;;)
                    ; // hang here!
            }
            LastSentenceInInterval++;
        }
        LastSentenceInInterval = NMEAGPS::NMEA_GLL;

        trace_header(DEBUG_PORT);
        DEBUG_PORT.flush();
    }

    // BAUD_GPS = 38400UL;
    // BAUD_GPS = 9600UL;

    if (!gpsPort.isListening())
        gpsPort.begin(BAUD_GPS);

    // gps.send_P(&tee, FPSTR(baud9600));
    // gps.send_P(&tee, FPSTR(baud38400));

    // delay(1000);

    // BAUD_GPS = 38400UL;
    // BAUD_GPS = 9600UL;

    // gpsPort.begin(BAUD_GPS);

    // delay(1000);

    sendUBX(ubxRate1Hz, sizeof(ubxRate1Hz));

    sendUBX(ubxDisableVTG, sizeof(ubxDisableVTG));
    sendUBX(ubxDisableGSA, sizeof(ubxDisableGSA));
    sendUBX(ubxDisableGSV, sizeof(ubxDisableGSV));
    sendUBX(ubxDisableGLL, sizeof(ubxDisableGLL));
    sendUBX(ubxDisableGST, sizeof(ubxDisableGST));
    sendUBX(ubxDisableZDA, sizeof(ubxDisableZDA));
    sendUBX(ubxDisableDTM, sizeof(ubxDisableDTM));
    sendUBX(ubxDisableGBS, sizeof(ubxDisableGBS));
    sendUBX(ubxDisableGNS, sizeof(ubxDisableGNS));
    sendUBX(ubxDisableGRS, sizeof(ubxDisableGRS));
    sendUBX(ubxDisableVLW, sizeof(ubxDisableVLW));

    sendUBX(ubxEnableRMC, sizeof(ubxEnableRMC));
    // sendUBX(ubxDisableRMC, sizeof(ubxDisableRMC));

    sendUBX(ubxEnableGGA, sizeof(ubxEnableGGA));
    // sendUBX(ubxDisableGGA, sizeof(ubxDisableGGA));

    sendUBX(ubxEnableTimTP, sizeof(ubxEnableTimTP));
    // sendUBX(ubxDisableTimTP, sizeof(ubxDisableTimTP));

    // sendUBX(ubxEnablePVT, sizeof(ubxEnablePVT));
    sendUBX(ubxDisablePVT, sizeof(ubxDisablePVT));

    // LastSentenceInInterval = NMEAGPS::NMEA_GGA;
    // LastSentenceInInterval = NMEAGPS::NMEA_ZDA;
    LastSentenceInInterval = NMEAGPS::NMEA_GGA;
}

void GPSloop()
{
    // Check for commands

    // parseGPScompleted = false;

#ifdef SERIAL_COMMAND
    if (DEBUG_PORT.available())
    {
        char c = DEBUG_PORT.read();

        switch (c)
        {
        case '?':
            DEBUG_PORT.print(FPSTR(help));
            echoing = false;
            tracing = false;
            DEBUG_PORT.print(F("Enter command> "));
            break;

        case '0':
            if (lastChar == 'r')
            {
                sendUBX(ubxRate10Hz, sizeof(ubxRate10Hz));
            }
            else
            {
                gps.send_P(&tee, FPSTR(enableRMC));
                // delay(COMMAND_DELAY);
                gps.send_P(&tee, FPSTR(disableGLL));
                // delay(COMMAND_DELAY);
                gps.send_P(&tee, FPSTR(disableGSV));
                // delay(COMMAND_DELAY);
                gps.send_P(&tee, FPSTR(disableGSA));
                // delay(COMMAND_DELAY);
                gps.send_P(&tee, FPSTR(enableGGA));
                // delay(COMMAND_DELAY);
                gps.send_P(&tee, FPSTR(disableVTG));
                // delay(COMMAND_DELAY);
                gps.send_P(&tee, FPSTR(disableZDA));
                LastSentenceInInterval = NMEAGPS::NMEA_GGA;
            }
            break;
        case '1':
            if (lastChar == 'r')
            {
                sendUBX(ubxRate1Hz, sizeof(ubxRate1Hz));
            }
            else
            {
                gps.send_P(&tee, FPSTR(enableRMC));
                // delay(COMMAND_DELAY);
                gps.send_P(&tee, FPSTR(enableGLL));
                // delay(COMMAND_DELAY);
                gps.send_P(&tee, FPSTR(enableGSV));
                // delay(COMMAND_DELAY);
                gps.send_P(&tee, FPSTR(enableGSA));
                // delay(COMMAND_DELAY);
                gps.send_P(&tee, FPSTR(enableGGA));
                // delay(COMMAND_DELAY);
                gps.send_P(&tee, FPSTR(enableVTG));
                // delay(COMMAND_DELAY);
                gps.send_P(&tee, FPSTR(enableZDA));
                LastSentenceInInterval = NMEAGPS::NMEA_ZDA;
            }
            break;
        case '3':
            changeBaud(baud38400, 38400UL);
            break;
        case '5':
            if (lastChar == 'r')
            {
                sendUBX(ubxRate5Hz, sizeof(ubxRate5Hz));
            }
            else
            {
                changeBaud(baud115200, 115200UL);
            }
            break;
        case '6':
            if (lastChar == 'r')
            {
                sendUBX(ubxRate16Hz, sizeof(ubxRate16Hz));
            }
            break;
        case '7':
            changeBaud(baud57600, 57600UL);
            break;
        case '9':
            changeBaud(baud9600, 9600UL);
            break;

        case 'd':
            // sendUBX(ubxDisableRMC, sizeof(ubxDisableRMC));
            // delay(COMMAND_DELAY);
            sendUBX(ubxDisableGLL, sizeof(ubxDisableGLL));
            sendUBX(ubxDisableGSV, sizeof(ubxDisableGSV));
            // delay(COMMAND_DELAY);
            sendUBX(ubxDisableGSA, sizeof(ubxDisableGSA));
            // delay(COMMAND_DELAY);
            // sendUBX(ubxDisableGGA, sizeof(ubxDisableGGA));
            // delay(COMMAND_DELAY);
            sendUBX(ubxDisableVTG, sizeof(ubxDisableVTG));
            // delay(COMMAND_DELAY);
            sendUBX(ubxDisableZDA, sizeof(ubxDisableZDA));
            LastSentenceInInterval = NMEAGPS::NMEA_GLL;
            break;

        case 'e':
            if (lastChar == 'r')
            {
                // resetGPS();
                sendUBX(ubxWarmstart, sizeof(ubxWarmstart));
            }
            else
            {
                echoing = !echoing;
            }
            break;

        case 'c':
            if (lastChar == 'r')
            {
                sendUBX(ubxRevertDefaultConfig, sizeof(ubxRevertDefaultConfig));
            }
            break;

        case 't':
            tracing = !tracing;
            break;

        default:
            break;
        }
        lastChar = c;
    }
#endif

#define SERIAL_UBLOX
#ifdef SERIAL_UBLOX
    while (DEBUG_PORT.available())
    {
        if (!gpsPort.isListening())
            gpsPort.begin(BAUD_GPS);

        char c = DEBUG_PORT.read();
        uint8_t hex = c;

        // DEBUG_PORT.print(hex, HEX);
        // DEBUG_PORT.print(' ');
        // gpsPort.write(hex);

        // DEBUG_PORT.print(c);
        // gpsPort.write(c);
        gpsPort.write(hex);
    }
#endif

    //  Check for GPS data

    if (gpsPort.available())
        digitalWrite(LED_1, LOW);

    static bool displayingHex = false;

    uint8_t gpsPortMode = echoing;

    if (gpsPortMode == relaying)
    {
        // Use advanced character-oriented methods to echo received characters to
        //    the Serial Monitor window.
        int i = 0;
        while (gpsPort.available())
        {
            i++;

            char c = gpsPort.read();

            DEBUG_PORT.write((uint8_t)c);

            /*
            if (c == '$')
            {
                DEBUG_PORT.println();
                displayingHex = false;
            }

            if (displayingHex)
            {
                // displayingHex = true;
                // DEBUG_PORT.print(F("0x"));
                if (c < 16)
                {
                    DEBUG_PORT.print('0');
                }
                DEBUG_PORT.print((uint8_t)c, HEX);
                DEBUG_PORT.print(' ');
            }
            else
            {
                DEBUG_PORT.print(c);
            }
            */

            /*
            if (((' ' <= c) && (c <= '~')) || (c == '\r') || (c == '\n'))
            {
                DEBUG_PORT.write(c);
                displayingHex = false;
            }
            else
            {
                if (!displayingHex)
                {
                    displayingHex = true;
                    DEBUG_PORT.print(F("0x"));
                }
                if (c < 0x10)
                    DEBUG_PORT.write('0');
                DEBUG_PORT.print((uint8_t)c, HEX);
                DEBUG_PORT.write(' ');
            }
            */

            gps.handle(c);
            if (gps.available())
            {
                parseGPScompleted = true;

                fix = gps.read();

                // if (displayingHex)
                //     displayingHex = false;

                displayingHex = true;
                // DEBUG_PORT.println();
                doSomeWork();
            }
        }
    }
    else if (gpsPortMode == echoing)
    {
        // Use advanced character-oriented methods to echo received characters to
        //    the Serial Monitor window.

        static uint16_t sentence = 0;
        int i = 0;
        // displayingHex = true;
        static char char_prev = 0;

        bool serialEnd = false;

        bool sentenceReady = false;

        static uint16_t offset = 0;

        const uint16_t length = 256;
        static char bufNMEA[length];
        static char bufUBLOX[length];

        static uint16_t asteriskIndex = 0;

        static uint8_t CK_A = 0;
        static uint8_t CK_B = 0;

        static uint16_t lastPos = 0;

        while (gpsPort.available())
        {
            i++;

            char c = gpsPort.read();

            if (offset == sizeof(bufNMEA) - 1)
            {
                bufNMEA[offset] = '\0';
                bufUBLOX[offset] = '\0';
                offset = 0;
            }

            if (c == '$')
            {
                offset = 0;
                sentence = NMEA;

                DEBUG_PORT.println();
            }
            else if ((uint8_t)c == 0xB5)
            {
                offset = 0;
                sentence = UBLOX;

                CK_A = 0;
                CK_B = 0;

                lastPos = 0;

                DEBUG_PORT.println();
            }

            // char_prev = c;

            //**** print char or hex
            if (sentence == NMEA)
            {
                if (c != '\r' && c != '\n')
                {
                    DEBUG_PORT.print(c);
                }
            }
            else if (sentence == UBLOX)
            {
                DEBUG_PORT.print(c);

                // if ((uint8_t)c < 16)
                //     DEBUG_PORT.print("0");

                // DEBUG_PORT.print((uint8_t)c, HEX);
                // DEBUG_PORT.print(" ");
            }

            if (sentence == NMEA)
            {
                if (c == '*')
                {
                    asteriskIndex = offset;
                }

                if (c != '\r' && c != '\n')
                {
                    bufNMEA[offset] = c;
                    // offset++;
                }

                if (asteriskIndex && offset == asteriskIndex + 2)
                {

                    // sentence = -1;
                    asteriskIndex = 0;
                    sentenceReady = true;

                    // buf[offset] = '\0';
                    // DEBUG_PORT.print(buf);
                }
            }
            else if (sentence == UBLOX)
            {
                bufUBLOX[offset] = (uint8_t)c;

                // if ((uint8_t)c < 16)
                //     DEBUG_PORT.print("0");

                // DEBUG_PORT.print((uint8_t)c, HEX);
                // DEBUG_PORT.print(" ");

                if (lastPos == 0)
                {
                    if (offset > 1)
                    {
                        CK_A = CK_A + (uint8_t)c;
                        CK_B = CK_B + CK_A;
                    }

                    if (offset == 3)
                    {
                        if (bufUBLOX[2] == 0x0D && bufUBLOX[3] == 0x01)
                        {
                            ubloxClassID = TIM_TP;
                            // DEBUG_PORT.print("TIM-TP");
                            // PPS = true;
                        }
                    }

                    if (offset == 5)
                    {
                        // http://forum.arduino.cc/index.php?topic=160623.msg1202380#msg1202380
                        static byte in[4] = {0, 0, 0, 0};
                        in[0] = bufUBLOX[4];
                        in[1] = bufUBLOX[5];
                        unsigned long *lval = (unsigned long *)in; // <-- note the * there to dereference the pointer

                        lastPos = 5 + *lval; //(uint8_t)c;
                    }
                }
                else if (lastPos != 0)
                {
                    if (offset > 1 && offset <= lastPos)
                    {
                        CK_A = CK_A + (uint8_t)c;
                        CK_B = CK_B + CK_A;
                    }

                    if (offset > lastPos)
                    {
                        static byte CheckSum[2] = {0, 0};

                        if (offset == lastPos + 1)
                        {
                            CheckSum[0] = (uint8_t)c;
                        }

                        if (offset == lastPos + 2)
                        {
                            CheckSum[1] = (uint8_t)c;

                            if (bufUBLOX[2] == 0x01 && bufUBLOX[3] == 0x07)
                            {
                                ubloxClassID = NAV_PVT;
                                DEBUG_PORT.print("NAV-PVT");
                            }
                            else if (bufUBLOX[2] == 0x01 && bufUBLOX[3] == 0x22)
                            {
                                ubloxClassID = NAV_CLOCK;
                                DEBUG_PORT.print("NAV-CLOCK");
                            }
                            else if (bufUBLOX[2] == 0x0D && bufUBLOX[3] == 0x01)
                            {
                                // ubloxClassID = TIM_TP;
                                DEBUG_PORT.print("TIM-TP");
                                PPS = true;
                            }
                            else
                            {
                                ubloxClassID = ClassIDUnknown;
                                DEBUG_PORT.print("UNKNOWN CLASS ID");
                            }

                            if (CheckSum[0] == CK_A && CheckSum[1] == CK_B)
                            {

                                DEBUG_PORT.print(", Valid");
                            }
                            else
                            {
                                DEBUG_PORT.print(", Invalid");

                                DEBUG_PORT.print(" checksum: ");
                                DEBUG_PORT.print(CK_A, HEX);
                                DEBUG_PORT.print(" ");
                                DEBUG_PORT.print(CK_B, HEX);
                            }

                            // sentence = -1;
                            sentenceReady = true;

                            CK_A = 0;
                            CK_B = 0;

                            lastPos = 0;
                        }
                    }
                }

                // offset++;
            }

            if (sentence == NMEA)
            {
                gps.handle(c);

                if (gps.available())
                {
                    parseGPScompleted = true;

                    fix = gps.read();

                    doSomeWork();
                }
            }

            // if(sentenceReady)
            // {
            //     sentenceReady = false;
            //     sentence = -1;
            // }

            if (sentenceReady && parseGPScompleted)
            {
                // sentence = -1;
                sentenceReady = false;

                if (ubloxClassID == TIM_TP)
                {
                    // http://forum.arduino.cc/index.php?topic=160623.msg1202380#msg1202380
                    static byte towMS_byte[4] = {0, 0, 0, 0};
                    towMS_byte[0] = bufUBLOX[6];
                    towMS_byte[1] = bufUBLOX[7];
                    towMS_byte[2] = bufUBLOX[8];
                    towMS_byte[3] = bufUBLOX[9];
                    unsigned long *towMS = (unsigned long *)towMS_byte; // <-- note the * there to dereference the pointer

                    unsigned long tow = *towMS / 1000;

                    static byte towSubMS_byte[4] = {0, 0, 0, 0};
                    towSubMS_byte[0] = bufUBLOX[10];
                    towSubMS_byte[1] = bufUBLOX[11];
                    towSubMS_byte[2] = bufUBLOX[12];
                    towSubMS_byte[3] = bufUBLOX[13];
                    unsigned long *towSubMS = (unsigned long *)towSubMS_byte; // <-- note the * there to dereference the pointer

                    static byte week_byte[2] = {0, 0};
                    week_byte[0] = bufUBLOX[18];
                    week_byte[1] = bufUBLOX[19];
                    unsigned short *week = (unsigned short *)week_byte; // <-- note the * there to dereference the pointer

                    DEBUGLOG("\r\nweek:%u, towMS:%lu, towSubMS:%lu, utcTime:%lu", *week, tow, *towSubMS, (unsigned long)fix.dateTime);
                }

                if (ubloxClassID == NAV_CLOCK)
                {
                    // http://forum.arduino.cc/index.php?topic=160623.msg1202380#msg1202380
                    static byte iTOWMS_byte[4] = {0, 0, 0, 0};
                    iTOWMS_byte[0] = bufUBLOX[6];
                    iTOWMS_byte[1] = bufUBLOX[7];
                    iTOWMS_byte[2] = bufUBLOX[8];
                    iTOWMS_byte[3] = bufUBLOX[9];
                    unsigned long *iTOWMS = (unsigned long *)iTOWMS_byte; // <-- note the * there to dereference the pointer

                    // unsigned long iTOW = *iTOWMS/1000;

                    DEBUGLOG("\r\niTOWMS:%lu, utcTime:%lu", *iTOWMS, (unsigned long)fix.dateTime);
                }
            }

            char_prev = c;

            offset++;

            serialEnd = true;
        }
    }
    else
    {
        // Use the normal fix-oriented methods to display fixes
        while (gps.available(gpsPort))
        {
            parseGPScompleted = true;

            fix = gps.read();
            doSomeWork();
            displayingHex = false;
        }
    }

    if (parseGPScompleted)
    {
        // DEBUG_PORT.print("\r\nparseGPScompleted");
    }

    digitalWrite(LED_1, HIGH);
}