/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file hardware.cpp
 * 
 * @brief Hardware class
 * 
 * @remarks For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * @license This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
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
// #include <I2Cdev.h>

#ifdef ADC_IS_ENABLED
#include <ADS1115_lite.h>
ADS1115_lite adc(ADC_I2C_ADDR);
#endif

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
 * @brief begin method
 * 
 * TODO: Need to move ALL hardware initialisation into here
 **/
void Hardware::begin () {

  Messages _message;
  extern struct DeviceStatus status;
  
  this->beginSerial();                                      // Start of serial comms
  this->initialise();                                       // Initialise hardware

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
 * @brief Initialise hardware
*/
void Hardware::initialise () {

  Messages _message;

  _message.serialPrintf("Initialising Hardware \n");

  configurePins(); // Load pin definitions from pins.h
  this->getI2CList(); // Scan and list I2C devices to serial monitor

  #ifdef ADC_IS_ENABLED
  _message.serialPrintf("Initialising ADS1115 \n");

  adc.setGain(ADS1115_REG_CONFIG_PGA_6_144V); // Set ADC Gain +/-6.144V range = Gain 2/3
  adc.setSampleRate(ADS1115_REG_CONFIG_DR_128SPS); // Set ADC Sample Rate - 128 SPS, or every 7.8ms  (default)
  
  if (!adc.testConnection()) {
      _message.serialPrintf("ADS1115 Connection failed");
			while(1); //Freeze
		} else {
			_message.serialPrintf("ADS1115 Initialised\n");
  }
  #endif

  _message.serialPrintf("Hardware Initialised \n");


}




/***********************************************************
* Begin Serial
*
* Default port Serial0 (U0UXD) used. (Same as programming port / usb)
* NOTE: Serial1 reserved for SPI
* NOTE: Serial2 reserved for gauge comms
*
* Serial.begin(baud-rate, protocol, RX pin, TX pin);
*
* TODO:  Should we move into hardware? 
* Serial port is not specifically tied to messages and is also shared with API
*/
void Hardware::beginSerial(void) {
	
	#if defined SERIAL0_ENABLED
		Serial.begin(SERIAL0_BAUD, SERIAL_8N1 , SERIAL0_RX_PIN, SERIAL0_TX_PIN); 
	#endif
	
}



/***********************************************************
 * Get list of I2C devices 
 * @brief Loop through I2C addresses and print list of devices to serial
 * @remark Based on: https://www.esp32.com/viewtopic.php?t=4742
 * @note Wire is required but glbally called in main setup in DIY-Flow-Bench.cpp
 ***/ 
void Hardware::getI2CList() {   
  
  Messages _message;

  _message.serialPrintf("Scanning for I2C devices...\n");
  byte count = 0;
  
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



// /** Get operational status.
//  * from - https://github.com/jrowberg/i2cdevlib/blob/master/Arduino/ADS1115/ADS1115.cpp
//  * @return Current operational status (false for active conversion, true for inactive)
//  * @see ADS1115_RA_CONFIG
//  * @see ADS1115_CFG_OS_BIT
//  */
// bool Hardware::isADCConversionReady() {
//     // I2Cdev::readBit(ADC_I2C_ADDR, 0x01, 15, buffer);
//     return buffer[0];
// }

// /** Poll the operational status bit until the conversion is finished
//  * from - https://github.com/jrowberg/i2cdevlib/blob/master/Arduino/ADS1115/ADS1115.cpp
//  * Retry at most 'max_retries' times
//  * conversion is finished, then return true;
//  * @see ADS1115_CFG_OS_BIT
//  * @return True if data is available, false otherwise
//  */
// bool Hardware::pollADCConversion(uint16_t max_retries) {  
//   for(uint16_t i = 0; i < max_retries; i++) {
//     if (isADCConversionReady()) return true;
//   }
//   return false;
// }



/***********************************************************
* @brief GET ADS1015 ADC value
* @note uses ADC1115-lite library - https://github.com/terryjmyers/ADS1115-Lite
*
***/
int16_t Hardware::getADCRawData(int channel) {

int16_t rawADCval;

  if (channel > 3) {
    return 0;
  }

  switch (channel) // MUX - Multiplex channel
  {
    case (0):
        adc.setMux(ADS1115_REG_CONFIG_MUX_SINGLE_0); // 0x4000
    break;

    case (1):
        adc.setMux(ADS1115_REG_CONFIG_MUX_SINGLE_1); // 0x5000
    break;

    case (2):
        adc.setMux(ADS1115_REG_CONFIG_MUX_SINGLE_2); // 0x6000
    break;

    case (3):
        adc.setMux(ADS1115_REG_CONFIG_MUX_SINGLE_3); // 0x7000
    break;
  }
  
  adc.triggerConversion(); // Start a conversion. This immediately returns
  rawADCval = adc.getConversion(); // This polls the ADS1115 and wait for conversion to finish, THEN returns the value

  return rawADCval;

}






// DEPRECATED
/***********************************************************
 * @brief Get ADC millivolts
 * @param channel ADC channel (0-3)
 ***/
//  double Hardware::getADCMillivolts(int channel) {

//   double volts;
//   double millivolts;
//   const double ADC_GAIN = 6.144F;

//   int16_t rawVal = getADCRawData(channel);
  
//   #if defined ADC_TYPE_ADS1115 // 16 bit
//     // 16 bits - sign bit = 15 bits mantissa = 32767 | 6.144v = max voltage (gain) of ADC | 187.5 uV / LSB
//     volts = rawVal * ADC_GAIN / 32767.00; //32767.00F
//     millivolts = volts * 1000.00; //1000.00F
  
//   #elif defined ADC_TYPE_ADS1015 // 12 bit
//     // 12 bits - sign bit = 11 bit mantissa = 2047 | 6.144v = max voltage (gain) of ADC
//     volts = (rawVal * ADC_GAIN) / 2047; 
//     millivolts = volts * 1000;
  
//   #endif
  
//   return millivolts;
// }





/***********************************************************
 * @brief Get ADC channel Voltage
 * @param channel ADC channel (0-3)
 ***/
 double Hardware::getADCVolts(int channel) {

  double volts;
  const double ADC_GAIN = 6.144;

  int rawADCval = getADCRawData(channel);
  
  #if defined ADC_TYPE_ADS1115 && defined ADC_IS_ENABLED 
    // 16 bits - sign bit = 15 bits mantissa = 32767 | 6.144v = max voltage (gain) of ADC | 187.5 uV / LSB
    volts = double(rawADCval) * (ADC_GAIN / 32767.00); 
  
  #elif defined ADC_TYPE_ADS1015 && defined ADC_IS_ENABLED 
    // 12 bits - sign bit = 11 bit mantissa = 2047 | 6.144v = max voltage (gain) of ADC
    volts = double(rawADCval) * (ADC_GAIN / 2047); 
  
  #else

    return 1.0;

  #endif
  
  return volts;
}







/***********************************************************
* @brief GET 3.3V SUPPLY VOLTAGE
* Measures 3.3v supply buck power to ESP32 
* 3.3v buck connected directly to ESP32 input
* Use a 0.1uf cap on input to help filter noise
*
* NOTE: ESP32 has 12 bit ADC (0-3300mv = 0-4095)
***/
double Hardware::get3v3SupplyVolts() {   

  return 3.3; //REMOVE: DEBUG: temp test !!!

  // long rawVoltageValue = analogRead(VCC_3V3_PIN);  
  // double vcc3v3SupplyVolts = (rawVoltageValue * 0.805860805860806) ;
  // return vcc3v3SupplyVolts + VCC_3V3_TRIMPOT;
}


/***********************************************************
* @brief GET 5V SUPPLY VOLTAGE
* Measures 5v supply buck power to ESP32 via voltage divider
* We use a 10k-10k divider on the official shield
* This gives a max of 2.5v which is fine for the ESP32's 3.3v logic
* Use a 0.1uf cap on input to help filter noise
*
* NOTE: ESP32 has 12 bit ADC (0-3300mv = 0-4095)
***/
double Hardware::get5vSupplyVolts() {   

  return 5.0; //REMOVE: DEBUG: temp test !!!

  // long rawVoltageValue = analogRead(VCC_5V_PIN);  
  // double vcc5vSupplyVolts = (2 * rawVoltageValue * 0.805860805860806) ;
  // return vcc5vSupplyVolts + VCC_5V_TRIMPOT;
}





/***********************************************************
 * @brief BENCH IS RUNNING
 * @return bool:bench is running
*/
bool Hardware::benchIsRunning() {
    
  Messages _message;
  Calculations _calculations;
  Sensors _sensors;
  
  extern struct ConfigSettings config;
  extern struct Translator translate;
  extern struct SensorData sensorVal;
  
  // TODO: Check scope of these...
  double refPressure = _calculations.convertPressure(sensorVal.PRefKPA, INWG);
  
  double mafFlowRateCFM = _calculations.calculateFlowCFM(_sensors.getMafRaw());

  if ((refPressure > config.min_bench_pressure))
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

  extern struct SensorData sensorVal;
  extern struct ConfigSettings config;
  extern struct Translator translate;
  
  double refPressure = _calculations.convertPressure(sensorVal.PRefKPA, INWG);
    
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

