#include <Wire.h>
#include "i2c.h"

#include "i2c_TCS3772.h"
TCS3772 tcs3772;


void setup()
{
    Serial.begin(115200);

    Serial.print("Probe TCS3772: ");
    if (tcs3772.initialize()) Serial.println("Sensor found");
    else
    {
        Serial.println("Sensor missing");
        while (1) {}
    }
}

void loop()
{

    static uint16_t value_crgb[4], scale_factor;

    tcs3772.getMeasurement(value_crgb);

    scale_factor = tcs3772.autoGain(value_crgb[0]);

    if (scale_factor)
    {
        Serial.print(" R: ");
        Serial.print(value_crgb[1]);
        Serial.print(" G: ");
        Serial.print(value_crgb[2]);
        Serial.print(" B: ");
        Serial.print(value_crgb[3]);
        Serial.print(" C: ");
        Serial.print(value_crgb[0]);
        Serial.print(" GAIN: ");
        Serial.print(scale_factor);
        Serial.println("");
    }
    delay(50);
}

/**<

Program size:
A1.0.5: 6754b
A1.5.7: 6454b
A1.6.3: 6322b / 494b
 */
