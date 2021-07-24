/****************************************
 * The DIY Flow Bench project
 * https://diyflowbench.com
 *
 * configuration.h - define variables and configure hardware
 * 
 * Open source flow bench project to measure and display volumetric air flow using an ESP32 / Arduino.
 *
 ***/

#pragma once


// Don't forget to update the changelog & README Versions!!

#define MAJOR_VERSION "ESP-PORT"
#define MINOR_VERSION "0"
#define BUILD_NUMBER "21072401"
#define RELEASE "V.ESP.X-ALPHA"
#define DEV_BRANCH "https://github.com/DeeEmm/DIY-Flow-Bench/tree/ESP32"


/****************************************
 * DEFAULT USER SETTINGS
 *
 ***/

//#define CYCLIC_AVERAGE_BUFFER 5           // Number of scans over which to average output (helps stabilise results)
#define MIN_TEST_PRESSURE_PERCENTAGE 80      // Lowest test pressure bench will generate accurate results. Please see note in wiki

#define CONF_API_ENABLED                       // enable API
#define CONF_DISABLE_API_CHECKSUM              // Add checksum to serial API response TODO UPDATE CHECKSUM TO NATIVE ESP32 CRC32


/****************************************
 * CONFIGURE FILESYSTEM
 ***/

#define FORMAT_FILESYSTEM_IF_FAILED true




/****************************************
 * SELECT BOARD TYPE 
 *
 * Default ESP32DUINO 
 ***/

//#define DIYFB_SHIELD                    // In development
#define ESP32DUINO                        // Wemos D1 R32



/****************************************
 * CONFIGURE COMMUNICATIONS
 ***/


#define BME280_I2C_ADDR 0x77              // (default 0x77) / Alternate 0x76
  


 
/****************************************
 * CONFIGURE ALARMS
 ***/
#define showAlarms true 

//TODO - more granular alarm control - need to be able to disable individual alarms



/****************************************
 * CONFIGURE MAF
 * Uncomment active MAF
 * If you want to modify the code to include additional MAF sensors
 * You will need to create your own MAF data file. Use exampleSensor.h as an example
 ***/


#include "mafData/ACDELCO_ 92281162.h"      // GM LS2              
//#include "mafData/MH95-3000-100.h"        // PMAS MH95-3000 in 100mm housing              
//#include "mafData/exampleKeyValueData.h"  // Example key > value data file (duplicate this as required)
//#include "mafData/exampleAnalogData.h"    // Example Analog point data file (duplicate this as required)
//#include "mafData/SIEMENS_5WK9605.h"      // Data from Tonys tests
//#include "mafData/DELPHI_AF10118.h"       // kg/hr - Data from efidynotuning.com/maf.htm (Stock - Ford '98 Explorer 5.0L)
//#include "mafData/TEST.h"                 // Test Data



/****************************************
 * CONFIGURE REFERENCE PRESSURE SENSOR
 * If you want to modify the code to include additional reference pressure sensors
 * You will need to add your volts to kPa algorythm in the function getRefPressure()
 ***/
#define DEFAULT_REF_PRESS 1

// Default: no selection
//#define REF_MPXV7007                      // -7 / +7 psi
//#define REF_MPX4250                       // 0-36psi (absolute)



/****************************************
 * CONFIGURE PITOT PRESSURE SENSOR
 * If you want to modify the code to include additional pitot pressure sensors
 * You will need to add your volts to kPa algorythm in the function getPitotPressure()
 * Note Pitot sensors need to be a differential pressure sensor (DP)
 ***/

// Default: no selection

// Uncomment ONE of the following
//#define PITOT_MPXV7007DP                  



/****************************************
 * CONFIGURE BARO SENSOR
 *
 * Default Baro 101.3529kpa - standard sealevel baro pressure (14.7 psi) 
 ***/
#define DEFAULT_BARO 101.3529

// Uncomment ONE of the following
//#define USE_REF_PRESS_AS_BARO
//#define BARO_SPARKFUN_BME280 
//#define BARO_MPX4115 
//#define BARO_ADAFRUIT_BME280

#define startupBaroScalingFactor 1        // scaling factor when using reference pressure sensor for baro correction
#define startupBaroScalingOffset 100      // scaling offset when using reference pressure sensor for baro correction



/****************************************
 * CONFIGURE TEMPERATURE SENSOR
 *
 * Default 21 Degrees Celcius
 ***/
#define DEFAULT_TEMP 21

// Uncomment ONE of the following
//#define TEMP_SPARKFUN_BME280
//#define TEMP_ADAFRUIT_BME280
//#define SIMPLE_TEMP_DHT11 



/****************************************
 * CONFIGURE HUMIDITY SENSOR
 *
 * Default 36% Rel H
 ***/
#define DEFAULT_RELH 36

// Uncomment ONE of the following
//#define RELH_SPARKFUN_BME280
//#define SIMPLE_RELH_DHT11 
//#define RELH_ADAFRUIT_BME280




 /****************************************
 * ADVANCED CALIBRATION SETTINGS
 *
 * For a more detailed explaination of these settings
 * Please refer to the WIKI - https://github.com/DeeEmm/DIY-Flow-Bench/wiki/4.-Configuration
 *
 ***/

// NOTE: This method is currently unused
// It is not clear if this method would produce usable results for all sensors.
// Suggest that once project is stable, method is implimented and results compared against known good data to validate 

// generate MAF data dable using three point method
#define calibrationPlateHighCFM 100       // Flow rate for large calibration orifice
#define calibrationPlateMidCFM 50         // Flow rate for med calibration orifice
#define calibrationPlateLowCFM 10         // Flow rate for small calibration orifice




 /****************************************
 * DEVELOPMENT SETTINGS
 *
 * Test environment /DM 
 ***/
//#define DM
#if defined DM
    #define BARO_SPARKFUN_BME280
    #define TEMP_SPARKFUN_BME280
    #define RELH_SPARKFUN_BME280
#endif