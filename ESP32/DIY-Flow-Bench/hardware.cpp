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

#include "configuration.h"
#include "constants.h"
#include "structs.h"
#include "pins.h"

#include <Wire.h>
#include "hardware.h"
#include "sensors.h"
#include "calculations.h"
#include "messages.h"
#include LANGUAGE_FILE

// NOTE: I2C code should be brought into hardware.cpp
// NOTE: If we replace I2Cdev with wire we need to rewrite the ADC Code
#include <I2Cdev.h>

/***********************************************************
* CONSTRUCTOR
*/
Hardware::Hardware() {
  
}



/***********************************************************
* Configure pins
*
***/
void Hardware::configurePins () {
 
  // Inputs
  //TODO
//  pinMode(MAF_PIN, INPUT); 
  pinMode(VCC_3V3_PIN, INPUT); 
  pinMode(VCC_5V_PIN, INPUT); 
  pinMode(SPEED_SENSOR_PIN, INPUT); 
//  pinMode(REF_PRESSURE_PIN, INPUT); 
//  pinMode(DIFF_PRESSURE_PIN, INPUT); 
//  pinMode(PITOT_PIN, INPUT); 
  
  #if defined TEMP_SENSOR_TYPE_LINEAR_ANALOG || defined TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11
    pinMode(TEMPERATURE_PIN, INPUT); 
  #endif
  #if defined RELH_SENSOR_TYPE_LINEAR_ANALOG || defined RELH_SENSOR_TYPE_SIMPLE_RELH_DHT11
    pinMode(HUMIDITY_PIN, INPUT);  
  #endif
  #if defined BARO_SENSOR_TYPE_LINEAR_ANALOG || defined BARO_SENSOR_TYPE_MPX4115
    pinMode(REF_BARO_PIN, INPUT);
  #endif
  

  // Outputs
  pinMode(VAC_BANK_1_PIN, OUTPUT);
  pinMode(VAC_BANK_2_PIN, OUTPUT);
  pinMode(VAC_BANK_3_PIN, OUTPUT);
  pinMode(VAC_SPEED_PIN, OUTPUT);
  pinMode(VAC_BLEED_VALVE_PIN, OUTPUT);
  pinMode(AVO_ENBL_PIN, OUTPUT);
  pinMode(AVO_STEP_PIN, OUTPUT);
  pinMode(AVO_DIR_PIN, OUTPUT);
  pinMode(VAC_BANK_1_PIN, OUTPUT);

  #ifdef ESP32_WROVER_KIT
    // Disable pins for use with JTAG Debugger (sets to high impedance)
    // pinMode(12, INPUT);
    // pinMode(13, INPUT);
    // pinMode(14, INPUT);
    // pinMode(15, INPUT);    
  #endif

}


/***********************************************************
* INITIALISE HARDWARE
* 
* TODO: Need to move ALL hardware initialisation into here
*
***/
void Hardware::begin () {

  Messages _message;

  extern struct DeviceStatus status;
  
  _message.serialPrintf("Initialising Hardware \n");
  
  configurePins();
  
  this->getI2CList(); // TODO: move into status dialog instead of printing to serial monitor
  
  _message.serialPrintf("Hardware Initialised \n");


  // Bench definitions for system status pane
  #if defined MAF_STYLE_BENCH
    status.benchType = "MAF Style";
  #elif defined ORIFICE_STYLE_BENCH
    status.benchType = "Orifice Style";
  #elif defined PITOT_STYLE_BENCH
    status.benchType = "Pitot Style";
  #elif defined VENTURI_STYLE_BENCH
    status.benchType = "Venturi Style";
  #endif

  // Board definitions for system status pane
  #if defined DIYFB_SHIELD                    
    status.boardType = "DIYFB Shield";
  #elif defined ESP32DUINO
    status.boardType = "ESP32DUINO";
  #elif defined ESP32_WROVER_KIT 
    status.boardType = "ESP32_WROVER_KIT";
  #endif
  
}



/***********************************************************
* Get list of I2C devices 
*
* Based on: https://www.esp32.com/viewtopic.php?t=4742
***/ 
void Hardware::getI2CList() {   
  
  Messages _message;

  Wire.begin (SCA_PIN, SCL_PIN); 

  _message.serialPrintf("Scanning for I2C devices...\n");
  byte count = 0;

  Wire.begin();
  
  for (byte i = 8; i < 120; i++)   {
    Wire.beginTransmission (i);          
    if (Wire.endTransmission () == 0)  {  
      _message.serialPrintf("Found address: %u (0x%X)\n", i, i);
      count++;
    }
  }
  
  _message.serialPrintf("Found %u device(s). \n", count);

}







/***********************************************************
* GET BME Data
* Fetch raw BME data
***/
void Hardware::getBMERawData() {
  
}



/** Get operational status.
 * from - https://github.com/jrowberg/i2cdevlib/blob/master/Arduino/ADS1115/ADS1115.cpp
 * @return Current operational status (false for active conversion, true for inactive)
 * @see ADS1115_RA_CONFIG
 * @see ADS1115_CFG_OS_BIT
 */
bool Hardware::isADCConversionReady() {
    I2Cdev::readBit(ADC_I2C_ADDR, 0x01, 15, buffer);
    return buffer[0];
}

/** Poll the operational status bit until the conversion is finished
 * from - https://github.com/jrowberg/i2cdevlib/blob/master/Arduino/ADS1115/ADS1115.cpp
 * Retry at most 'max_retries' times
 * conversion is finished, then return true;
 * @see ADS1115_CFG_OS_BIT
 * @return True if data is available, false otherwise
 */
bool Hardware::pollADCConversion(uint16_t max_retries) {  
  for(uint16_t i = 0; i < max_retries; i++) {
    if (isADCConversionReady()) return true;
  }
  return false;
}



/***********************************************************
* GET ADS1015 ADC value
*
* Based on: 
* https://www.ti.com/lit/ds/symlink/ads1115.pdf
*
* Bitwise gain values (in hex)
* 0x0000: +/- 6.144V (187.5uV / bit)
* 0x0200: +/- 4.096V (125uV / bit)
* 0x0400: +/- 2.048V (62.5uV / bit)
* 0x0600: +/- 1.024V (31.25uV / bit)
* 0x0800: +/- 0.512V (15.525uV / bit)
* 0x0A00: +/- 0.256V (7.8125uV / bit)
*
***/
int16_t Hardware::getADCRawData(int channel) {

  if (channel > 3) {
    return 0;
  }

  if (pollADCConversion(ADC_MAX_RETRIES) == true) {

    // Prepare config
    _config = 0x0003;  // Disable the comparator (default val)
    _config |= 0x0110; // RATE - Data rate or Sample Per Seconds bits 5 through 7
    _config |= 0x0100; // MODE - mode bit 8 - Single-shot mode (default) 
    _config |= 0x0000; // GAIN - PGA/voltage gain bits 9 through 11 
    
    switch (channel) // MUX - Multiplex channel, bits 12 through 14
    {
      case (0):
          _config |= 0x4000; // Channel 1
          break;
      case (1):
          _config |= 0x5000; // Channel 2
          break;
      case (2):
          _config |= 0x6000; // Channel 3
          break;
      case (3):
          _config |= 0x7000; // Channel 4
          break;
    }
    
    _config |= 0x8000; // WRITE: Set to start a single-conversion
    
    _config >>= 8; // Bit shift
    _config &= 0xFF; // Bit mask
    
    I2Cdev::writeWord(ADC_I2C_ADDR, 0x01, _config);
    
    //delay(10); // TODO: DELAYS are a hack, need to test for transmission complete. (Delays are bad Mmmmnnkay) 
    
    I2Cdev::readBytes(ADC_I2C_ADDR, 0x00 , 2, buffer);
    rawADCval = buffer[0] + (buffer[1] << 8);
  
  }
    
  #ifdef ADC_TYPE_ADS1015
    rawADCval >>= 4;  // Bit shift 12-bit results
  #endif
  
  return int16_t(rawADCval);
  
}







/***********************************************************
 * Get ADC millivolts
 ***/
 double Hardware::getADCMillivolts(int channel) {

  int16_t rawVal;
  double millivolts;
  double microvolts;
  
  rawVal = getADCRawData(channel);
  
  #if defined ADC_TYPE_ADS1115 // 16 bit
    // 16 bits - sign bit = 15 bits mantissa = 32767 | 6.144v = max voltage (gain) of ADC | 187.5 uV / LSB
    // millivolts = ((rawVal * (6.144 / 32768)) / 1000); 
    microvolts = (rawVal * 6.144) / 32768;
    millivolts = microvolts / 1000;
  
  #elif defined ADC_TYPE_ADS1015 // 12 bit
    // 12 bits - sign bit = 11 bit mantissa = 2047 | 6.144v = max voltage (gain) of ADC
    // millivolts = ((rawVal * (6.144 / 2047)) / 1000); 
    microvolts = (rawVal * 6.144) / 2047; 
    millivolts = microvolts / 1000;
  
  #endif
  
  return millivolts;
}



/***********************************************************
* GET 3.3V SUPPLY VOLTAGE
* Measures 3.3v supply buck power to ESP32 
* 3.3v buck connected directly to ESP32 input
* Use a 0.1uf cap on input to help filter noise
*
* NOTE: ESP32 has 12 bit ADC (0-3300mv = 0-4095)
***/
double Hardware::get3v3SupplyMillivolts() {   

  return 3300; //REMOVE: DEBUG: temp test !!!

  // long rawVoltageValue = analogRead(VCC_3V3_PIN);  
  // double vcc3v3SupplyMillivolts = (rawVoltageValue * 0.805860805860806) ;
  // return vcc3v3SupplyMillivolts + VCC_3V3_TRIMPOT;
}


/***********************************************************
* GET 5V SUPPLY VOLTAGE
* Measures 5v supply buck power to ESP32 via voltage divider
* We use a 10k-10k divider on the official shield
* This gives a max of 2.5v which is fine for the ESP32's 3.3v logic
* Use a 0.1uf cap on input to help filter noise
*
* NOTE: ESP32 has 12 bit ADC (0-3300mv = 0-4095)
***/
double Hardware::get5vSupplyMillivolts() {   

  return 5000; //REMOVE: DEBUG: temp test !!!

  // long rawVoltageValue = analogRead(VCC_5V_PIN);  
  // double vcc5vSupplyMillivolts = (2 * rawVoltageValue * 0.805860805860806) ;
  // return vcc5vSupplyMillivolts + VCC_5V_TRIMPOT;
}



/***********************************************************
* BENCH IS RUNNING
*/
bool Hardware::benchIsRunning() {
    
  Messages _message;
  Calculations _calculations;
  Sensors _sensors;
  
  extern struct ConfigSettings config;
  extern struct Translator translate;
  
  // TODO: Check scope of these...
  double refPressure = _calculations.convertPressure(_sensors.getPRefValue(), INWG);
  
  double mafFlowRateCFM = _calculations.calculateFlowCFM();

  if ((refPressure > config.min_bench_pressure) && (mafFlowRateCFM > config.min_flow_rate))
  {
	  _message.Handler(translate.LANG_VAL_BENCH_RUNNING);
	  return true;
  } else {
    _message.Handler(translate.LANG_VAL_NO_ERROR);
	  return false;
  }
}


/***********************************************************
* CHECK REFERENCE PRESSURE
* Make sure that reference pressure is within limits
***/
void Hardware::checkRefPressure() {
  
  Messages _message;
  Calculations _calculations;
  Sensors _sensors;
  
  extern struct ConfigSettings config;
  extern struct Translator translate;
  
  double refPressure = _calculations.convertPressure(_sensors.getPRefValue(), INWG);
    
  // Check that pressure does not fall below limit set by MIN_TEST_PRESSURE_PERCENTAGE when bench is running
  // note alarm commented out in alarm function as 'nag' can get quite annoying
  // Is this a redundant check? Maybe a different alert would be more appropriate
  if ((refPressure < (config.cal_ref_press * (MIN_TEST_PRESSURE_PERCENTAGE / 100))) && (Hardware::benchIsRunning()))
  {
    _message.Handler(translate.LANG_VAL_REF_PRESS_LOW);
  }
}




/***********************************************************
* benchOn
*
*/
void Hardware::benchOn() {
  digitalWrite(VAC_BANK_1_PIN, HIGH);
}


/***********************************************************
* benchOff
*
*/
void Hardware::benchOff() {
  digitalWrite(VAC_BANK_1_PIN, LOW);
}

