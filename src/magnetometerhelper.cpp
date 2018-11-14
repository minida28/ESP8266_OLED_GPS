#include "magnetometerhelper.h"

HMC5883L compass;

int headingDegInt = 0;

void MAGNETOMETERsetup()
{
    //   Serial.begin(9600);

    // Initialize Initialize HMC5883L
    Serial.println("Initialize HMC5883L");
    while (!compass.begin())
    {
        Serial.println("Could not find a valid HMC5883L sensor, check wiring!");
        delay(500);
    }

    // Set measurement range
    compass.setRange(HMC5883L_RANGE_1_3GA);

    // Set measurement mode
    compass.setMeasurementMode(HMC5883L_CONTINOUS);

    // Set data rate
    compass.setDataRate(HMC5883L_DATARATE_30HZ);

    // Set number of samples averaged
    compass.setSamples(HMC5883L_SAMPLES_8);

    // Set calibration offset. See HMC5883L_calibration.ino
    compass.setOffset(0, 0);
}

void MAGNETOMETERloop()
{
    static unsigned long prevMillis = 0;
    if (millis() - prevMillis > 0)
    {
        prevMillis = millis();

        Vector norm = compass.readNormalize();

        // Calculate heading
        float heading = atan2(norm.YAxis, norm.XAxis);

        // Set declination angle on your location and fix heading
        // You can find your declination on: http://magnetic-declination.com/
        // (+) Positive or (-) for negative
        // For Bytom / Poland declination angle is 4'26E (positive)
        // Formula: (deg + (min / 60.0)) / (180 / M_PI);
        float declinationAngle = (0.0 + (38.0 / 60.0)) / (180 / PI);
        heading += declinationAngle;

        // Correct for heading < 0deg and heading > 360deg
        if (heading < 0)
        {
            heading += 2 * PI;
        }

        if (heading > 2 * PI)
        {
            heading -= 2 * PI;
        }

        // Convert to degrees
        // static int headingDegrees_old = 0;
        double headingDegrees = heading * 180 / PI;

        // Output
        if ((int)headingDegrees != headingDegInt)
        {
            headingDegInt = (int)headingDegrees;

            Serial.print("Heading = ");
            Serial.print(heading);
            Serial.print(" Degress = ");
            Serial.print((int)headingDegrees);
            Serial.println();
        }
    }
}