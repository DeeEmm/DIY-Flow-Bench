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
#define BUILD_NUMBER "21080701"
#define RELEASE "V.ESP.X-ALPHA"
#define DEV_BRANCH "https://github.com/DeeEmm/DIY-Flow-Bench/tree/ESP32"


/****************************************
 * SYSTEM SETTINGS
 ***/
#define showAlarms true
#define MIN_REFRESH_RATE 200
//#define DEBUG



/****************************************
 * DEFAULT USER SETTINGS
 *
 ***/

//#define CYCLIC_AVERAGE_BUFFER 5           // Number of scans over which to average output (helps stabilise results)
#define MIN_TEST_PRESSURE_PERCENTAGE 80      // Lowest test pressure bench will generate accurate results. Please see note in wiki
#define CONF_API_ENABLED                       // enable API
#define CONF_DISABLE_API_CHECKSUM              // Add checksum to serial API response TODO: UPDATE CHECKSUM TO NATIVE ESP32 CRC32



/****************************************
 * CONFIGURE FILESYSTEM
 ***/

#define FORMAT_FILESYSTEM_IF_FAILED true




/****************************************
 * SELECT BOARD TYPE 
 *
 * Default ESP32DUINO 
 ***/

//#define DIYFB_SHIELD                    
#define ESP32DUINO




/****************************************
 * CONFIGURE COMMUNICATIONS
 ***/

// (default 0x77) / Alternate 0x76
#define BME280_I2C_ADDR 0x77            
  



/****************************************
 * CONFIGURE MAF
 * Uncomment active MAF
 * If you want to modify the code to include additional MAF sensors
 * You will need to create your own MAF data file. Use exampleSensor.h as an example
 ***/
    
#include "mafData/ACDELCO_ 92281162.h" 
const char* MAF_SENSOR = "ACDELCO_92281162";

//#define MAF_SENSOR MH95-3000-100 
//const char* MAF_SENSOR = "MH95-3000-100";        // PMAS MH95-3000 in 100mm housing              

//#define MAF_SENSOR SIEMENS_5WK9605 
//const char* MAF_SENSOR = "SIEMENS_5WK9605";     // Data from Tonys tests

//#define MAF_SENSOR DELPHI_AF10118 
//const char* MAF_SENSOR = "DELPHI_AF10118";       // kg/hr - Data from efidynotuning.com/maf.htm (Stock - Ford '98 Explorer 5.0L)

//#define MAF_SENSOR exampleKeyValueData 
//const char* MAF_SENSOR = "exampleKeyValueData"; // Example key > value data file (duplicate this as required)

//#define MAF_SENSOR exampleAnalogData 
//const char* MAF_SENSOR = "exampleAnalogData";    // Example Analog point data file (duplicate this as required)

//#define MAF_SENSOR TEST 
//const char* MAF_SENSOR = "TEST";                 // Test Data



/****************************************
 * CONFIGURE REFERENCE PRESSURE SENSOR
 * If you want to modify the code to include additional reference pressure sensors
 * You will need to add your volts to kPa algorythm in the function getRefPressure()
 ***/
#define DEFAULT_REF_PRESS 1


#define PREF_SENSOR_NOT_USED 
const char* PREF_SENSOR = "Not Used";

//#define PREF_SENSOR_REF_MPXV7007 
//const char* PREF_SENSOR = "MPXV7007";        

//#define PREF_SENSOR_REF_MPX4250 
//const char* PREF_SENSOR = "PX4250";         



/****************************************
 * CONFIGURE PITOT PRESSURE SENSOR
 * If you want to modify the code to include additional pitot pressure sensors
 * You will need to add your volts to kPa algorythm in the function getPitotPressure()
 * Note Pitot sensors need to be a differential pressure sensor (DP)
 ***/

// Uncomment ONE of the following 

#define PITOT_SENSOR_NOT_USED 
const char* PITOT_SENSOR = "Not Used";

//#define PITOT_SENSOR_MPXV7007DP 
//const char* PITOT_SENSOR = "MPXV7007DP";

/****************************************
 * CONFIGURE BARO SENSOR
 *
 * Default Baro 101.3529kpa - standard sealevel baro pressure (14.7 psi) 
 ***/

#define BARO_SENSOR_FIXED_VALUE 
const char* BARO_SENSOR = "Fixed Value";

//#define BARO_SENSOR_REF_PRESS_AS_BARO 
//const char* BARO_SENSOR = "Ref Pressure";

//#define BARO_SENSOR_ADAFRUIT_BME280 
//const char* BARO_SENSOR = "ADAFRUIT_BME280";

//#define BARO_SENSOR_SPARKFUN_BME280 
//const char* BARO_SENSOR = "SPARKFUN_BME280";

//#define BARO_SENSOR_MPX4115 
//const char* BARO_SENSOR = "MPX4115";

#define DEFAULT_BARO 101.3529
#define startupBaroScalingFactor 1          // scaling factor when using reference pressure sensor for baro correction
#define startupBaroScalingOffset 100        // scaling offset when using reference pressure sensor for baro correction



/****************************************
 * CONFIGURE TEMPERATURE SENSOR
 *
 * Default 21 Degrees Celcius
 ***/
#define DEFAULT_TEMP 21


#define TEMP_SENSOR_FIXED_VALUE
const char* TEMP_SENSOR = "Fixed Value";

//#define TEMP_SENSOR_SPARKFUN_BME280 
//const char* TEMP_SENSOR = "SPARKFUN_BME280";

//#define TEMP_SENSOR_ADAFRUIT_BME280 
//const char* TEMP_SENSOR = "ADAFRUIT_BME280";

//#define TEMP_SENSOR_SIMPLE_TEMP_DHT11 
//const char* TEMP_SENSOR = "SIMPLE_TEMP_DHT11"; 



/****************************************
 * CONFIGURE HUMIDITY SENSOR
 *
 * Default 36% Rel H
 ***/
#define DEFAULT_RELH 36

// Uncomment ONE of the following
#define RELH_SENSOR_FIXED_VALUE 
const char* RELH_SENSOR = "Fixed Value";

//#define RELH_SENSOR_SPARKFUN_BME280 
//const char* RELH_SENSOR = "SPARKFUN_BME280";

//#define RELH_SENSOR_SIMPLE_RELH_DHT11
//const char* RELH_SENSOR = "SIMPLE_RELH_DHT11";

//#define RELH_SENSOR_ADAFRUIT_BME280 
//const char* RELH_SENSOR = "ADAFRUIT_BME280";




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
#if defined DEBUG
    #define BARO_SENSOR_SPARKFUN_BME280
    #define TEMP_SENSOR_SPARKFUN_BME280
    #define RELH_SENSOR_SPARKFUN_BME280
#endif