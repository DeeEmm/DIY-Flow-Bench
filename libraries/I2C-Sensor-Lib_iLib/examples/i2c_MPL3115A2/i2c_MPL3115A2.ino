#include <Wire.h>
#include "i2c.h"

#include "i2c_MPL3115A2.h"
MPL3115A2 mpl3115;

void setup()
{
    Serial.begin(115200);

    Serial.print("Probe MPL3115A2: ");
    if (mpl3115.initialize()) Serial.println("Sensor found");
    else
    {
        Serial.println("Sensor missing");
        while (1) {}
    }

    // onetime-measure:
    mpl3115.setEnabled(0);
    mpl3115.triggerMeasurement();
}

void loop()
{
    mpl3115.awaitMeasurement();

    float altitude;
    mpl3115.getAltitude(altitude);

    float temperature;
    mpl3115.getTemperature(temperature);
    mpl3115.triggerMeasurement();

    Serial.print(" Height: ");
    Serial.print(altitude);
    Serial.print(" Temp: ");
    Serial.print(temperature);
    Serial.println("");
}

/**<

Program size:
A1.0.5:
A1.5.7: 6980b
A1.6.3: 6890b / 495b
 */

