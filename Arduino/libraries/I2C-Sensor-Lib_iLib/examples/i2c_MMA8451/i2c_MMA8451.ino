#include <Wire.h>
#include "i2c.h"

#include "i2c_MMA8451.h"
MMA8451 mma8451;


void setup()
{
    Serial.begin(115200);

    Serial.print("Probe MMA8451: ");
    if (mma8451.initialize()) Serial.println("Sensor found!");
    else
    {
        Serial.println("Sensor missing");
        while(1) {};
    }
}

void loop()
{
    static float xyz_g[3];

    mma8451.getMeasurement(xyz_g);

    Serial.print(" X: ");
    Serial.print(xyz_g[0],2);
    Serial.print(" \tY: ");
    Serial.print(xyz_g[1],2);
    Serial.print(" \tZ: ");
    Serial.print(xyz_g[2],2);
    Serial.println("");
    delay(20);

}


/**<

Program size:
A1.0.5:
A1.5.7: 6992b
A1.6.3: 6988b / 494b
 */
