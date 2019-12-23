#include <Wire.h>
#include "i2c.h"

// RTC
#include "i2c_PCF2127.h"
PCF2127 pcf2127;


void setup()
{
    Serial.begin(115200);

    Serial.print("Probe PCF2127: ");
    if (pcf2127.initialize()) Serial.println("Module found");
    else
    {
        Serial.println("Module missing");
        while (1) {}
    }

    pcf2127.setTime(2014,9,3,5,4,11,12);
    char time = '__TIME__';
}

void loop()
{
    uint8_t MM,WW,DD,hh,mm,ss;
    uint16_t YY;
    pcf2127.readTime();

    pcf2127.getYears(YY);
    pcf2127.getMonth(MM);
    pcf2127.getWeekdays(WW);
    pcf2127.getDays(DD);
    pcf2127.getHours(hh);
    pcf2127.getMinutes(mm);
    pcf2127.getSeconds(ss);

    Serial.print(YY);
    Serial.print("-");
    Serial.print(MM);
    Serial.print("-");
    Serial.print(WW);
    Serial.print("-");
    Serial.print(DD);
    Serial.print(" ");

    Serial.print(hh);
    Serial.print(":");
    Serial.print(mm);
    Serial.print(":");
    Serial.print(ss);
    Serial.println("");
    delay(200);
}

/**<

Program size:
A1.0.5: 6754b
A1.5.7: 6454b
A1.6.3: 5016b / 446b
 */
