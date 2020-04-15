#include <Wire.h>
#include "i2c.h"

#include "i2c_L3G.h"
L3G l3g;


void setup()
{
    Serial.begin(115200);

    Serial.print("Probe L3G: ");
    if (l3g.initialize()) Serial.println("Sensor found!");
    else
    {
        Serial.println("Sensor missing");
        while(1) {};
    }
}

void loop()
{
    float xyz_dps[3];

    l3g.getMeasurement(xyz_dps);

    Serial.print(" X: ");
    Serial.print(xyz_dps[0],2);
    Serial.print(" \tY: ");
    Serial.print(xyz_dps[1],2);
    Serial.print(" \tZ: ");
    Serial.print(xyz_dps[2],2);
    Serial.println("");
    delay(20);

}

/**<

Program size:
A1.0.5:
A1.5.7: 7226b
A1.6.3: 7160b / 483b
 */
