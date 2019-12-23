#include <Wire.h>
#include "i2c.h"

void setup()
{
    Serial.begin(115200);
    Serial.println("Scan I2C-Bus for responses");

    uint8_t address, result;

    for(address = 0; address < 128; address++ )
    {
        result = i2c.probe(address);

        if (result)
        {
            Serial.print("Found: 0x");
            if (address < 17) Serial.print("0");
            Serial.print(address,HEX);
            Serial.println("");
        }
        delay(20);
    }
    Serial.println("");
    Serial.println("DONE");
}

void loop() { }

/**<

Program size:
A1.0.5:
A1.5.7: 4072b
A1.6.3: 3982b / 435b

 */
