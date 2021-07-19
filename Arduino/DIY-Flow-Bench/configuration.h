/****************************************
 * The DIY Flow Bench project
 * https://diyflowbench.com
 *
 * configuration.h - define variables and configure hardware
 * 
 * Open source flow bench project to measure and display volumetric air flow using an ESP32 / Arduino.
 *
 * The standard project board is the ESP32DUINO  
 *   
 * Other ESP32 based boards can be made to work. 
 * You can define custom pin definitions in boards.h
 * 
 * Default temp / baro / RelH uses BME280 device (Sparkfun / clone)
 * I2C address for the BME280 is 0x77
 *
 * Default MAF unit recommended is the GM LS2 MAF (ACDELCO_ 92281162.h)
 * This will measure up to approx 277cfm
 * 
 * DEPENDENCIES
 * The program has a number of dependencies that must be available for it to work.
 *
 * <EEPROM.h>
 * <WiFi.h>
 * <WebServer.h>
 * <SPIFFS.h>
 * <Adafruit_BME280_DEV.h> 
 * "SparkFunBME280.h"
 * <Wire.h>
 * <SimpleDHT.h> 
 *
 *
 ***/

#pragma once


// Don't forget to update the changelog & README Versions!!

#define MAJOR_VERSION "ESP"
#define MINOR_VERSION "0"
#define BUILD_NUMBER "21071901"
#define RELEASE "V.ESP.X-ALPHA"




/****************************************
 * CONFIGURE FILESYSTEM
 ***/

#define FORMAT_FILESYSTEM_IF_FAILED true



/****************************************
 * LANGUAGE SETTINGS
 ***/

#define LANGUAGE_FILE "language/EN_Language.h"



/****************************************
 * DEVELOPER SETTINGS
 ***/

//#define DEBUG_MODE             
//#define DM



/****************************************
 * GENERAL SETTINGS
 ***/

#define MIN_BENCH_PRESSURE 0              // Minimum pressure that we consider the bench is 'operational' / 'running' / vac source is on
#define MIN_FLOW_RATE 3                   // Flow rate in cfm below which bench is considered off
#define CYCLIC_AVERAGE_BUFFER 5           // Number of scans over which to average output (helps stabilise results)
#define MIN_MAF_MILLIVOLTS 100
#define minTestPressurePercentage 80      // Lowest test pressure bench will generate accurate results. Please see note in wiki



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

#define API_ENABLED                       // enable API
#define DISABLE_API_CHECKSUM              // Add checksum to serial API response TODO UPDATE CHECKSUM TO NATIVE ESP32 CRC32
#define API_DELIM ':'

#define SERIAL_BAUD_RATE 115200   

#define WIFI_SSID "WIFI-SSID"             // Your Wifi SSID
#define WIFI_PSWD "<WIFI-PSWD>"           // Your Wifi Password
#define HOSTNAME "diy_fb"                 // Default Hostname
#define ACESSPOINT_NAME "DIY_FB"          // Default Accesspoint name

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
 * LEAK TEST SETTINGS
 ***/
#define leakTestTolerance 2               // Tolerance in cfm

// TODO - Review need for automaticed check. Possibly better to just display stored value



 /****************************************
 * CALIBRATION SETTINGS
 ***/
 
#define calibrationRefPressure 10         // Reference pressure orifices were measured at (leave at 10" if calibrating with CD)
#define calibrationFlowRate 14.4          // Standard flow rate for CD @ 10"/wg



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
#if defined DM
    #define BARO_SPARKFUN_BME280
    #define TEMP_SPARKFUN_BME280
    #define RELH_SPARKFUN_BME280
#endif