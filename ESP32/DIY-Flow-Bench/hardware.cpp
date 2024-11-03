/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file hardware.cpp
 * 
 * @brief Hardware class - integration of physical hardware devices
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

#ifdef ADC_IS_ENABLED
#include <ADS1115_lite.h>
ADS1115_lite adc(ADC_I2C_ADDR);
#endif




/***********************************************************
 * @brief CONSTRUCTOR
 ***/
Hardware::Hardware() {
  
}



/***********************************************************
 * @brief Configure pins
 * @note Conditional configiration based on board type and hardware
 *
 ***/
void Hardware::configurePins () {
 
  // Inputs
  #ifdef VCC_3V3_PIN
  pinMode(VCC_3V3_PIN, INPUT); 
  #endif

  pinMode(VCC_5V_PIN, INPUT);    //This is required as it is used to get the supply voltage for the MPVX7007 pressure calc.

  #ifdef SPEED_SENSOR_PIN
  pinMode(SPEED_SENSOR_PIN, INPUT);
  #endif

  #ifdef ORIFICE_BCD_BIT1_PIN
  pinMode(ORIFICE_BCD_BIT1_PIN, INPUT); 
  #endif

  #ifdef ORIFICE_BCD_BIT2_PIN
  pinMode(ORIFICE_BCD_BIT2_PIN, INPUT); 
  #endif

  #ifdef ORIFICE_BCD_BIT3_PIN
  pinMode(ORIFICE_BCD_BIT3_PIN, INPUT); 
  #endif

  #ifdef MAF_SRC_IS_PIN
  pinMode(MAF_PIN, INPUT); 
  #endif

  #ifdef PREF_SRC_PIN
  pinMode(REF_PRESSURE_PIN, INPUT); 
  #endif

  #ifdef PDIFF_SRC_IS_PIN
  pinMode(DIFF_PRESSURE_PIN, INPUT); 
  #endif

  #ifdef PITOT_SRC_IS_PIN
  pinMode(PITOT_PIN, INPUT); 
  #endif

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
  #ifdef VAC_BANK_1_PIN
  pinMode(VAC_BANK_1_PIN, OUTPUT);
  #endif

  #ifdef VAC_BANK_2_PIN
  pinMode(VAC_BANK_2_PIN, OUTPUT);
  #endif

  #ifdef VAC_BANK_3_PIN
  pinMode(VAC_BANK_3_PIN, OUTPUT);
  #endif

  #ifdef VAC_SPEED_PIN
  pinMode(VAC_SPEED_PIN, OUTPUT);
  #endif

  #ifdef VAC_BLEED_VALVE_PIN
  pinMode(VAC_BLEED_VALVE_PIN, OUTPUT);
  #endif

  #ifdef AVO_STEP_PIN
  pinMode(AVO_STEP_PIN, OUTPUT);
  #endif

  #ifdef AVO_DIR_PIN
  pinMode(AVO_DIR_PIN, OUTPUT);
  #endif

  #ifdef FLOW_VALVE_STEP_PIN
  pinMode(FLOW_VALVE_STEP_PIN, OUTPUT);
  #endif

  #ifdef FLOW_VALVE_DIR_PIN
  pinMode(FLOW_VALVE_DIR_PIN, OUTPUT);
  #endif

}

/***********************************************************
 * @brief begin function
 * 
 * TODO: Need to move ALL hardware initialisation into here
 **/
void Hardware::begin () {

  Messages _message;
  ConfigSettings config;

  extern struct DeviceStatus status;
  
  this->beginSerial();                                      // Start of serial comms
  this->initialise();                                       // Initialise hardware

  
}




/***********************************************************
 * @brief Initialise hardware
 ***/
void Hardware::initialise () {

  Messages _message;

  _message.serialPrintf("\r\nDIY Flow Bench \nVersion: %s \nBuild: %s \n", RELEASE, BUILD_NUMBER);    
  _message.serialPrintf("Initialising Hardware \n");

  configurePins(); // Load pin definitions from pins.h
  this->getI2CList(); // Scan and list I2C devices to serial monitor

  #ifdef ADC_IS_ENABLED
  _message.serialPrintf("Initialising ADS1115 \n");

  adc.setGain(ADS1115_REG_CONFIG_PGA_6_144V); // Set ADC Gain +/-6.144V range = Gain 2/3
  adc.setSampleRate(ADS1115_REG_CONFIG_DR_128SPS); // Set ADC Sample Rate - 128 SPS, or every 7.8ms  (default)
  // adc.setSampleRate(ADS1115_REG_CONFIG_DR_8SPS); // Set ADC Sample Rate - 128 SPS, or every 7.8ms  (default)
  
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
 * @brief Begin Serial
 *
 * @note Default port Serial0 (U0UXD) is used. (Same as programming port / usb)
 * @note Serial1 is reserved for SPI
 * @note Serial2 is reserved for gauge comms
 *
 * @note Serial.begin(baud-rate, protocol, RX pin, TX pin);
 *
 ***/
void Hardware::beginSerial(void) {
	
	#if defined SERIAL0_ENABLED
		Serial.begin(SERIAL0_BAUD, SERIAL_8N1 , SERIAL0_RX_PIN, SERIAL0_TX_PIN); 
	#endif
	
}




/***********************************************************
 * @brief Scan through I2C addresses and print list of devices found to serial
 * @remark Based on: https://www.esp32.com/viewtopic.php?t=4742
 * @note Wire is required but globally called in main setup in DIY-Flow-Bench.cpp
 ***/ 
void Hardware::getI2CList() {   
  
  Messages _message;

  _message.serialPrintf("Scanning for I2C devices...\n");
  byte count = 0;
  byte error;

  for (byte address = 1; address < 127; address++)   {
    Wire.beginTransmission (address);          
    // if (Wire.endTransmission () == 0)  {  
    error = Wire.endTransmission ();
    // _message.serialPrintf("Error: %u (0x%X)\n", error, address); // Uncomment to report all failed addresses
    if (error == 0)  {  
      _message.serialPrintf("Found address: %u (0x%X)\n", address, address);
      count++;
    }
  }
  
  _message.serialPrintf("Found %u device(s). \n", count);

}




/***********************************************************
 * @brief GET ADS1015 ADC value
 * @note uses ADC1115-lite library - https://github.com/terryjmyers/ADS1115-Lite
 *
 ***/
int32_t Hardware::getADCRawData(int channel) {

  int32_t rawADCval = 0;

  #ifdef ADC_IS_ENABLED

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

  #endif

  return rawADCval;

}




/***********************************************************
 * @brief Get ADC channel Voltage
 * @param channel ADC channel (0-3)
 * @note 1115: 16 bits less sign bit = 15 bits mantissa = 32767 | 6.144v = max voltage (gain) of ADC | 187.5 uV / LSB
 * @note 1015: 12 bits less sign bit = 11 bit mantissa = 2047 | 6.144v = max voltage (gain) of ADC
 ***/
 double Hardware::getADCVolts(int channel) {

  double volts;

  int rawADCval = getADCRawData(channel);
  
  #if defined ADC_TYPE_ADS1115 && defined ADC_IS_ENABLED 
    
    volts = rawADCval * ADC_GAIN / ADC_RANGE; 
  
  #elif defined ADC_TYPE_ADS1015 && defined ADC_IS_ENABLED 

    volts = rawADCval * ADC_GAIN / 2047.00F; 
  
  #else

    return 1.00F;

  #endif
  
  return volts;
}




/***********************************************************
 * @brief GET 5V SUPPLY VOLTAGE
 * @details Measures 5v supply buck power to ESP32 via voltage divider
 * @note We use a 10k-10k divider on the official shield. This gives a max of 2.5v which is fine for the ESP32's 3.3v logic
 * @note Use a 0.1uf cap on input to help filter noise
 *
 * NOTE: ESP32 has 12 bit ADC (0-3300mv = 0-4095)
 ***/
double Hardware::get5vSupplyVolts() {   

  long rawVoltageValue = analogRead(VCC_5V_PIN);  
  double vcc5vSupplyVolts = (2 * rawVoltageValue * 0.805860805860806) ;

  #ifdef USE_FIXED_5V_VALUE
    return 5.0; 
  #else
    return vcc5vSupplyVolts + VCC_5V_TRIMPOT;
  #endif
}




/***********************************************************
 * @brief GET 3.3V SUPPLY VOLTAGE
 * @details Measures 3.3v supply buck power to ESP32 via voltage divider
 * @note We use a 10k-10k divider on the official shield. This helps to capture voltages above +Vcc
 * @note Use a 0.1uf cap on input to help filter noise
 * @note ESP32 has 12 bit ADC (0-3300mv = 0-4095)
 ***/
double Hardware::get3v3SupplyVolts() {   

  #ifdef VCC_3V3_PIN  
  long rawVoltageValue = analogRead(VCC_3V3_PIN);  
  double vcc3v3SupplyVolts = (2 * rawVoltageValue * 0.805860805860806) ;
  #endif

  #ifdef USE_FIXED_3_3V_VALUE
    return 3.3; 
  #else
    return vcc3v3SupplyVolts + VCC_3V3_TRIMPOT;
  #endif
}





/***********************************************************
 * @brief BENCH IS RUNNING
 * @return bool:bench is running
 * @note used by calibration function in API.cpp
 ***/
bool Hardware::benchIsRunning() {
    
  Messages _message;
  Calculations _calculations;
  Sensors _sensors;
  
  extern struct ConfigSettings config;
  extern struct Translator translate;
  extern struct SensorData sensorVal;

  double refPressure;
  double mafFlowRateCFM; 
 
  // TODO: Check scope of these...
  refPressure = _calculations.convertPressure(sensorVal.PRefKPA, INH2O);
  mafFlowRateCFM = _calculations.convertFlow(_sensors.getMafFlow());

  // comvert negative value into posotive
  refPressure = fabs(refPressure); 

  if ((refPressure > config.min_bench_pressure))
  {
	  _message.Handler(translate.LANG_BENCH_RUNNING);
	  return true;
  } else {
    _message.Handler(translate.LANG_NO_ERROR);
	  return false;
  }
}


/***********************************************************
 * @brief CHECK REFERENCE PRESSURE
 * @details Check that reference pressure is within limits
 ***/
void Hardware::checkRefPressure() {
  
  Messages _message;
  Calculations _calculations;

  extern struct SensorData sensorVal;
  extern struct ConfigSettings config;
  extern struct Translator translate;
  
  double refPressure = _calculations.convertPressure(sensorVal.PRefKPA, INH2O);
    
  // REVIEW  - Ref pressure check
  // Check that pressure does not fall below limit set by MIN_TEST_PRESSURE_PERCENTAGE when bench is running
  // note alarm commented out in alarm function as 'nag' can get quite annoying
  // Is this a redundant check? Maybe a different alert would be more appropriate
  if ((refPressure < (config.cal_ref_press * (MIN_TEST_PRESSURE_PERCENTAGE / 100))) && (Hardware::benchIsRunning()))
  {
    _message.Handler(translate.LANG_REF_PRESS_LOW);
  }
}




/***********************************************************
 * @brief benchOn
 * @details Switches Vac Motor output on
 * 
 ***/
void Hardware::benchOn() {
  digitalWrite(VAC_BANK_1_PIN, HIGH);
}


/***********************************************************
 * @brief benchOff
 * @details Switches vac motor 1 output off
 ***/
void Hardware::benchOff() {
  digitalWrite(VAC_BANK_1_PIN, LOW);
}



/***********************************************************
 * @brief Uptime
 * @return double uptimeResponse in format hhhhhh.mm
 * @note uses ints to truncate fractional part of value
 ***/
float Hardware::uptime() {

  extern struct DeviceStatus status;
  
  int minutesUp;
  int hoursUp;
  float fractional = 0.0;
  float uptimeResponse = 0.0;

  minutesUp =  (millis() - status.boot_time) / 60000;
  hoursUp = minutesUp / 60;
  if (hoursUp >= 1) {
    minutesUp = minutesUp - (hoursUp * 60);
  }
  fractional = minutesUp / 100.00F;

  uptimeResponse = hoursUp + fractional;

  return uptimeResponse;

}


/***********************************************************
 * @brief Sets VFD reference voltage DAC1
 * @note need to utilise PID control to compare target depression with actual and adjust reference
 * @ref https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/dac.html
 ***/

void Hardware::setVFDRef() {
}


/***********************************************************
 * @brief Sets Bleed Valve reference voltage using DAC2
 * @note need to utilise PID control to compare target depression with actual and adjust reference
 * @ref https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/dac.html
 ***/
void Hardware::setBleedValveRef() {

}
