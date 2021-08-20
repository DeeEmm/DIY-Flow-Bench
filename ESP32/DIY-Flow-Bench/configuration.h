/***********************************************************
* The DIY Flow Bench project
* https://diyflowbench.com
*
* configuration.h - define variables and configure hardware
* 
* Open source flow bench project to measure and display volumetric air flow using an ESP32 / Arduino.
*
***/
#pragma once

#include "constants.h"


// Don't forget to update the changelog & README Versions!!
// TODO: Automate build numbering with git tasks

#define MAJOR_VERSION "ESP-PORT"
#define MINOR_VERSION "0"
#define BUILD_NUMBER "21082001"
#define RELEASE "V.ESP.X-ALPHA"
#define DEV_BRANCH "https://github.com/DeeEmm/DIY-Flow-Bench/tree/ESP32"


/***********************************************************
* SYSTEM SETTINGS
***/
#define BOOT_MESSAGE "May the flow be with you..."
#define showAlarms true
#define MIN_REFRESH_RATE 200
#define LANGUAGE_FILE "language/EN_Language.h"



/***********************************************************
* DEFAULT USER SETTINGS
*
***/

#define MIN_TEST_PRESSURE_PERCENTAGE 80                     // Lowest test pressure bench will generate accurate results. Please see note in wiki
#define CONF_API_ENABLED                                    // enable API
#define CONF_DISABLE_API_CHECKSUM                           // Add checksum to serial API response TODO: UPDATE CHECKSUM TO NATIVE ESP32 CRC32



/***********************************************************
* CONFIGURE FILESYSTEM
***/

#define FORMAT_FILESYSTEM_IF_FAILED true




/***********************************************************
* SELECT BOARD TYPE 
*
* Default ESP32DUINO 
***/

//#define DIYFB_SHIELD                    
#define ESP32DUINO




/***********************************************************
* CONFIGURE COMMUNICATIONS
***/

// (default 0x77) / Alternate 0x76
#define BME280_I2C_ADDR 0x77            
  



/***********************************************************
* CONFIGURE MAF
* Uncomment active MAF
* If you want to modify the code to include additional MAF sensors
* You will need to create your own MAF data file. Use exampleSensor.h as an example
***/
    
// Uncomment One line only
#define MAF_SENSOR_FILE "mafData/ACDELCO_ 92281162.h" 
// #define MAF_SENSOR_FILE  "MH95-3000-100"                 // PMAS MH95-3000 in 100mm housing              
// #define MAF_SENSOR_FILE "mafData/SIEMENS_5WK9605"        // Data from Tonys tests
// #define MAF_SENSOR_FILE  "mafData/DELPHI_AF10118"        // kg/hr - Data from efidynotuning.com/maf.htm 
// #define MAF_SENSOR_FILE  "mafData/exampleKeyValueData"   // Example key > value data file 
// #define MAF_SENSOR_FILE  "mafData/exampleAnalogData"     // Example Analog point data file 
// #define MAF_SENSOR_FILE  "mafData/TEST"                  // Test Data



/***********************************************************
* CONFIGURE REFERENCE PRESSURE SENSOR
* If you want to modify the code to include additional reference pressure sensors
* You will need to add your volts to kPa algorithm in the function calculateRefPressure()
***/
#define DEFAULT_REF_PRESS 1

// Uncomment One line only
#define PREF_SENSOR_NOT_USED
// #define PREF_SENSOR_TYPE_MPXV7007      
// #define PREF_SENSOR_TYPE_PX4250         



/***********************************************************
* CONFIGURE PITOT PRESSURE SENSOR
* If you want to modify the code to include additional pitot pressure sensors
* You will need to add your volts to kPa algorythm in the function calculatePitotPressure()
* Note Pitot sensors need to be a differential pressure sensor (DP)
***/

// Uncomment One line only
#define PITOT_SENSOR_NOT_USED
// #define PITOT_SENSOR_TYPE_MPXV7007DP

/***********************************************************
* CONFIGURE BARO SENSOR
*
* Default Baro 101.3529kpa - standard sealevel baro pressure (14.7 psi) 
***/


// Uncomment One line only
#define BARO_SENSOR_TYPE_FIXED_VALUE
// #define BARO_SENSOR_TYPE_LINEAR_ANALOG                   // Use analog signal from REF_BARO_PIN
// #define BARO_SENSOR_TYPE_REF_PRESS_AS_BARO
// #define BARO_SENSOR_TYPE_ADAFRUIT_BME280
// #define BARO_SENSOR_TYPE_SPARKFUN_BME280
// #define BARO_SENSOR_TYPE_MPX4115

#define DEFAULT_BARO 101.3529
#define BARO_ANALOG_SCALE 1.0                               // Scaling factor used for raw analog value
#define startupBaroScalingFactor 1                          // scaling factor when using reference pressure sensor for baro correction
#define startupBaroScalingOffset 100                        // scaling offset when using reference pressure sensor for baro correction



/***********************************************************
* CONFIGURE TEMPERATURE SENSOR
*
* Default 21 Degrees Celsius
***/
#define DEFAULT_TEMP 21                                     // Value to return if no sensor used
#define TEMP_ANALOG_SCALE 1.0                               // Scaling factor used for raw analog value

// Uncomment One line only
//#define TEMP_SENSOR_NOT_USED
#define TEMP_SENSOR_TYPE_FIXED_VALUE
// #define TEMP_SENSOR_TYPE_LINEAR_ANALOG                   // Use analog signal from TEMPERATURE_PIN
// #define TEMP_SENSOR_TYPE_SPARKFUN_BME280
// #define TEMP_SENSOR_TYPE_ADAFRUIT_BME280
// #define TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11



/***********************************************************
* CONFIGURE HUMIDITY SENSOR
*
* Default 36% Rel H
***/
#define DEFAULT_RELH 36                                     // Value to return if no sensor used
#define RELH_ANALOG_SCALE 1.0                               // Scaling factor for raw analog value

// Uncomment ONE of the following
//#define RELH_SENSOR_NOT_USED
#define RELH_SENSOR_TYPE_FIXED_VALUE
// #define RELH_SENSOR_TYPE_LINEAR_ANALOG                   // Use analog signal from HUMIDITY_PIN
// #define RELH_SENSOR_TYPE_SPARKFUN_BME280
// #define RELH_SENSOR_TYPE_ADAFRUIT_BME280
// #define RELH_SENSOR_TYPE_SIMPLE_RELH_DHT11




/***********************************************************
* ADVANCED CALIBRATION SETTINGS
*
* For a more detailed explanation of these settings
*
***/

// NOTE: This method is currently unused
// It is not clear if this method would produce usable results for all sensors.
// Suggest that once project is stable, method is implimented and results compared against known good data to validate 

// generate MAF data table using three point method
#define calibrationPlateHighCFM 100                 // Flow rate for large calibration orifice
#define calibrationPlateMidCFM 50                   // Flow rate for med calibration orifice
#define calibrationPlateLowCFM 10                   // Flow rate for small calibration orifice




/***********************************************************
* DEVELOPMENT SETTINGS
*
* Test environment /DM 
***/
#if defined DEBUG
    #define BARO_SENSOR_TYPE_SPARKFUN_BME280
    #define TEMP_SENSOR_TYPE_SPARKFUN_BME280
    #define RELH_SENSOR_TYPE_SPARKFUN_BME280
#endif