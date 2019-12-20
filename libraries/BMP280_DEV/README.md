# BMP280_DEV
An Arduino compatible, non-blocking, I2C/SPI library for the Bosch BMP280 barometer.

![alt text](https://cdn-learn.adafruit.com/assets/assets/000/026/851/small240/sensors_2651_iso_ORIG.jpg?1438369374 "Adafruit BMP280 Breakout Board")

© Copyright, image courtesy of [Adafruit Industries](https://www.adafruit.com/product/2651) lisensed under the terms of the [Create Commons Attribution-ShareAlike 3.0 Unported](https://creativecommons.org/licenses/by-sa/3.0/legalcode). 

This BMP280_DEV library offers the following features:

- Returns temperature in degrees celius (**°C**), pressure in hectoPascals/millibar (**hPa**) and altitude in metres (**m**)
- NORMAL or FORCED modes of operation
- I2C or hardware SPI communications with configurable clock rates
- Non-blocking operation 
- In NORMAL mode barometer returns results at the specified standby time interval
- Highly configurable, allows for changes to pressure and temperature oversampling, IIR filter and standby time

## __Version__

- Version 1.0.6 -- Merged multiple instances and initialisation pull requests by sensslen
- Version 1.0.5 -- Fixed bug in BMP280_DEV::getTemperature() function, thanks to Jon M.
- Version 1.0.3 -- Change library name in the library.properties file
- Version 1.0.2 -- Modification to allow external creation of a HSPI object on the ESP32
- Version 1.0.1 -- Added ESP32 HSPI support and changed library name for Arduino compatibility
- Version 1.0.0 -- Intial version

## __Arduino Compatibility__

- All Arduino boards, but for 5V Arduino boards (such as the Uno, Nano, Mega, Leonardo, etc...), please check if the BMP280 breakout board requires a 5V to +3.3V voltage level shifter

## __Installation__

After download simply un-zip the file and place the BMP280 directory in your _.../Arduino/libraries/..._ folder. The _.../Arduino/..._ folder is the one where your Arduino IDE sketches are usually located.

## __Usage__

### __BMP280_DEV Library__

Simply include the BMP280_DEV.h file at the beginning of your sketch:

```
#include <BMP280_DEV.h>
```

For I2C communication the BMP280_DEV object is created (instantiated) without parameters:

```
BMP280_DEV bmp280;	// Set up I2C communications
```

By default the library uses the BMP280's I2C address 0x77. (To use the alternate I2C address: 0x76, see the begin() function below.

For SPI communication the chip select (CS) Arduino digital output pin is specified as an argument, for example digital pin 10:

```
BMP280_dev bmp280(10);	// Set up SPI communications on digital pin D10
```

The library also supports the ESP32 HSPI operation on pins: SCK 14, MOSI 13, MISO 27 and user defined SS (CS):

```
SPIClass SPI1(HSPI);							// Create the SPI1 HSPI object
BMP280_DEV bmp(21, HSPI, SPI1);		// Set up HSPI port communications on the ESP32
```

By default the I2C runs in fast mode at 400kHz and SPI at 1MHz.

---
### __Device Initialisation__

To initialise the bmp280 it is necessary to call the begin() function with or without parameters. The parameters specify the starting mode, pressure/temperature oversampling, IIR filter and standby time options respectively:

```
bmp280.begin(SLEEP_MODE, OVERSAMPLING_X16, OVERSAMPLING_X2, IIR_FILTER_4, TIME_STANDBY_05MS);
```

Alternatively simply call the begin function without any paremeters, this sets up the default configuration: SLEEP_MODE, pressure oversampling X16, temperature oversampling X2, IIR filter OFF and a standby time of 0.5ms:

```
bmp280.begin();	// Initialise the BMP280 with default configuration
```

Another alternative is to pass the BMP280's mode as an argument:

```
bmp280.begin(NORMAL_MODE);	// Initialise the BMP280 in NORMAL_MODE with default configuration
```

Or, specifying mode and alternate I2C address:

```
bmp280.begin(FORCED_MODE, BMP280_I2C_ALT_ADDR);	// Initialise the BMP280 in FORCED_MODE with the alternate I2C address (0x76)
```

Or even just the alternate I2C address, (BMP280 initialised in SLEEP_MODE by default):

```
bmp280.begin(BMP280_I2C_ALT_ADDR);	// Initialise the BMP280 with the alternate I2C address (0x76)
```

Note that the begin functions return the value 1 upon successful initialisation, otherwise it returns 0 for failure.

---
### __Device Configuration__

After initialisation it is possible to change the BMP280 configuration with the following functions:

```
bmp280.setPresOversamping(OVERSAMPING_X4);	// Options are OVERSAMPLING_SKIP, _X1, _X2, _X4, _X8, _X16
```

```
bmp280.setTempOversamping(OVERSAMPING_X4);	// Options are OVERSAMPLING_SKIP, _X1, _X2, _X4, _X8, _X16
```

```
bmp280.setIIRFilter(IIR_FILTER_16);	// Options are IIR_FILTER_OFF, _2, _4, _8, _16
```

```
bmp280.setTimeStandby(TIME_STANDBY_2000MS);	// Options are TIME_STANDBY_05MS, _62MS, _125MS, _250MS, _500MS, _1000MS, 2000MS, 4000MS
```
---
### __Modes Of Operation__

The BMP280 has 3 modes of operation: **SLEEP_MODE**, **NORMAL_MODE** and **FORCED_MODE**: 

- **SLEEP_MODE**: puts the device into an inactive standby state 

- **NORMAL_MODE**: performs continuous conversions, separated by the standby time

- **FORCED_MODE**: performs a single conversion, returning to **SLEEP_MODE** upon completion

To kick-off conversions in **NORMAL_MODE**:

```
bmp280.startNormalConversion();	// Start continuous conversions, separated by the standby time
```

To perform a single oneshot conversion in **FORCED_MODE**:

```
bmp280.startForcedConversion();	// Start a single oneshot conversion
```

To stop the conversion at anytime and return to **SLEEP_MODE**:

```
bmp280.stopConversion();	// Stop conversion and return to SLEEP_MODE
```
---
### __Results Acquisition__

The BMP280 barometer library acquires temperature in degrees celius (**°C**), pressure in hectoPascals/millibar (**hPa**) and altitude in metres (**m**). The acquisition functions scan the BMP280's status register and return 1 if the barometer results are ready and have been successfully read, 0 if they are not; this allows for non-blocking code implementation. The temperature, pressure and altitude results themselves are _float_ variables by passed reference to the function and are updated upon a successful read.

Here are the results acquisition functions:

```
bmp280.getMeasurements(temperature, pressure, altitude);	// Acquire temperature, pressue and altitude measurements
```

```
bmp280.getTempPres(temperature, pressure);	// Acquire both the temperature and pressure
```

```
bmp280.getTemperature(temperature);	// Acquire the temperature only
```

```
bmp280.getPressure(pressure);	// Acquire the pressure only, (also calculates temperature, but doesn't return it)
```

```
bmp280.getAltitude(altitude);	// Acquire the altitude only
```
---
### __Code Implementation__

Here is an example sketch of how to use the BMP280 library for non-blocking I2C operation, default configuration with continuous conversion in NORMAL_MODE, but with a standby sampling time of 1 second:

```
#include <BMP280_DEV.h>                           // Include the BMP280_DEV.h library

float temperature, pressure, altitude;            // Create the temperature, pressure and altitude variables
BMP280_DEV bmp280;                                // Instantiate (create) a BMP280_DEV object and set-up for I2C operation (address 0x77)

void setup() 
{
  Serial.begin(115200);                           // Initialise the serial port
  bmp280.begin();                                 // Default initialisation, place the BMP280 into SLEEP_MODE 
  bmp280.setTimeStandby(TIME_STANDBY_1000MS);     // Set the standby time to 1s
  bmp280.startNormalConversion();                 // Start NORMAL conversion mode
}

void loop() 
{
  if (bmp280.getMeasurements(temperature, pressure, altitude))    // Check if the measurement is complete
  {
    Serial.print(temperature);                    // Display the results    
    Serial.print(F("*C   "));
    Serial.print(pressure);    
    Serial.print(F("hPa   "));
    Serial.print(altitude);
    Serial.println(F(m"));
  }
}
```

A second sketch example for I2C operation, default configuration in FORCED conversion mode:

```
#include <BMP280_DEV.h>                           // Include the BMP280_DEV.h library

float temperature, pressure, altitude;            // Create the temperature, pressure and altitude variables
BMP280_DEV bmp280;                                // Instantiate (create) a BMP280_DEV object and set-up for I2C operation (address 0x77)

void setup() 
{
  Serial.begin(115200);                           // Initialise the serial port
  bmp280.begin();                                 // Default initialisation, place the BMP280 into SLEEP_MODE 
}

void loop() 
{
  bmp280.startForcedConversion();                 // Start a forced conversion (if in SLEEP_MODE)
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
```

The sketches for SPI operation are identical except that the line:

```
BMP280_DEV bmp280;	// Instantiate (create) a BMP280_DEV object and set-up for I2C operation (address 0x77)
```

...should be replaced with the line:

```
BMP280_DEV bmp280(10);	// Instantiate (create) a BMP280_DEV object and set-up for SPI operation with chip select on D10
```

For more details see code examples provided in the _.../examples/..._ directory.

---
## __Example Code__

I2C Normal Mode: sampling at 2 seconds standby time intervals with default settings

I2C Normal Mode (Alterntive I2C Address): sampling at 2 second standby time intervals with default settings

I2C Forced Mode: with default settings

SPI Normal Mode: sampling at 2 second standby time intervals with default settings

SPI Forced Mode: with default settings

ESP32 HSPI Normal Mode: sampling at 1 second standby time intervals with default settings

