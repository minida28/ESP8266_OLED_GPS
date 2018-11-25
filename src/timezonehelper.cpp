#include "timezonehelper.h"
#include "displayhelper.h"
#include "encoderhelper.h"
#include "locationhelper.h"

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

uint16_t create_timezone_start_position(const char *fileName)
{
    PRINT("\r\n%s\r\n", __PRETTY_FUNCTION__);

    File file = SPIFFS.open(fileName, "r");
    if (!file)
    {
        file.close();
        return 0;
    }

    // // skip first line
    // while (file.available())
    // {
    //     if (file.find('\n'))
    //     {
    //         break;
    //     }
    // }

    int count = 0;

    const char *listFileName = "/timezoneIndexPos.csv";

    if (SPIFFS.exists(listFileName))
    {
        // SPIFFS.remove(listFileName);
        return true;
    }

    File listFile = SPIFFS.open(listFileName, "a");
    listFile.println("0");

    while (file.available())
    {
        char c = file.read();

        if (c == '\n')
        {
            count++;
            listFile.println(file.position());

            DEBUGLOG("pos: %d\r\n", file.position());
        }
    }

    listFile.close();

    file.close();

    DEBUGLOG("maxIndex: %d\r\n", count);

    return count;
}

uint8_t get_zones_info(char *bufZonesName, char *bufZonesString)
{
    // PRINT("\r\n%s\r\n", __PRETTY_FUNCTION__);

    File file = SPIFFS.open(PSTR("/zones.csv"), "r");
    if (!file)
    {
        PRINT("\r\nfile /zones.csv not found");

        file.close();
        return 0;
    }

    static int32_t posStart = 0;
    static uint32_t posEnd = 0;

    // file.seek(posStart, SeekSet);

    char buf[128];
    uint8_t offset = 0;
    uint8_t count = 0;
    bool process = false;

    if (rightPinFlag)
    {
        if (posEnd == file.size())
            posStart = 0;
        else
            posStart = posEnd;

        file.seek(posStart, SeekSet);

        count = 0;

        while (file.available())
        {
            char c = file.read();

            posEnd = file.position();

            if (c == '"')
            {
                count++;
            }

            if (c != '"' && c != '\r' && c != '\n')
            {
                buf[offset] = c;
                offset++;
            }

            if (count == 4)
            {
                buf[offset] = '\0';

                process = true;

                break;
            }
        }
    }
    else if (leftPinFlag)
    {
        count = 0;

        // bool next = false;

        while (true)
        {
            if (posStart == 0)
                posStart = file.size();

            posStart--;
            file.seek(posStart, SeekSet);

            char c = file.read();

            if (c != '\r' && c != '\n')
            {
                // Serial.print(c);
            }

            if (c == '"')
            {
                count++;
            }

            if (count == 4)
            {
                file.seek(-1, SeekCur);

                // Serial.println("BREAK");
                // next = true;

                break;
            }
        }

        count = 0;

        while (file.available())
        {
            char c = file.read();

            posEnd = file.position();

            if (c == '"')
            {
                count++;
            }

            if (c != '"' && c != '\r' && c != '\n')
            {
                buf[offset] = c;
                offset++;
            }

            if (count == 4)
            {
                buf[offset] = '\0';

                process = true;

                break;
            }
        }
    }

    else
    {
        // if (posStart == 0 && posEnd == 0)
        // {
            while (!file.find(configLocation.timezonename))
            {
                break;
            }

            posStart = file.position() - (strlen(configLocation.timezonename) + 1);
        // }

        file.seek(posStart, SeekSet);

        count = 0;

        while (file.available())
        {
            char c = file.read();

            posEnd = file.position();

            if (c == '"')
            {
                count++;
            }

            if (c != '"' && c != '\r' && c != '\n')
            {
                buf[offset] = c;
                offset++;
            }

            if (count == 4)
            {
                buf[offset] = '\0';

                process = true;

                break;
            }
        }
    }

    if (process)
    {
        char *token; // this is used by strtok() as an index

        token = strtok(buf, ",");
        strlcpy(bufZonesName, token, strlen(token) + 1);
        token = strtok(NULL, "\0");
        strlcpy(bufZonesString, token, strlen(token) + 1);
    }

    file.close();
    return true;
}