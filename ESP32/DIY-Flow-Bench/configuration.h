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

#define MAJOR_VERSION "V2"
#define MINOR_VERSION "0"
#define BUILD_NUMBER "21092201"
#define RELEASE "V.2.0-RC.2"
#define DEV_BRANCH "https://github.com/DeeEmm/DIY-Flow-Bench/tree/ESP32"


/***********************************************************
* SYSTEM SETTINGS
***/
#define BOOT_MESSAGE "May the flow be with you..."
#define PAGE_TITLE "DIY Flow Bench"
#define showAlarms true
#define MIN_REFRESH_RATE 200
#define LANGUAGE_FILE "language/EN_Language.h"



/***********************************************************
* What style of bench is this?
***/
#define MAF_STYLE_BENCH
//#define ORIFICE_STYLE_BENCH
//#define PITOT_STYLE_BENCH
//#define VENTURI_STYLE_BENCH


/***********************************************************
* What orifices are used?
* 
* TODO: Move into configuration data
*
* {diameter_in_mm, cfm_flow@15"}
***/
// double orificeData [][2] = {
//     {51.85,209.5},
//     {42.38,139.5},
//     {29.98,70.4},
//     {21.16,35.5},
//     {9.92,7.8}
// };



/***********************************************************
* DEFAULT USER SETTINGS
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

#define DIYFB_SHIELD                    
//#define ESP32DUINO



/***********************************************************
* TUNE HARDWARE 
***/

#define SUPPLY_MV_TRIMPOT 333                               // Measure and compare to serial monitor value           



/***********************************************************
* CONFIGURE BME280
***/

// (default 0x76) / Alternate 0x77
// Check report in serial monitor on boot for address
#define BME280_I2C_ADDR 0x76            
//#define BME280_I2C_ADDR 0x77            
  
  


/***********************************************************
* CONFIGURE ADC
*
* NOTE: standard shield can use adafruit 1015 or 1115 ADC
* MAF / PRef / PDiff / Pitot sensors all come through ADC
* This allows conversion from 5v to 3.3 via ADC > I2C
***/

// Default is 0x48
// Check report in serial monitor on boot for address
#define ADC_I2C_ADDR 0x48 

#define ADC_TYPE_ADS1015 // 12 bit (3 mV/bit)
// #define ADC_TYPE_ADS1115 // 16 bit (188uV/bit)




/***********************************************************
* CONFIGURE MAF
* Uncomment active MAF
* If you want to modify the code to include additional MAF sensors
* You will need to create your own MAF data file. Use exampleSensor.h as an example
***/
    
// Uncomment One sensor only
#define MAF_SENSOR_FILE "mafData/ACDELCO_92281162.h" 
// #define MAF_SENSOR_FILE "mafData/ACDELCO_19330122.h" 
// #define MAF_SENSOR_FILE "mafData/VDO_AFM_043.h"

// #define MAF_SENSOR_FILE  "MH95_3000_100"                 // PMAS MH95-3000 in 100mm housing              
// #define MAF_SENSOR_FILE  "mafData/SIEMENS_5WK9605"       // Data from Tonys tests
// #define MAF_SENSOR_FILE  "mafData/DELPHI_AF10118"        // kg/hr - Data from efidynotuning.com/maf.htm 

// #define MAF_SENSOR_FILE  "mafData/TEST"                  // Test Data


// Set signal source (Uncomment One line only)
//#define MAF_SRC_PIN
#define MAF_SRC_ADC


#define MAF_MV_TRIMPOT 0.0
#define MAF_ADC_CHANNEL 0


/***********************************************************
* CONFIGURE REFERENCE PRESSURE SENSOR
* If you want to modify the code to include additional reference pressure sensors
* You will need to add your volts to kPa algorithm in the function sensors->getPRef()
*
* Recommended sensor is the MPXV7007DP
***/
#define DEFAULT_REF_PRESS_VALUE 1                           // Fixed pressure value in Pascals

// Set signal source (Uncomment One line only)
//#define PREF_SRC_PIN
#define PREF_SRC_ADC

// Set sensor type (Uncomment One line only)
// #define PREF_SENSOR_NOT_USED
// #define PREF_SENSOR_TYPE_LINEAR_ANALOG 
#define PREF_SENSOR_TYPE_MPXV7007        


#define PREF_MV_TRIMPOT 0
#define PREF_ANALOG_SCALE 1.0                               // Scaling factor used for raw analog value
#define PREF_ADC_CHANNEL 2                                  // TODO: TEMP SWAPPED WITH PDIFF (ERROR ON PCB)





/***********************************************************
* CONFIGURE DIFFERENTIAL PRESSURE SENSOR
* If you want to modify the code to include additional reference pressure sensors
* You will need to add your volts to kPa algorithm in the function sensors->getPDiff()
* 
* Recommended sensor is the MPXV7007DP
***/
#define DEFAULT_DIFF_PRESS_VALUE 1                          // Fixed pressure value in Pascals

// Set signal source (Uncomment One line only)
//#define PDIFF_SRC_PIN
#define PDIFF_SRC_ADC

// Set sensor type (Uncomment One line only)
#define PDIFF_SENSOR_NOT_USED            
// #define PDIFF_SENSOR_TYPE_LINEAR_ANALOG 
// #define PDIFF_SENSOR_TYPE_MPXV7007          

#define PDIFF_MV_TRIMPOT 0
#define PDIFF_ANALOG_SCALE 1.0                               // Scaling factor used for raw analog value
#define PDIFF_ADC_CHANNEL 1                                  // TODO: TEMP SWAPPED WITH PREF (ERROR ON PCB)



/***********************************************************
* CONFIGURE PITOT PRESSURE SENSOR
* If you want to modify the code to include additional pitot pressure sensors
* You will need to add your volts to kPa algorithm in the function sensors->getPitot()
* Note Pitot sensors need to be a differential pressure sensor (DP)
*
* Recommended sensor is the MPXV7007DP
***/

// Set signal source (Uncomment One line only)
//#define PITOT_SRC_PIN
#define PITOT_SRC_ADC



// Set sensor type (Uncomment One line only)
// #define PITOT_SENSOR_NOT_USED
// #define PITOT_SENSOR_TYPE_LINEAR_ANALOG                   // Use analog signal from PITOT_PIN
#define PITOT_SENSOR_TYPE_MPXV7007DP

#define PITOT_MV_TRIMPOT 0.0
#define PITOT_ANALOG_SCALE 1.0                               // Scaling factor used for raw analog value
#define PITOT_ADC_CHANNEL 3


/***********************************************************
* CONFIGURE BARO SENSOR
*
* Default Baro 101.3529kpa - standard sea level baro pressure (14.7 psi) 
*
* Recommended sensor is the BME280
***/


// Uncomment One line only
// #define BARO_SENSOR_TYPE_FIXED_VALUE
// #define BARO_SENSOR_TYPE_LINEAR_ANALOG                   // Use analog signal from REF_BARO_PIN
#define BARO_SENSOR_TYPE_BME280
// #define BARO_SENSOR_TYPE_MPX4115

#define DEFAULT_BARO_VALUE 101.3529
#define BARO_ANALOG_SCALE 1.0                               // Scaling factor used for raw analog value
#define startupBaroScalingFactor 1                          // scaling factor when using reference pressure sensor for baro correction
#define startupBaroScalingOffset 100                        // scaling offset when using reference pressure sensor for baro correction

#define BARO_MV_TRIMPOT 0



/***********************************************************
* CONFIGURE TEMPERATURE SENSOR
*
* Recommended sensor is the BME280
***/
#define DEFAULT_TEMP_VALUE 21                               // Value to return if no sensor used
#define TEMP_ANALOG_SCALE 1.0                               // Scaling factor used for raw analog value

// Uncomment One line only
//#define TEMP_SENSOR_NOT_USED
// #define TEMP_SENSOR_TYPE_FIXED_VALUE
// #define TEMP_SENSOR_TYPE_LINEAR_ANALOG                   // Use analog signal from TEMPERATURE_PIN
#define TEMP_SENSOR_TYPE_BME280
// #define TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11

#define TEMP_MV_TRIMPOT 0
#define TEMP_ANALOG_SCALE 1.0                               // Scaling factor used for raw analog value



/***********************************************************
* CONFIGURE HUMIDITY SENSOR
*
* Recommended sensor is the BME280
***/
#define DEFAULT_RELH_VALUE 36                               // Value to return if no sensor used
#define RELH_ANALOG_SCALE 1.0                               // Scaling factor for raw analog value

// Uncomment ONE of the following
//#define RELH_SENSOR_NOT_USED
// #define RELH_SENSOR_TYPE_FIXED_VALUE
// #define RELH_SENSOR_TYPE_LINEAR_ANALOG                   // Use analog signal from HUMIDITY_PIN
#define RELH_SENSOR_TYPE_BME280
// #define RELH_SENSOR_TYPE_SIMPLE_RELH_DHT11

#define RELH_MV_TRIMPOT 0


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
#define calibrationPlateHighCFM 100                         // Flow rate for large calibration orifice
#define calibrationPlateMidCFM 50                           // Flow rate for med calibration orifice
#define calibrationPlateLowCFM 10                           // Flow rate for small calibration orifice

