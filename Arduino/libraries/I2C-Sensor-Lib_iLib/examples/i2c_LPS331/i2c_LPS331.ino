#include <Wire.h>
#include "i2c.h"

// Pressure-Sensor
#include "i2c_LPS331.h"
LPS331 lps331;


void setup()
{
    Serial.begin(115200);

    Serial.print("Probe LPS331: ");
    if (lps331.initialize()) Serial.println("Sensor found!");
    else
    {
        Serial.println("Sensor missing");
        while(1) {};
    }
}

void loop()
{
    static float mbar, degC;

    Serial.print("Altitude: ");
    //static int32_t cm;
    //lps331.getAltitude(cm);
    //Serial.print(cm);
    static float meter;
    lps331.getAltitude(meter);
    Serial.print(meter);

    //lps331.getMeasurement(mbar);
    //Serial.print("Pressure: ");
    //Serial.print(mbar);

    lps331.getTemperature(degC);
    Serial.print(" \tTemperature: ");
    Serial.print(degC);
    Serial.println("");

    delay(20);
}


/**<

Program size:
A1.0.5: 7098b
A1.5.7:
A1.6.3: 7184b / 567b
 */
