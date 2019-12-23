#include <Wire.h>
#include "i2c.h"

#include "i2c_BMP280.h"
BMP280 bmp280;

void setup()
{
    Serial.begin(115200);

    Serial.print("Probe BMP280: ");
    if (bmp280.initialize()) Serial.println("Sensor found");
    else
    {
        Serial.println("Sensor missing");
        while (1) {}
    }

    // onetime-measure:
    bmp280.setEnabled(0);
    bmp280.triggerMeasurement();
}

void loop()
{
    bmp280.awaitMeasurement();

    float temperature;
    bmp280.getTemperature(temperature);

    float pascal;
    bmp280.getPressure(pascal);

    static float meters, metersold;
    bmp280.getAltitude(meters);
    metersold = (metersold * 10 + meters)/11;

    bmp280.triggerMeasurement();

    Serial.print(" HeightPT1: ");
    Serial.print(metersold);
    Serial.print(" m; Height: ");
    Serial.print(meters);
    Serial.print(" Pressure: ");
    Serial.print(pascal);
    Serial.print(" Pa; T: ");
    Serial.print(temperature);
    Serial.println(" C");
}

/**<

Program size:
A1.0.5:
A1.5.7: 9680b
A1.6.3: 9664b / 561b

 */

