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

#include <ArduinoJson.h>

#include "system.h"
#include "configuration.h"
#include "constants.h"
#include "structs.h"
#include "datahandler.h"

#include <Wire.h>
#include "hardware.h"
#include "sensors.h"
#include "calculations.h"
#include "messages.h"
#include "system.h"


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
 * @note Conditional configuration based on board type and hardware
 * @details Initiates I/O from pin configuration in struct and reports to serial monitor
 * @details Messaging identifies I/O in event initialisation crash
 *
 ***/
void Hardware::initaliseIO () {

  Messages _message;

  extern struct Pins pins;
  extern struct DeviceStatus status;

  _message.serialPrintf("Initialising I/O \n");   



  // Set Inputs
  if (pins.VCC_3V3_PIN < 99 ) {
    _message.serialPrintf("Input VCC_3V3_PIN: %d\n", pins.VCC_3V3_PIN );
    pinMode(pins.VCC_3V3_PIN, INPUT);   
  }
  if (pins.VCC_5V_PIN < 99) {
    _message.serialPrintf("Input VCC_5V_PIN: %d\n", pins.VCC_5V_PIN );  
    pinMode(pins.VCC_5V_PIN, INPUT);
  }  
  if (pins.SPEED_SENSOR_PIN < 99 ) {
    _message.serialPrintf("Input SPEED_SENSOR_PIN: %d\n", pins.SPEED_SENSOR_PIN );
    pinMode(pins.SPEED_SENSOR_PIN, INPUT);   
  }
  if (pins.ORIFICE_BCD_BIT1_PIN < 99 ) {
    _message.serialPrintf("Input ORIFICE_BCD_BIT1_PIN: %d\n", pins.ORIFICE_BCD_BIT1_PIN );
    pinMode(pins.ORIFICE_BCD_BIT1_PIN, INPUT);   
  }
  if (pins.ORIFICE_BCD_BIT2_PIN < 99 ) {
    _message.serialPrintf("Input ORIFICE_BCD_BIT2_PIN: %d\n", pins.ORIFICE_BCD_BIT2_PIN );
    pinMode(pins.ORIFICE_BCD_BIT2_PIN, INPUT);   
  }
  if (pins.ORIFICE_BCD_BIT3_PIN < 99 ) {
    pinMode(pins.ORIFICE_BCD_BIT3_PIN, INPUT);   
    _message.serialPrintf("Input ORIFICE_BCD_BIT3_PIN: %d\n", pins.ORIFICE_BCD_BIT3_PIN );
    pinMode(pins.ORIFICE_BCD_BIT3_PIN, INPUT);   
  }
  #ifdef MAF_SRC_IS_PIN
  if ((pins.MAF_PIN < 99)  ) {
    _message.serialPrintf("Input MAF_PIN: %d\n", pins.MAF_PIN );
    pinMode(pins.MAF_PIN, INPUT);   
  }
  #endif
  #ifdef PREF_SRC_PIN
  if (pins.REF_PRESSURE_PIN < 99 ) {
    _message.serialPrintf("Input REF_PRESSURE_PIN: %d\n", pins.REF_PRESSURE_PIN );
    pinMode(pins.REF_PRESSURE_PIN, INPUT);   
  }
  #endif
  #ifdef PDIFF_SRC_IS_PIN
  if (pins.DIFF_PRESSURE_PIN < 99 ) {
    _message.serialPrintf("Input DIFF_PRESSURE_PIN: %d\n", pins.DIFF_PRESSURE_PIN );
    pinMode(pins.DIFF_PRESSURE_PIN, INPUT);   
  }
  #endif
  #ifdef PITOT_SRC_IS_PIN
  if (pins.PITOT_PIN < 99 ) {
    _message.serialPrintf("Input PITOT_PIN: %d\n", pins.PITOT_PIN );
    pinMode(pins.PITOT_PIN, INPUT);   
  }
  #endif
  #ifdef TEMP_SENSOR_TYPE_LINEAR_ANALOG
  if (pins.TEMPERATURE_PIN < 99 ) {
    _message.serialPrintf("Input TEMPERATURE_PIN: %d\n", pins.TEMPERATURE_PIN );
    pinMode(pins.TEMPERATURE_PIN, INPUT);   
  }
  #endif
  #ifdef RELH_SENSOR_TYPE_LINEAR_ANALOG
  if (pins.HUMIDITY_PIN < 99 ) {
    _message.serialPrintf("Input HUMIDITY_PIN: %d\n", pins.HUMIDITY_PIN );
    pinMode(pins.HUMIDITY_PIN, INPUT);   
  }
  #endif
  #ifdef BARO_SENSOR_TYPE_LINEAR_ANALOG
  if (pins.REF_BARO_PIN < 99 ) {
    _message.serialPrintf("Input REF_BARO_PIN: %d\n", pins.REF_BARO_PIN );
    pinMode(pins.REF_BARO_PIN, INPUT);     
  }
  #endif
  // if (pins.SERIAL0_RX_PIN < 99 ) {
  //   _message.serialPrintf("Input SERIAL0_RX_PIN: %d\n", pins.SERIAL0_RX_PIN );
  //   pinMode(pins.SERIAL0_RX_PIN, INPUT);   
  // }
  // if (pins.SERIAL2_RX_PIN < 99 ) {
  //   _message.serialPrintf("Input SERIAL2_RX_PIN: %d\n", pins.SERIAL2_RX_PIN );
  //   pinMode(pins.SERIAL2_RX_PIN, INPUT);   
  // }
   if (pins.SDA_PIN < 99 ) {
    _message.serialPrintf("Input SDA_PIN: %d\n", pins.SDA_PIN );
    pinMode(pins.SDA_PIN, INPUT);   
  }
  if (pins.SCL_PIN < 99 ) {
    _message.serialPrintf("Input SCL_PIN: %d\n", pins.SCL_PIN );
    pinMode(pins.SCL_PIN, INPUT);   
  }
  #ifdef SD_CARD_IS_ENABLED
  i  if (pins.SD_CS_PIN < 99 ) {
    _message.serialPrintf("Input SD_CS_PIN: %d\n", pins.SD_CS_PIN );
    pinMode(pins.SD_CS_PIN, INPUT);     
  }
  if (pins.SD_MISO_PIN < 99 ) {
    _message.serialPrintf("Input SD_MISO_PIN: %d\n", pins.SD_MISO_PIN );
    pinMode(pins.SD_MISO_PIN, INPUT);   
  }
  if (pins.SD_SCK_PIN < 99 ) {
    _message.serialPrintf("Input SD_SCK_PIN: %d\n", pins.SD_SCK_PIN );
    pinMode(pins.SD_SCK_PIN, INPUT);   
  }
  #endif
  // if (pins.WEMOS_SPARE_PIN_1 >= 0 ) {
  //   _message.serialPrintf("Input WEMOS_SPARE_PIN_1: %d\n", pins.WEMOS_SPARE_PIN_1 );
  //   pinMode(pins.WEMOS_SPARE_PIN_1, INPUT);   
  // }

  // // Set Outputs
  if (pins.VAC_BANK_1_PIN < 99 ) {
    _message.serialPrintf("Output VAC_BANK_1_PIN: %d\n", pins.VAC_BANK_1_PIN );
    pinMode(pins.VAC_BANK_1_PIN, OUTPUT);
  }
  if (pins.VAC_BANK_2_PIN < 99 ) {
    _message.serialPrintf("Output VAC_BANK_2_PIN: %d\n", pins.VAC_BANK_2_PIN );
    pinMode(pins.VAC_BANK_2_PIN, OUTPUT);
  }
  if (pins.VAC_BANK_3_PIN < 99 ) {
    _message.serialPrintf("Output VAC_BANK_3_PIN: %d\n", pins.VAC_BANK_3_PIN );
    pinMode(pins.VAC_BANK_3_PIN, OUTPUT);
  }
  if (pins.VAC_SPEED_PIN < 99 ) {
    _message.serialPrintf("Output VAC_SPEED_PIN: %d\n", pins.VAC_SPEED_PIN );
    pinMode(pins.VAC_SPEED_PIN, OUTPUT);
  }
  if (pins.VAC_BLEED_VALVE_PIN < 99 ) {
    _message.serialPrintf("Output VAC_BLEED_VALVE_PIN: %d\n", pins.VAC_BLEED_VALVE_PIN );
    pinMode(pins.VAC_BLEED_VALVE_PIN, OUTPUT);
  }
  if (pins.AVO_STEP_PIN < 99 ) {
    _message.serialPrintf("Output AVO_STEP_PIN: %d\n", pins.AVO_STEP_PIN );
    pinMode(pins.AVO_STEP_PIN, OUTPUT);
  }
  if (pins.AVO_DIR_PIN < 99 ) {
    _message.serialPrintf("Output AVO_DIR_PIN: %d\n", pins.AVO_DIR_PIN );
    pinMode(pins.AVO_DIR_PIN, OUTPUT);
  }
  if (pins.FLOW_VALVE_STEP_PIN < 99 ) {
    _message.serialPrintf("Output FLOW_VALVE_STEP_PIN: %d\n", pins.FLOW_VALVE_STEP_PIN );
    pinMode(pins.FLOW_VALVE_STEP_PIN, OUTPUT);
  }
  if (pins.FLOW_VALVE_DIR_PIN < 99 ) {
    _message.serialPrintf("Output FLOW_VALVE_DIR_PIN: %d\n", pins.FLOW_VALVE_DIR_PIN );
    pinMode(pins.FLOW_VALVE_DIR_PIN, OUTPUT);
  }
  #ifdef SD_CARD_IS_ENABLED
  if (pins.SD_MOSI_PIN < 99 ) {
    _message.serialPrintf("Output SD_MOSI_PIN: %d\n", pins.SD_MOSI_PIN );
    pinMode(pins.SD_MOSI_PIN, OUTPUT);
  }
  #endif
  // if (pins.SERIAL0_TX_PIN < 99 ) {
  //   _message.serialPrintf("Output SERIAL0_TX_PIN: %d\n", pins.SERIAL0_TX_PIN );
  //   pinMode(pins.SERIAL0_TX_PIN, OUTPUT);
  // }
  // if (pins.SERIAL2_TX_PIN < 99 ) {
  //   _message.serialPrintf("Output SERIAL2_TX_PIN: %d\n", pins.SERIAL2_TX_PIN );
  //   pinMode(pins.SERIAL2_TX_PIN, OUTPUT);
  // }

  _message.debugPrintf("I/O Initialised");

}



/***********************************************************
 * @name begin
 * @brief Hardware initialisation and set up
 * 
 **/
void Hardware::begin () {

  Messages _message;

  _message.serialPrintf("Initialising Hardware \n");

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
    if (error == 0)  {  
      _message.serialPrintf("Found address: %u (0x%X)\n", address, address);
      count++;
    } else {
      // _message.serialPrintf("Error: %u (0x%X)\n", error, address); // Uncomment to report all failed addresses
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

  extern struct Pins pins;
  extern struct ConfigSettings config;


  #ifdef USE_FIXED_5V_VALUE
    return 5.0; 
  #else
    long rawVoltageValue = analogRead(pins.VCC_5V_PIN);  
    double vcc5vSupplyVolts = (2 * rawVoltageValue * 0.805860805860806) ;
    return vcc5vSupplyVolts + config.VCC_5V_TRIMPOT;
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
  extern struct Language language;
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
	  _message.Handler(language.LANG_BENCH_RUNNING);
	  return true;
  } else {
    _message.Handler(language.LANG_NO_ERROR);
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
  extern struct Language language;
  
  double refPressure = _calculations.convertPressure(sensorVal.PRefKPA, INH2O);
    
  // REVIEW  - Ref pressure check
  // Check that pressure does not fall below limit set by MIN_TEST_PRESSURE_PERCENTAGE when bench is running
  // note alarm commented out in alarm function as 'nag' can get quite annoying
  // Is this a redundant check? Maybe a different alert would be more appropriate
  if ((refPressure < (config.cal_ref_press * (MIN_TEST_PRESSURE_PERCENTAGE / 100))) && (Hardware::benchIsRunning()))
  {
    _message.Handler(language.LANG_REF_PRESS_LOW);
  }
}




/***********************************************************
 * @brief benchOn
 * @details Switches Vac Motor output on
 * 
 ***/
void Hardware::benchOn() {

  extern struct Pins pins;

  digitalWrite(pins.VAC_BANK_1_PIN, HIGH);
//  digitalWrite(pins.VAC_BANK_2_PIN, HIGH);
//  digitalWrite(pins.VAC_BANK_3_PIN, HIGH);
}


/***********************************************************
 * @brief benchOff
 * @details Switches vac motor 1 output off
 ***/
void Hardware::benchOff() {

  extern struct Pins pins;

  digitalWrite(pins.VAC_BANK_1_PIN, LOW);
//  digitalWrite(pins.VAC_BANK_2_PIN, LOW);
//  digitalWrite(pins.VAC_BANK_3_PIN, LOW);
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





void Hardware::stepperTest() {

  extern struct DeviceStatus status;
  extern struct Pins pins;

  Messages _message;

  int numberOfSteps = 50;
  int pulseWidthMicros = 10;  // microseconds
  int millisbetweenSteps = 250; // milliseconds - or try 1000 for slower steps


  _message.serialPrintf("Testing Stepper\n");

  digitalWrite(pins.AVO_DIR_PIN, HIGH);
  for(int n = 0; n < numberOfSteps; n++) {
    digitalWrite(pins.AVO_STEP_PIN, HIGH);
    delayMicroseconds(pulseWidthMicros); // this line is probably unnecessary
    digitalWrite(pins.AVO_STEP_PIN, LOW);
    
    delay(millisbetweenSteps);
    
  }
  
  delay(1000);
  

  digitalWrite(pins.AVO_DIR_PIN, LOW);
  for(int n = 0; n < numberOfSteps; n++) {
    digitalWrite(pins.AVO_STEP_PIN, HIGH);
    delayMicroseconds(pulseWidthMicros); // probably not needed
    digitalWrite(pins.AVO_STEP_PIN, LOW);
    
    delay(millisbetweenSteps);
  }

  _message.serialPrintf("Stepper Test Finished\n");

}
