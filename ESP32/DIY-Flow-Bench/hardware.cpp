/***********************************************************
 * The DIY Flow Bench project
 * https://diyflowbench.com
 * 
 * Hardware.cpp - Hardware class
 *
 * Open source flow bench project to measure and display volumetric air flow using an ESP32 / Arduino.
 * 
 * For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions 
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 * 
 ***/

#include <Arduino.h>
#include "hardware.h"
#include "configuration.h"
#include "constants.h"
#include "structs.h"
#include "pins.h"
#include "maths.h"
#include "messages.h"
#include LANGUAGE_FILE



/***********************************************************
* CONSTRUCTOR
*/
Hardware::Hardware() {
}



/***********************************************************
 * INITIALISE HARDWARE
 
 // TODO: Need to move ALL hardware initialisation into here
 
 ***/
 void Hardware::configurePins () {
   
    pinMode(VAC_BANK_1, OUTPUT);
 
 }


/***********************************************************
 * INITIALISE HARDWARE
 
 // TODO: Need to move ALL hardware initialisation into here
 
 ***/
 void Hardware::Initialise () {
 
  Messages _message;
  
  configurePins();
  

  // Support for ADAFRUIT_BME280 temp, pressure & Humidity sensors
  // https://github.com/adafruit/Adafruit_BME280_Library
  #if defined(PREF_SENSOR_REF_ADAFRUIT_BME280) || defined(TEMP_SENSOR_ADAFRUIT_BME280) || defined(BARO_SENSOR_ADAFRUIT_BME280)
    #include <Adafruit_BME280.h> 
    Adafruit_BME280 adafruitBme280; // Instantiate (create) a BMP280_DEV object and set-up for I2C operation (address 0x77)

    //I2C address - BME280_I2C_ADDR
    if (!adafruitBme280.begin()) {  
      _message.Handler(LANG_BME280_READ_FAIL);
      _message.DebugPrintLn("Adafruit BME280 Initialisation failed");      
    } else {
      _message.DebugPrintLn("Adafruit BME280 Initialised");      
    }
  #endif

  // Support for SPARKFUN_BME280 temp, pressure & Humidity sensors
  // https://learn.sparkfun.com/tutorials/sparkfun-bme280-breakout-hookup-guide?_ga=2.39864294.574007306.1596270790-134320310.1596270790
  #if defined (RELH_SENSOR_SPARKFUN_BME280) || defined(TEMP_SENSOR_SPARKFUN_BME280) || defined(BARO_SENSOR_SPARKFUN_BME280)
    #include "SparkFunBME280.h"
    #include <Wire.h>
    BME280 SparkFunBME280;

    Wire.begin();
    SparkFunBME280.setI2CAddress(BME280_I2C_ADDR); 
    if (SparkFunBME280.beginI2C() == false) //Begin communication over I2C
    {
      _message.Handler(LANG_BME280_READ_FAIL);
      _message.DebugPrintLn("Sparkfun BME280 Initialisation failed");      
    } else {
      _message.DebugPrintLn("Sparkfun BME280 Initialised");      
    }
  #endif

  // Support for DHT11 humidity / temperature sensors
  // https://github.com/winlinvip/SimpleDHT
  #if RELH_SENSOR_SIMPLE_RELH_DHT11 || TEMP_SENSOR_SIMPLE_TEMP_DHT11
    #include <SimpleDHT.h>  
    SimpleDHT11 dht11(HUMIDITY_PIN);    
  #endif


 }


/***********************************************************
 * GET BOARD VOLTAGE
 ***/
float Hardware::getSupplyMillivolts() {   
  int rawVoltageValue = analogRead(VOLTAGE_PIN);
  float supplyMillivolts = rawVoltageValue * (5.0 / 1024.0) * 1000;

  return supplyMillivolts;
}



/***********************************************************
* BENCH IS RUNNING
*/
bool Hardware::benchIsRunning() {
    
  Messages _message;
  Maths _maths;
  
  extern struct ConfigSettings config;
  
  // TODO: Check scope of these...
  float refPressure = _maths.calculateRefPressure(INWG);
  float mafFlowRateCFM = _maths.calculateMafFlowCFM();

  if ((refPressure > config.min_bench_pressure) && (mafFlowRateCFM > config.min_flow_rate))
  {
	  _message.Handler(LANG_BENCH_RUNNING);
	  return true;
  } else {
    _message.Handler(LANG_NO_ERROR);
	  return false;
  }
}


/***********************************************************
* CHECK REFERENCE PRESSURE
* Make sure that reference pressure is within limits
***/
void Hardware::checkRefPressure() {
  
  Messages _message;
  Maths _maths;
  
  extern struct ConfigSettings config;
  
  float refPressure = _maths.calculateRefPressure(INWG);
    
  // Check that pressure does not fall below limit set by MIN_TEST_PRESSURE_PERCENTAGE when bench is running
  // note alarm commented out in alarm function as 'nag' can get quite annoying
  // Is this a redundant check? Maybe a different alert would be more appropriate
  if ((refPressure < (config.cal_ref_press * (MIN_TEST_PRESSURE_PERCENTAGE / 100))) && (Hardware::benchIsRunning()))
  {
    _message.Handler(LANG_REF_PRESS_LOW);
  }
}