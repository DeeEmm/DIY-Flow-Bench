//////////////////////////////////////////////////////////////////////////////////////
// BMP280_DEV - ESP32 HSPI Communications, Default Configuration, Normal Conversion
//////////////////////////////////////////////////////////////////////////////////////

#include <BMP280_DEV.h>                            // Include the BMP280_DEV.h library

float temperature, pressure, altitude;            // Create the temperature, pressure and altitude variables
//BMP280_DEV bmp280(21);                            // Create BMP280_DEV object and set-up for VSPI operation, SCK 5, MOSI 18, MISO 19, SS 21
SPIClass SPI1(HSPI);                              // Create (instantiate) the SPI1 object for HSPI operation
BMP280_DEV bmp280(21, HSPI, SPI1);                // Create BMP280_DEV object and set-up for HSPI operation, SCK 14, MOSI 13, MISO 27, SS 21

void setup() 
{
  Serial.begin(115200);                           // Initialise the serial port
  bmp280.begin();                                 // Default initialisation, place the BMP280 into SLEEP_MODE 
  bmp280.setTimeStandby(TIME_STANDBY_1000MS);     // Set the standby time to 1 second (1000ms)
  bmp280.startNormalConversion();                 // Start NORMAL continuous conversion
  
  xTaskCreatePinnedToCore(                        // Kick-off "TaskOne" pinned to core 1
    taskOne,
    "TaskOne",
    10000,
    NULL,
    1,
    NULL,
    1);
}

void taskOne(void* parameter)
{
  while(true)
  {
    if (bmp280.getMeasurements(temperature, pressure, altitude))    // Check if the measurement is complete
    {
      Serial.print(temperature);                    // Display the results    
      Serial.print(F("*C   "));
      Serial.print(pressure);    
      Serial.print(F("hPa   "));
      Serial.print(altitude);
      Serial.println(F("m"));  
    }
  }
}

void loop() { delay(1000); }                        // Add 1 second delay
