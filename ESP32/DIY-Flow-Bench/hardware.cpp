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
#include <Wire.h>
#include "Preferences.h"

#include "system.h"
#include "constants.h"
#include "structs.h"
#include "datahandler.h"

#include "hardware.h"
#include "sensors.h"
#include "calculations.h"
#include "messages.h"
#include "system.h"

#include <ADS1115_lite.h>

// Kludge to fix ADC2 + WiFi - Source: https://forum.arduino.cc/t/interesting-esp32-issue-cant-use-analogread-in-esp_wifimanager-library/679348/2
#include "soc/sens_reg.h"    // needed for manipulating ADC2 control register
uint64_t reg_b;              // Used to store ADC2 control register
int sensorValue = 0;         // variable to store the value coming from the sensor

ADS1115_lite adc(ADS1115_DEFAULT_ADDRESS);

/***********************************************************
 * @brief CONSTRUCTOR
 ***/
Hardware::Hardware() {
}

// /***********************************************************
//  * @brief save_ADC_Reg - Kludge to fix ADC2 + WiFi 
//  * @details  Save ADC2 control register value : Do this before begin Wifi/Bluetooth
//  * @note Source: https://forum.arduino.cc/t/interesting-esp32-issue-cant-use-analogread-in-esp_wifimanager-library/679348/2
//  ***/
// void Hardware::save_ADC_Reg(void)
// {
//   reg_b = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);
// }

// /***********************************************************
//  * @brief restore_ADC_Reg - Kludge to fix ADC2 + WiFi 
//  * @details  Restore ADC2 control register value : Do this after Wifi/Bluetooth
//  * @note Source: https://forum.arduino.cc/t/interesting-esp32-issue-cant-use-analogread-in-esp_wifimanager-library/679348/2
//  ***/
// void Hardware::restore_ADC_Reg(void)
// { 
//   WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, reg_b);
//   SET_PERI_REG_MASK(SENS_SAR_READ_CTRL2_REG, SENS_SAR2_DATA_INV);
// }

// /***********************************************************
//  * @brief readAnalog - Kludge to fix ADC2 + WiFi 
//  * @details Read Analog pin value 
//  * @note Source: https://forum.arduino.cc/t/interesting-esp32-issue-cant-use-analogread-in-esp_wifimanager-library/679348/2
//  ***/
// long Hardware::readAnalog(int sensorPin)
// { 
//   restore_ADC_Reg();
//   sensorValue = analogRead(sensorPin);
//   // Serial.print("Pin " + String(sensorPin) + " = ");
//   // Serial.println(sensorValue);

//   return sensorValue;

// }

/***********************************************************
 * @name begin
 * @brief Hardware initialisation and set up
 * 
 **/
void Hardware::begin () {

  extern struct Configuration config;

  Hardware _hardware;
  Messages _message;

  _message.serialPrintf("Initialising Hardware \n");


  this->getI2CList(); // Scan and print I2C device list to serial monitor

  if (config.iADC_TYPE != SENSOR_DISABLED) {

    _message.serialPrintf("Initialising ADS1115 ( Address: %u ) \n", config.iADC_I2C_ADDR);

    adc.setGain(ADS1115_REG_CONFIG_PGA_6_144V); // Set ADC Gain +/-6.144V range = Gain 2/3
    adc.setSampleRate(ADS1115_REG_CONFIG_DR_8SPS); // Set ADC Sample Rate - 8 SPS
    
    if (!adc.testConnection()) {
      _message.serialPrintf("ADS1115 Connection failed");
      while(1); // Freeze
    } else {
      _message.serialPrintf("ADS1115 Initialised\n");
    }
  }

  _message.serialPrintf("Hardware Initialised \n");

}





/***********************************************************
* @brief resetPins
* @details reset pins settings in NVM
***/ 
void Hardware::resetPins () {

  Messages _message;
  Preferences _prefs;

  _prefs.begin("pins");
  _prefs.clear();
  _prefs.end();

  initialisePins();

}







/***********************************************************
* @brief initalisePins
* @details initialise pins settings in NVM
* @note Preferences Key can not exceed 15 chars long
* @note Default pins schema is for official WEMOS D1 R32 style board
***/ 
void Hardware::initialisePins () {

  Messages _message;
  Preferences _prefs;
  Pins pins;

  _prefs.begin("pins");

  if (_prefs.isKey("SERIAL2_TX")) { // we've already initialised _prefs
    _prefs.end();
    return;
  }

  _message.serialPrintf("Initialising Pins \n");    

  if (!_prefs.isKey("VCC_5V")) _prefs.putInt("VCC_5V", pins.VCC_5V);
  if (!_prefs.isKey("VCC_3V3")) _prefs.putInt("VCC_3V3", pins.VCC_3V3);
  if (!_prefs.isKey("SPEED_SENS")) _prefs.putInt("SPEED_SENS", pins.SPEED_SENS);
  if (!_prefs.isKey("ORIFICE_BCD_1")) _prefs.putInt("ORIFICE_BCD_1", pins.ORIFICE_BCD_1);
  if (!_prefs.isKey("ORIFICE_BCD_2")) _prefs.putInt("ORIFICE_BCD_2", pins.ORIFICE_BCD_2);
  if (!_prefs.isKey("ORIFICE_BCD_3")) _prefs.putInt("ORIFICE_BCD_3", pins.ORIFICE_BCD_3);
  if (!_prefs.isKey("MAF")) _prefs.putInt("MAF", pins.MAF);
  if (!_prefs.isKey("PREF")) _prefs.putInt("PREF", pins.PREF);
  if (!_prefs.isKey("PDIFF")) _prefs.putInt("PDIFF", pins.PDIFF);
  if (!_prefs.isKey("PITOT")) _prefs.putInt("PITOT", pins.PITOT);
  if (!_prefs.isKey("TEMPERATURE")) _prefs.putInt("TEMPERATURE", pins.TEMPERATURE);
  if (!_prefs.isKey("HUMIDITY")) _prefs.putInt("HUMIDITY", pins.HUMIDITY);
  if (!_prefs.isKey("REF_BARO")) _prefs.putInt("REF_BARO", pins.REF_BARO);
  if (!_prefs.isKey("SWIRL_ENCODER_A")) _prefs.putInt("SWIRL_ENCODER_A", pins.SWIRL_ENCODER_A);
  if (!_prefs.isKey("SWIRL_ENCODER_B")) _prefs.putInt("SWIRL_ENCODER_B", pins.SWIRL_ENCODER_B);
  if (!_prefs.isKey("SERIAL0_RX")) _prefs.putInt("SERIAL0_RX", pins.SERIAL0_RX);
  if (!_prefs.isKey("SERIAL2_RX")) _prefs.putInt("SERIAL2_RX", pins.SERIAL2_RX);
  if (!_prefs.isKey("SDA")) _prefs.putInt("SDA", pins.SDA);
  if (!_prefs.isKey("SCL")) _prefs.putInt("SCL", pins.SCL);
  if (!_prefs.isKey("SD_CS")) _prefs.putInt("SD_CS", pins.SD_CS);
  if (!_prefs.isKey("SD_MISO")) _prefs.putInt("SD_MISO", pins.SD_MISO);
  if (!_prefs.isKey("SD_SCK")) _prefs.putInt("SD_SCK", pins.SD_SCK);
  if (!_prefs.isKey("SPARE_PIN_1")) _prefs.putInt("SPARE_PIN_1", pins.SPARE_PIN_1);
  if (!_prefs.isKey("SPARE_PIN_2")) _prefs.putInt("SPARE_PIN_2", pins.SPARE_PIN_2);

  // Store output pin values in struct
  if (!_prefs.isKey("VAC_SPEED")) _prefs.putInt("VAC_SPEED", pins.VAC_SPEED); 
  if (!_prefs.isKey("VAC_BANK_1")) _prefs.putInt("VAC_BANK_1", pins.VAC_BANK_1);
  if (!_prefs.isKey("VAC_BANK_2")) _prefs.putInt("VAC_BANK_2", pins.VAC_BANK_2);
  if (!_prefs.isKey("VAC_BANK_3")) _prefs.putInt("VAC_BANK_3", pins.VAC_BANK_3);
  if (!_prefs.isKey("VAC_BLEED_VALVE")) _prefs.putInt("VAC_BLEED_VALVE", pins.VAC_BLEED_VALVE);
  if (!_prefs.isKey("AVO_STEP")) _prefs.putInt("AVO_STEP", pins.AVO_STEP);
  if (!_prefs.isKey("AVO_DIR")) _prefs.putInt("AVO_DIR", pins.AVO_DIR);
  if (!_prefs.isKey("FLOW_VALVE_STEP")) _prefs.putInt("FLOW_VALVE_STEP", pins.FLOW_VALVE_STEP);
  if (!_prefs.isKey("FLOW_VALVE_DIR")) _prefs.putInt("FLOW_VALVE_DIR", pins.FLOW_VALVE_DIR);
  if (!_prefs.isKey("SD_MOSI")) _prefs.putInt("SD_MOSI", pins.SD_MOSI);
  if (!_prefs.isKey("SERIAL0_TX")) _prefs.putInt("SERIAL0_TX", pins.SERIAL0_TX);
  if (!_prefs.isKey("SERIAL2_TX")) _prefs.putInt("SERIAL2_TX", pins.SERIAL2_TX);
  
  _prefs.end();

}











/***********************************************************
* @name loadPinsData
* @brief Read pins data from NVM
***/
void Hardware::loadPinsData () {

  DataHandler _data;
  Messages _message;
  Hardware _hardware;
  Preferences _prefs;

  extern struct DeviceStatus status;
  extern struct Pins pins;

  _message.serialPrintf("Loading Pins Data \n");     

  _prefs.begin("pins");

  // Load pin values into struct
  pins.VCC_3V3 = _prefs.getInt("VCC_3V3");
  pins.VCC_5V = _prefs.getInt("VCC_5V");
  pins.SPEED_SENS = _prefs.getInt("SPEED_SENS");
  pins.ORIFICE_BCD_1 = _prefs.getInt("ORIFICE_BCD_1");
  pins.ORIFICE_BCD_2 = _prefs.getInt("ORIFICE_BCD_2");
  pins.ORIFICE_BCD_3 = _prefs.getInt("ORIFICE_BCD_3");
  pins.MAF = _prefs.getInt("MAF");
  pins.PREF = _prefs.getInt("PREF");
  pins.PDIFF = _prefs.getInt("PDIFF");
  pins.PITOT = _prefs.getInt("PITOT");
  pins.TEMPERATURE = _prefs.getInt("TEMPERATURE");
  pins.HUMIDITY = _prefs.getInt("HUMIDITY");
  pins.REF_BARO = _prefs.getInt("REF_BARO");
  pins.SWIRL_ENCODER_A = _prefs.getInt("SWIRL_ENCODER_A");
  pins.SWIRL_ENCODER_B = _prefs.getInt("SWIRL_ENCODER_B");
  pins.SERIAL0_RX = _prefs.getInt("SERIAL0_RX");
  pins.SERIAL2_RX = _prefs.getInt("SERIAL2_RX");
  pins.SDA = _prefs.getInt("SDA");
  pins.SCL = _prefs.getInt("SCL");
  pins.SD_CS = _prefs.getInt("SD_CS");
  pins.SD_MISO = _prefs.getInt("SD_MISO");
  pins.SD_SCK = _prefs.getInt("SD_SCK");
  pins.SPARE_PIN_1 = _prefs.getInt("SPARE_PIN_1");
  pins.SPARE_PIN_2 = _prefs.getInt("SPARE_PIN_2");

  // Store output pin values in struct
  pins.VAC_SPEED = _prefs.getInt("VAC_SPEED");
  pins.VAC_BANK_1 = _prefs.getInt("VAC_BANK_1");
  pins.VAC_BANK_2 = _prefs.getInt("VAC_BANK_2");
  pins.VAC_BANK_3 = _prefs.getInt("VAC_BANK_3");
  pins.VAC_BLEED_VALVE = _prefs.getInt("VAC_BLEED_VALVE");
  pins.AVO_STEP = _prefs.getInt("AVO_STEP");
  pins.AVO_DIR = _prefs.getInt("AVO_DIR");
  pins.FLOW_VALVE_STEP = _prefs.getInt("FLOW_VALVE_STEP");
  pins.FLOW_VALVE_DIR = _prefs.getInt("FLOW_VALVE_DIR");
  pins.SD_MOSI = _prefs.getInt("SD_MOSI");
  pins.SERIAL0_TX = _prefs.getInt("SERIAL0_TX");
  pins.SERIAL2_TX = _prefs.getInt("SERIAL2_TX");

  status.nvmPins = _prefs.freeEntries();
  _message.debugPrintf("Pins NVM Free Entries: %u \n", status.nvmPins); 

  _prefs.end();

  status.pinsLoaded = true;

}










/***********************************************************
 * @brief Configure pins
 * @note Conditional configuration based on board type and hardware
 * @details Initiates I/O from pin configuration in struct and reports to serial monitor
 * @details Messaging identifies I/O in event initialisation crash
 * @return bool
 *
 ***/
int Hardware::setPinMode () {

  Messages _message;

  extern struct Pins pins;
  extern struct DeviceStatus status;
  extern struct Configuration config;

  int currentPin = status.ioError;

  _message.serialPrintf("Initialising I/O \n");   

  // if (config.iADC_TYPE != SENSOR_DISABLED){
  //   ADS1115_lite adc(config.iADC_I2C_ADDR);
  // }



  try {
    // Set Inputs
    if (pins.VCC_3V3 > -1 ) {
      _message.verbosePrintf("Input VCC_3V3: %d\n", pins.VCC_3V3 );
      currentPin = pins.VCC_3V3 ;
      pinMode(pins.VCC_3V3, INPUT);   
    }
    if (pins.VCC_5V > -1) {
      _message.verbosePrintf("Input VCC_5V: %d\n", pins.VCC_5V );  
      currentPin = pins.VCC_5V;
      pinMode(pins.VCC_5V, INPUT);
    }  
    if (pins.SPEED_SENS > -1 ) {
      _message.verbosePrintf("Input SPEED_SENS: %d\n", pins.SPEED_SENS );
      currentPin = pins.SPEED_SENS;
      pinMode(pins.SPEED_SENS, INPUT);   
    }
    if (pins.ORIFICE_BCD_1 > -1 ) {
      _message.verbosePrintf("Input ORIFICE_BCD_1: %d\n", pins.ORIFICE_BCD_1 );
      currentPin = pins.ORIFICE_BCD_1;
      pinMode(pins.ORIFICE_BCD_1, INPUT);   
    }
    if (pins.ORIFICE_BCD_2 > -1 ) {
      _message.verbosePrintf("Input ORIFICE_BCD_2: %d\n", pins.ORIFICE_BCD_2 );
      pinMode(pins.ORIFICE_BCD_2, INPUT);   
      currentPin = pins.ORIFICE_BCD_2;
      pinMode(pins.ORIFICE_BCD_2, INPUT);   
    }
    if (pins.ORIFICE_BCD_3 > -1 ) {
      pinMode(pins.ORIFICE_BCD_3, INPUT);   
      _message.verbosePrintf("Input ORIFICE_BCD_3: %d\n", pins.ORIFICE_BCD_3 );
      currentPin = pins.ORIFICE_BCD_3;
      pinMode(pins.ORIFICE_BCD_3, INPUT);   
    }
    if (config.iMAF_SRC_TYP == LINEAR_ANALOG && pins.MAF > -1) {
      _message.verbosePrintf("Input MAF_BENCH: %d\n", pins.MAF );
      currentPin = pins.MAF;
      pinMode(pins.MAF, INPUT);   
    }
    if (config.iPREF_SENS_TYP == LINEAR_ANALOG && pins.PREF > -1){
      _message.verbosePrintf("Input PREF: %d\n", pins.PREF );
      currentPin = pins.PREF;
      pinMode(pins.PREF, INPUT);   
    }
    if (config.iPDIFF_SENS_TYP == LINEAR_ANALOG && pins.PDIFF > -1) {
      _message.verbosePrintf("Input PDIFF: %d\n", pins.PDIFF );
      currentPin = pins.PDIFF;
      pinMode(pins.PDIFF, INPUT);   
    }
    if (config.iPITOT_SENS_TYP  == LINEAR_ANALOG && pins.PITOT > -1) {
      _message.verbosePrintf("Input PITOT_BENCH: %d\n", pins.PITOT );
      currentPin = pins.PITOT;
      pinMode(pins.PITOT, INPUT);   
    }
    if (config.iTEMP_SENS_TYP == LINEAR_ANALOG && pins.TEMPERATURE > -1) {
      _message.verbosePrintf("Input TEMPERATURE: %d\n", pins.TEMPERATURE );
      currentPin = pins.TEMPERATURE;
      pinMode(pins.TEMPERATURE, INPUT);   
    }
    if (config.iRELH_SENS_TYP == LINEAR_ANALOG && pins.HUMIDITY > -1 ){
      _message.verbosePrintf("Input HUMIDITY: %d\n", pins.HUMIDITY );
      currentPin = pins.HUMIDITY;
      pinMode(pins.HUMIDITY, INPUT);   
    }
    if (config.iBARO_SENS_TYP == LINEAR_ANALOG && pins.REF_BARO > -1 ) {
      _message.verbosePrintf("Input REF_BARO: %d\n", pins.REF_BARO );
      currentPin = pins.REF_BARO;
      pinMode(pins.REF_BARO, INPUT);     
    }
    if (pins.SWIRL_ENCODER_A > -1 ) {
      _message.verbosePrintf("Input SERIAL0_RX: %d\n", pins.SWIRL_ENCODER_A );
      currentPin = pins.SWIRL_ENCODER_A;
      pinMode(pins.SWIRL_ENCODER_A, INPUT);   
    }
    if (pins.SWIRL_ENCODER_B > -1 ) {
      _message.verbosePrintf("Input SERIAL2_RX: %d\n", pins.SWIRL_ENCODER_B );
      currentPin = pins.SWIRL_ENCODER_B;
      pinMode(pins.SWIRL_ENCODER_B, INPUT);   
    }
        // if (pins.SERIAL0_RX > -1 ) {
    //   _message.verbosePrintf("Input SERIAL0_RX: %d\n", pins.SERIAL0_RX );
      // currentPin = pins.SERIAL0_RX;
    //   pinMode(pins.SERIAL0_RX, INPUT);   
    // }
    // if (pins.SERIAL2_RX > -1 ) {
    //   _message.verbosePrintf("Input SERIAL2_RX: %d\n", pins.SERIAL2_RX );
      // currentPin = pins.SERIAL2_RX;
    //   pinMode(pins.SERIAL2_RX, INPUT);   
    // }
    if (pins.SDA > -1 ) {
      _message.verbosePrintf("Input SDA: %d\n", pins.SDA );
      currentPin = pins.SDA;
      pinMode(pins.SDA, INPUT);   
    }
    if (pins.SCL > -1 ) {
      _message.verbosePrintf("Input SCL: %d\n", pins.SCL );
      currentPin = pins.SCL;
      pinMode(pins.SCL, INPUT);   
    }
    if (config.bSD_ENABLED) {
      // if (pins.SD_CS > -1 ) {
      //   _message.verbosePrintf("Input SD_CS: %d\n", pins.SD_CS );
      // currentPin = pins.SD_CS;
      //   pinMode(pins.SD_CS, INPUT);     
      // }
      // if (pins.SD_MISO > -1 ) {
      //   _message.verbosePrintf("Input SD_MISO: %d\n", pins.SD_MISO );
      // currentPin = pins.SD_MISO;
      //   pinMode(pins.SD_MISO, INPUT);   
      // }
      // if (pins.SD_SCK > -1 ) {
      //   _message.verbosePrintf("Input SD_SCK: %d\n", pins.SD_SCK );
      // currentPin = pins.SD_SCK;
      //   pinMode(pins.SD_SCK, INPUT);   
      // }
    }
    // if (pins.SPARE_PIN_1 >= 0 ) {
    //   _message.verbosePrintf("Input SPARE_PIN_1: %d\n", pins.SPARE_PIN_1 );
      // currentPin = pins.SPARE_PIN_1;
    //   pinMode(pins.SPARE_PIN_1, INPUT);   
    // }

    // // Set Outputs
    if (pins.VAC_BANK_1 > -1 ) {
      _message.verbosePrintf("Output VAC_BANK_1: %d\n", pins.VAC_BANK_1 );
      currentPin = pins.VAC_BANK_1;
      pinMode(pins.VAC_BANK_1, OUTPUT);
    }
    if (pins.VAC_BANK_2 > -1 ) {
      _message.verbosePrintf("Output VAC_BANK_2: %d\n", pins.VAC_BANK_2 );
      currentPin = pins.VAC_BANK_2;
      pinMode(pins.VAC_BANK_2, OUTPUT);
    }
    if (pins.VAC_BANK_3 > -1 ) {
      _message.verbosePrintf("Output VAC_BANK_3: %d\n", pins.VAC_BANK_3 );
      currentPin = pins.VAC_BANK_3;
      pinMode(pins.VAC_BANK_3, OUTPUT);
    }
    if (pins.VAC_SPEED > -1 ) {
      _message.verbosePrintf("Output VAC_SPEED: %d\n", pins.VAC_SPEED );
      currentPin = pins.VAC_SPEED;
      pinMode(pins.VAC_SPEED, OUTPUT);
    }
    if (pins.VAC_BLEED_VALVE > -1 ) {
      _message.verbosePrintf("Output VAC_BLEED_VALVE: %d\n", pins.VAC_BLEED_VALVE );
      currentPin = pins.VAC_BLEED_VALVE;
      pinMode(pins.VAC_BLEED_VALVE, OUTPUT);
    }
    if (pins.AVO_STEP > -1 ) {
      _message.verbosePrintf("Output AVO_STEP: %d\n", pins.AVO_STEP );
      currentPin = pins.AVO_STEP;
      pinMode(pins.AVO_STEP, OUTPUT);
    }
    if (pins.AVO_DIR > -1 ) {
      _message.verbosePrintf("Output AVO_DIR: %d\n", pins.AVO_DIR );
      currentPin = pins.AVO_DIR;
      pinMode(pins.AVO_DIR, OUTPUT);
    }
    if (pins.FLOW_VALVE_STEP > -1 ) {
      _message.verbosePrintf("Output FLOW_VALVE_STEP: %d\n", pins.FLOW_VALVE_STEP );
      currentPin = pins.FLOW_VALVE_STEP;
      pinMode(pins.FLOW_VALVE_STEP, OUTPUT);
    }
    if (pins.FLOW_VALVE_DIR > -1 ) {
      _message.verbosePrintf("Output FLOW_VALVE_DIR: %d\n", pins.FLOW_VALVE_DIR );
      currentPin = pins.FLOW_VALVE_DIR;
      pinMode(pins.FLOW_VALVE_DIR, OUTPUT);
    }
    if (config.bSD_ENABLED) {
      // if (pins.SD_MOSI > -1 ) {
      //   _message.verbosePrintf("Output SD_MOSI: %d\n", pins.SD_MOSI );
      // currentPin = pins.SD_MOSI;
      //   pinMode(pins.SD_MOSI, OUTPUT);
      // }
    }
    // if (pins.SERIAL0_TX > -1 ) {
    //   _message.verbosePrintf("Output SERIAL0_TX: %d\n", pins.SERIAL0_TX );
      // currentPin = pins.SERIAL0_TX;
    //   pinMode(pins.SERIAL0_TX, OUTPUT);
    // }
    // if (pins.SERIAL2_TX > -1 ) {
    //   _message.verbosePrintf("Output SERIAL2_TX: %d\n", pins.SERIAL2_TX );
      // currentPin = pins.SERIAL2_TX;
    //   pinMode(pins.SERIAL2_TX, OUTPUT);
    // }

  } catch (...) {
    _message.debugPrintf("I/O Initialisation Error. Pin: %u", currentPin);
    return currentPin;
  }
  _message.debugPrintf("I/O Initialised");
  return status.ioError;

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

  extern struct Configuration config;
  extern struct SensorData sensorVal;

  

  int32_t rawADCval = 0;

  if (config.iADC_TYPE != SENSOR_DISABLED){

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

  }

  return rawADCval;

}




/***********************************************************
 * @brief Get ADC channel Voltage
 * @param channel ADC channel (0-3)
 * @note 1115: 16 bits less sign bit = 15 bits mantissa = 32767 | 6.144v = max voltage (gain) of ADC | 187.5 uV / LSB
 * @note 1015: 12 bits less sign bit = 11 bit mantissa = 2047 | 6.144v = max voltage (gain) of ADC
 * int iADC_RANGE = 32767;
 * double dADC_GAIN = 6.144;
 ***/
 double Hardware::getADCVolts(int channel) {

  extern struct Configuration config;
  
  
  double volts;

  int rawADCval = getADCRawData(channel);

  switch (config.iADC_TYPE) {

    case SENSOR_DISABLED:
      return 1.00F;
    break;

    case ADS1115:
      volts = rawADCval * config.dADC_GAIN / config.iADC_RANGE;
      // volts = rawADCval * 6.144F / 32767;
    break;

    case ADS1015:
      volts = rawADCval * config.dADC_GAIN / 2047.00F; 
      // volts = rawADCval * 32767 / 2047.00F; 
    break;

    default:
      return 1.00F;
    break;

  }
  
  
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

  // TODO wifi / AC2 issue - https://github.com/espressif/esp-idf/issues/3714

  extern struct Pins pins;
  extern struct Configuration config;

  double vcc5vSupplyVolts = 5.0F;

  if (config.bFIXED_5V == true) {
    return vcc5vSupplyVolts; 
  } else {
    // long rawVoltageValue = readAnalog(pins.VCC_5V);  // Uses custom function to fix ADC2 + WiFi issue
    long rawVoltageValue = analogRead(pins.VCC_5V);  // Uses custom function to fix ADC2 + WiFi issue
    vcc5vSupplyVolts = (2 * static_cast<double>(rawVoltageValue) * 0.805860805860806F) ;
    return vcc5vSupplyVolts + config.dVCC_5V_TRIM;
  }
}




/***********************************************************
 * @brief GET 3.3V SUPPLY VOLTAGE
 * @details Measures 3.3v supply buck power to ESP32 via voltage divider
 * @note We use a 10k-10k divider on the official shield. This helps to capture voltages above +Vcc
 * @note Use a 0.1uf cap on input to help filter noise
 * @note ESP32 has 12 bit ADC (0-3300mv = 0-4095)
 ***/
double Hardware::get3v3SupplyVolts() {    

  extern struct Pins pins; 
  extern struct Configuration config;

  double vcc3v3SupplyVolts = 3.3F;

  if (config.bFIXED_3_3V) {
    return vcc3v3SupplyVolts; 
  } else {
    long rawVoltageValue = analogRead(pins.VCC_3V3 );  
    vcc3v3SupplyVolts = (2 * static_cast<double>(rawVoltageValue) * 0.805860805860806F) ;
    return vcc3v3SupplyVolts + config.dVCC_3V3_TRIM;
  }
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
  
  extern struct BenchSettings settings;
  extern struct Language language;
  extern struct SensorData sensorVal;

  double refPressure;
  double mafFlowRateCFM; 
 
  // TODO: Check scope of these...
  refPressure = _calculations.convertPressure(sensorVal.PRefKPA, INH2O);
  mafFlowRateCFM = _calculations.convertFlow(_sensors.getMafFlow());

  // comvert negative value into posotive
  refPressure = fabs(refPressure); 

  if ((refPressure > settings.min_bench_pressure))
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
  extern struct BenchSettings settings;
  extern struct Language language;
  extern struct Configuration config;
  
  double refPressure = _calculations.convertPressure(sensorVal.PRefKPA, INH2O);
    
  // REVIEW  - Ref pressure check
  // Check that pressure does not fall below limit set by iMIN_PRESS_PCT when bench is running
  // note alarm commented out in alarm function as 'nag' can get quite annoying
  // Is this a redundant check? Maybe a different alert would be more appropriate
  if ((refPressure < (settings.cal_ref_press * (config.iMIN_PRESS_PCT / 100))) && (Hardware::benchIsRunning()))
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

  digitalWrite(pins.VAC_BANK_1, HIGH);
//  digitalWrite(pins.VAC_BANK_2, HIGH);
//  digitalWrite(pins.VAC_BANK_3, HIGH);
}


/***********************************************************
 * @brief benchOff
 * @details Switches vac motor 1 output off
 ***/
void Hardware::benchOff() {

  extern struct Pins pins;

  digitalWrite(pins.VAC_BANK_1, LOW);
//  digitalWrite(pins.VAC_BANK_2, LOW);
//  digitalWrite(pins.VAC_BANK_3, LOW);
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

  digitalWrite(pins.AVO_DIR, HIGH);
  for(int n = 0; n < numberOfSteps; n++) {
    digitalWrite(pins.AVO_STEP, HIGH);
    delayMicroseconds(pulseWidthMicros); // this line is probably unnecessary
    digitalWrite(pins.AVO_STEP, LOW);
    
    delay(millisbetweenSteps);
    
  }
  
  delay(1000);
  

  digitalWrite(pins.AVO_DIR, LOW);
  for(int n = 0; n < numberOfSteps; n++) {
    digitalWrite(pins.AVO_STEP, HIGH);
    delayMicroseconds(pulseWidthMicros); // probably not needed
    digitalWrite(pins.AVO_STEP, LOW);
    
    delay(millisbetweenSteps);
  }

  _message.serialPrintf("Stepper Test Finished\n");

}
