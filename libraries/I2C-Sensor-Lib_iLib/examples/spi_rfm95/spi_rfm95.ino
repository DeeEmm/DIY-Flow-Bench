#include <SPI.h>
#include "spi_rfm95.h"
RFM95 rfm;


#define TIMEON   20
#define TIMEOFF  (1000-TIMEON)
#define LEDPINA   8


void setup()
{
    Serial.begin(115200);

    Serial.println(rfm.getFrequency());
    Serial.print("Probe RFM95W: ");
    if (rfm.initialize()) Serial.println("missing");
    else
    {
        Serial.println("found");
        while(1) {};
    }

    Serial.println(rfm.getFrequency());
    rfm.receiveDataCont();

    //while (1) { rfm.handleIRQ(); };

    pinMode(LEDPINA, OUTPUT);
    digitalWrite(LEDPINA,HIGH);
}

void loop()
{
    rfm.handleIRQ();
    delay(TIMEOFF);
    if (rfm.canSend())
    {
        digitalWrite(LEDPINA,HIGH);
        rfm.sendData();
    };

    delay(TIMEON);
    digitalWrite(LEDPINA,LOW);
}

/**<

Program size:
A105: b
A157: b

 */

