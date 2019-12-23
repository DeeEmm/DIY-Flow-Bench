#include <Wire.h>
#include "i2c.h"

// IMU-Sensor
#include "i2c_MPU9250.h"
MPU9250 mpu9250;

void setup()
{
    Serial.begin(115200);

    Serial.print("Probe MPU9250: ");
    switch (mpu9250.initialize())
    {
        case 0: Serial.println("MPU-Sensor missing"); while(1) {};
        case 1: Serial.println("Found unknown Sensor."); break;
        case 2: Serial.println("MPU6500 found."); break;
        case 3: Serial.println("MPU9250 found!"); break;
    }

    Serial.print("Probe AK8963: ");
    if (i2c.probe(0x0C)) Serial.println("AK8963 found!");
    else                 Serial.println("AK8963 missing");

}

void loop()
{
    static float xyz_GyrAccMag[9];

    mpu9250.getMeasurement(xyz_GyrAccMag);

    Serial.print("XYZ ACC g[");
    Serial.print(xyz_GyrAccMag[0],2);
    Serial.print(";");
    Serial.print(xyz_GyrAccMag[1],2);
    Serial.print(";");
    Serial.print(xyz_GyrAccMag[2],2);
    Serial.print("]");

    Serial.print(" \t GYR dps[");
    Serial.print(xyz_GyrAccMag[4],2);
    Serial.print(";");
    Serial.print(xyz_GyrAccMag[5],2);
    Serial.print(";");
    Serial.print(xyz_GyrAccMag[6],2);
    Serial.print("]");

    Serial.print(" \t T: ");
    Serial.print(xyz_GyrAccMag[3],2);
    Serial.print(" C");

    Serial.println("");
    delay(20);
}

/**<

Program size:
A1.0.5:
A1.5.7:
A1.6.3: 7752b / 631b
 */
