#include <Arduino.h>
#include <TM1637Display.h>

// LED Display module connection pins (Digital Pins)
#define CLK 2
#define DIO 3

TM1637Display display(CLK, DIO);

// The amount of time (in milliseconds) between scans
#define SAMPLE_DELAY   250


void setup()
{
  Serial.begin(9600);
  display.setBrightness(7);
}

void loop()
{

  //read MAF
  int sensorValue = analogRead(A0);

  //scale MAF input to give voltage + divide by 100 to show on LED display
  float voltage = (sensorValue * (5.0 / 1023.0))*100;

  //filter out transients smalller then 0.01 volts
  if (voltage < 10) {
    voltage = 0;
  }

  //send data to serial
  Serial.println(voltage);

  //display data on LED display
  display.showNumberDec(voltage, false);

  delay(SAMPLE_DELAY);
}
