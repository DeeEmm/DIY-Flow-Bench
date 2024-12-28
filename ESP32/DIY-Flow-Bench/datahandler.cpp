/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file datahandler.cpp
 * 
 * @brief Data Handler class
 * @note contols data exchange / file management 
 * 
 * @remarks For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * @license This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 ***/



#include "Arduino.h"

#include "datahandler.h"

#include <rom/rtc.h>
#include <FS.h>
#include <SPI.h>
#include <SD.h>
#include <Update.h>
#include <SPIFFS.h>
#include <Preferences.h>

#include <ArduinoJson.h>
// #include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "Wire.h"
#include <HTTPClient.h>
#include <cstring>
#include <string>
#include <stdio.h>

#include "structs.h"
#include "constants.h"
#include "hardware.h"
#include "messages.h"
#include "calculations.h"
#include "comms.h"
#include "webserver.h"
#include "calibration.h"
#include "API.h"
#include "mafdata.h"


void DataHandler::begin() {

    extern struct BenchSettings settings;
    extern struct Language language;
    extern struct DeviceStatus status;
    extern struct Configuration config;

    // Preferences _config_pref;
    Hardware _hardware;
    Pins pins;
    Messages _message;
    Comms _comms;
    DataHandler _data;
    Calibration _calibration;
    Calculations _calculations;

    StaticJsonDocument<1024> pinData;

    // Start serial comms
    this->beginSerial(); 

    _message.serialPrintf("\r\nDIY Flow Bench\n");                                         
    // NOTE: RELEASE and BUILD_NUMBER are defined at compile time within extra_scripts directive in user_actions_pre.py 
    _message.serialPrintf("DIYFB Version: %s \nBuild: %s \n", RELEASE, BUILD_NUMBER);                                         
    _message.serialPrintf("For help please visit the WIKI:\n");                                         
    _message.serialPrintf("https://github.com/DeeEmm/DIY-Flow-Bench/wiki\n");                                         

    // Load configuration / settings / calibration / liftdata / pins data from NVM
    this->initialiseConfig();
    this->loadConfig();

    this->initialiseSettings();
    this->loadSettings();
    
    _hardware.loadPinsData();

    if (_hardware.setPinMode()) {
        _message.serialPrintf("Pins set successfully\n");
    } else {
        _message.serialPrintf("!! Pins not set !!\n");
        status.doBootLoop = true;
    }

    this->initialiseLiftData();
    this->loadLiftData();
    
    _calibration.initialiseCalibrationData();
    _calibration.loadCalibrationData();

    // Initialise SPIFFS Filesystem
    _message.serialPrintf("Initialising File System \n"); 
    if (SPIFFS.begin()) {
    } else {
        _message.serialPrintf("...Failed\n");
        #if defined FORMAT_FILESYSTEM_IF_FAILED
            SPIFFS.format();
            _message.serialPrintf("!! File System Formatted !!\n");
            _message.serialPrintf("!! DIYFB Restarting !!\n");            
            delay(2000);
            ESP.restart();
        #endif
    }

    // Initialise WiFi
    _comms.initaliseWifi();

    // BootLoop method traps program pointer within loop until files are uploaded
    if (status.doBootLoop == true) bootLoop();


    // Start Wire (I2C) 
    Wire.begin (pins.SDA_PIN, pins.SCL_PIN); 
    Wire.setClock(100000);

    // TODO Initialise SD card
    if (config.bSD_ENABLED) {

    // test code from https://github.com/espressif/arduino-esp32/blob/master/libraries/SD/examples/SD_Test/SD_Test.ino

    // _message.serialPrintf("=== SDCARD File system info === \n");

    // int sck = SD_SCK_PIN;
    // int miso = SD_MISO_PIN;
    // int mosi = SD_MOSI_PIN;
    // int cs = SD_CS_PIN;

    // SPIClass spi = SPIClass(VSPI);

    //     SPI.begin(sck, miso, mosi, cs);
    //     if (!SD.begin(cs, spi, 2000000U)) {
    //     Serial.println("Card Mount Failed");

    //     } else {

    //     uint8_t cardType = SD.cardType();

    //     if (cardType == CARD_NONE) {
    //         Serial.println("No SD card attached");
    //     }

    //     Serial.print("SD Card Type: ");
    //     if (cardType == CARD_MMC) {
    //         Serial.println("MMC");
    //     } else if (cardType == CARD_SD) {
    //         Serial.println("SDSC");
    //     } else if (cardType == CARD_SDHC) {
    //         Serial.println("SDHC");
    //     } else {
    //         Serial.println("UNKNOWN");
    //     }

    //     uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    //     Serial.printf("SD Card Size: %lluMB\n", cardSize);

    //     listDir(SD, "/", 0);
    //     createDir(SD, "/mydir");
    //     listDir(SD, "/", 0);
    //     removeDir(SD, "/mydir");
    //     listDir(SD, "/", 2);
    //     writeFile(SD, "/hello.txt", "Hello ");
    //     appendFile(SD, "/hello.txt", "World!\n");
    //     readFile(SD, "/hello.txt");
    //     deleteFile(SD, "/foo.txt");
    //     renameFile(SD, "/hello.txt", "/foo.txt");
    //     readFile(SD, "/foo.txt");
    //     testFileIO(SD, "/test.txt");
    //     Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    //     Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
    //     }

    }

    
    // Display Filesystem Stats
    status.spiffs_mem_size = SPIFFS.totalBytes();
    status.spiffs_mem_used = SPIFFS.usedBytes();

    _message.serialPrintf("=== SPIFFS File system info === \n");
    _message.serialPrintf("Total space:      %s \n", _calculations.byteDecode(status.spiffs_mem_size));
    _message.serialPrintf("Total space used: %s \n", _calculations.byteDecode(status.spiffs_mem_used));

}


/***********************************************************
* @name checkSubstring.cpp
* @brief Checks for presence of second string within first string
* @param firstString
* @param secondString
* @returns true if string found
***/
bool DataHandler::checkSubstring(std::string firstString, std::string secondString){
    if(secondString.size() > firstString.size())
        return false;

    for (int i = 0; i < firstString.size(); i++){
        int j = 0;
        // If the first characters match
        if(firstString[i] == secondString[j]){
            int k = i;
            while (firstString[i] == secondString[j] && j < secondString.size()){
                j++;
                i++;
            }
            if (j == secondString.size())
                return true;
            else // Re-initialize i to its original value
                i = k;
        }
    }
    return false;
}




/***********************************************************
 * @brief writeJSONFile
 * @details write JSON string to file
 ***/
void DataHandler::writeJSONFile(String data, String filename, int dataSize){

  Messages _message;

  // StaticJsonDocument<dataSize> jsonData;
  DynamicJsonDocument jsonData(dataSize);
  DeserializationError error = deserializeJson(jsonData, data);
  if (!error)  {
    _message.debugPrintf("Writing JSON file... \n");
    File outputFile = SPIFFS.open(filename, FILE_WRITE);
    serializeJsonPretty(jsonData, outputFile);
    outputFile.close();
  }  else  {
    _message.statusPrintf("Webserver::writeJSONFile ERROR \n");
  }
}




// /***********************************************************
// * @brief createLiftDataFile
// * @details Create blank lift data json file
// * @note Called from DataHandler::begin() if liftdata.json not found
// ***/
// void DataHandler::createLiftDataFile () {

//   Messages _message;
//   String jsonString;  
//   StaticJsonDocument<LIFT_DATA_JSON_SIZE> liftData;

//   _message.serialPrintf("Creating liftdata.json file... \n"); 

//   liftData["LIFTDATA1"] = 0.0;
//   liftData["LIFTDATA2"] = 0.0;
//   liftData["LIFTDATA3"] = 0.0;
//   liftData["LIFTDATA4"] = 0.0;
//   liftData["LIFTDATA5"] = 0.0;
//   liftData["LIFTDATA6"] = 0.0;
//   liftData["LIFTDATA7"] = 0.0;
//   liftData["LIFTDATA8"] = 0.0;
//   liftData["LIFTDATA9"] = 0.0;
//   liftData["LIFTDATA10"] = 0.0;
//   liftData["LIFTDATA11"] = 0.0;
//   liftData["LIFTDATA12"] = 0.0;

//   serializeJsonPretty(liftData, jsonString);
//   writeJSONFile(jsonString, "/liftdata.json", LIFT_DATA_JSON_SIZE);

// }




// /***********************************************************
// * @brief createCalibration File
// * @details Create configuration json file
// * @note Called from DataHandler::begin() if cal.json not found
// ***/
// void DataHandler::createCalibrationFile () {

//   extern struct CalibrationData calVal;
//   Messages _message;
//   String jsonString;
//   StaticJsonDocument<CAL_DATA_JSON_SIZE> calData;
  
//   _message.debugPrintf("Creating cal.json file... \n"); 
  
//   calData["FLOW_OFFSET"] = calVal.flow_offset;
//   calData["USER_OFFSET"] = calVal.user_offset;
//   calData["LEAK_BASE"] = calVal.leak_cal_baseline;
//   calData["LEAK_BASE_REV"] = calVal.leak_cal_baseline_rev;
//   calData["LEAK_OFFSET"] = calVal.leak_cal_offset;
//   calData["LEAK_OFFSET_REV"] = calVal.leak_cal_offset_rev;

//   serializeJsonPretty(calData, jsonString);

//   File outputFile = SPIFFS.open("/cal.json", FILE_WRITE);
//   serializeJsonPretty(calData, outputFile);
//   outputFile.close();
  
// }





/***********************************************************
 * @brief loadJSONFile
 * @details Loads JSON data from file
 * @note uses MAF_JSON_SIZE - largest possible file size
 ***/
StaticJsonDocument<JSON_FILE_SIZE> DataHandler::loadJSONFile(String filename) {

  Messages _message;

//   extern struct Language language;

  // Allocate the memory pool on the stack.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument <JSON_FILE_SIZE> jsonData;

  if (SPIFFS.exists(filename))  {
    File jsonFile = SPIFFS.open(filename, FILE_READ);

    if (!jsonFile)    {
    //   _message.Handler(language.LANG_ERROR_LOADING_FILE);
      _message.statusPrintf("Failed to open file for reading \n");
    }    else    {
      size_t size = jsonFile.size();
      if (size > JSON_FILE_SIZE)    {
          _message.statusPrintf("File too large \n");
      }

      DeserializationError error = deserializeJson(jsonData, jsonFile);
      if (error)      {
        _message.statusPrintf("loadJSONFile->deserializeJson() failed: %s \n", error.f_str());
      }

      jsonFile.close();
      return jsonData;

    }
    jsonFile.close();
  }  else  {
    _message.statusPrintf("File missing \n");
  }

  return jsonData;

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
void DataHandler::beginSerial(void) {
	
    Messages _message;
    extern struct Pins pins;

	// #if defined SERIAL0_ENABLED
	// 	Serial.begin(SERIAL0_BAUD, SERIAL_8N1 , pins.SERIAL0_RX_PIN, pins.SERIAL0_TX_PIN); 
	// #endif
	
    Serial.begin(SERIAL0_BAUD);
    _message.serialPrintf("Serial started \n"); 

}




/***********************************************************
* @brief initaliseConfiguration
* @details define settings in NVM
* @note Replaces pre-compile macros in original config.h file
* @note Preferences Key can not exceed 15 chars long
* @note Datatype prefix allows for reduced save method
* @note b = bool, i = int, d = double, s = string
***/ 
void DataHandler::initialiseConfig () {

  extern struct Configuration config;

  Messages _message;
  Preferences _config_pref;

  _config_pref.begin("config", false);

  if (_config_pref.isKey("bSWIRL_ENBLD")) { // we've already initialised _config_pref
    _config_pref.end();
    return;
  }

  _message.serialPrintf("Initialising Configuration \n");    

  // _config_pref.clear(); // completely remove namepace
  // _config_pref.remove("iADC_I2C_ADDR"); // remove individual key

  if (!_config_pref.isKey("bSD_ENABLED")) _config_pref.putBool("bSD_ENABLED", false);
  if (!_config_pref.isKey("iMIN_PRESS_PCT")) _config_pref.putInt("iMIN_PRESS_PCT", 80);
  if (!_config_pref.isKey("dPIPE_RAD_FT")) _config_pref.putDouble("dPIPE_RAD_FT", 0.328084);

  if (!_config_pref.isKey("dVCC_3V3_TRIM")) _config_pref.putDouble("dVCC_3V3_TRIM", 0.0);
  if (!_config_pref.isKey("dVCC_5V_TRIM")) _config_pref.putDouble("dVCC_5V_TRIM", 0.0);
  if (!_config_pref.isKey("bFIXED_3_3V")) _config_pref.putBool("bFIXED_3_3V", true);
  if (!_config_pref.isKey("bFIXED_5V")) _config_pref.putBool("bFIXED_5V", true);

  if (!_config_pref.isKey("bBME280_ENBLD")) _config_pref.putBool("bBME280_ENBLD", true);
  if (!_config_pref.isKey("iBME280_ADDR")) _config_pref.putInt("iBME280_ADDR", 118);
  if (!_config_pref.isKey("iBME280_SCN_MS")) _config_pref.putInt("iBME280_SCN_MS", 1000);

  if (!_config_pref.isKey("bBME680_ENBLD")) _config_pref.putBool("bBME680_ENBLD", true);
  if (!_config_pref.isKey("iBME680_ADDR")) _config_pref.putInt("iBME680_ADDR", 119);
  if (!_config_pref.isKey("iBME680_SCN_MS")) _config_pref.putInt("iBME680_SCN_MS", 1000);

  if (!_config_pref.isKey("iADC_TYPE")) _config_pref.putInt("iADC_TYPE", 11);
  if (!_config_pref.isKey("iADC_I2C_ADDR")) _config_pref.putInt("iADC_I2C_ADDR", 72);
  if (!_config_pref.isKey("iADC_SCAN_DLY")) _config_pref.putInt("iADC_SCAN_DLY", 1000);
  if (!_config_pref.isKey("iADC_MAX_RETRY")) _config_pref.putInt("iADC_MAX_RETRY", 10);
  if (!_config_pref.isKey("iADC_RANGE")) _config_pref.putInt("iADC_RANGE", 32767);
  if (!_config_pref.isKey("dADC_GAIN")) _config_pref.putDouble("dADC_GAIN", 6.144);

  if (!_config_pref.isKey("iMAF_SRC_TYPE")) _config_pref.putInt("iMAF_SRC_TYPE", 11);
  if (!_config_pref.isKey("iMAF_SENS_TYPE")) _config_pref.putString("iMAF_SENS_TYPE", 0);
  if (!_config_pref.isKey("dMAF_MV_TRIM")) _config_pref.putDouble("dMAF_MV_TRIM", 0.0);
  if (!_config_pref.isKey("iMAF_ADC_CHAN")) _config_pref.putInt("iMAF_ADC_CHAN", 0);

  if (!_config_pref.isKey("iPREF_SENS_TYP")) _config_pref.putInt("iPREF_SENS_TYP", 4);
  if (!_config_pref.isKey("iPREF_SRC_TYP")) _config_pref.putInt("iPREF_SRC_TYP", 11);
  if (!_config_pref.isKey("iFIXED_PREF_VAL")) _config_pref.putInt("iFIXED_PREF_VAL", 1);
  if (!_config_pref.isKey("dPREF_MV_TRIM")) _config_pref.putDouble("dPREF_MV_TRIM", 0.0);
  if (!_config_pref.isKey("dPREF_ALG_SCALE")) _config_pref.putDouble("dPREF_ALG_SCALE", 0.0);
  if (!_config_pref.isKey("iPREF_ADC_CHAN")) _config_pref.putInt("iPREF_ADC_CHAN", 1);

  if (!_config_pref.isKey("iPDIFF_SENS_TYP")) _config_pref.putInt("iPDIFF_SENS_TYP", 4);
  if (!_config_pref.isKey("iPDIFF_SRC_TYP")) _config_pref.putInt("iPDIFF_SRC_TYP", 11);
  if (!_config_pref.isKey("iFIXD_PDIFF_VAL")) _config_pref.putInt("iFIXD_PDIFF_VAL", 1);
  if (!_config_pref.isKey("dPDIFF_MV_TRIM")) _config_pref.putDouble("dPDIFF_MV_TRIM", 0.0);
  if (!_config_pref.isKey("dPDIFF_SCALE")) _config_pref.putDouble("dPDIFF_SCALE", 0.0);
  if (!_config_pref.isKey("iPDIFF_ADC_CHAN")) _config_pref.putInt("iPDIFF_ADC_CHAN", 1);

  if (!_config_pref.isKey("iPITOT_SENS_TYP")) _config_pref.putInt("iPITOT_SENS_TYP", 4);
  if (!_config_pref.isKey("iPITOT_SRC_TYP")) _config_pref.putInt("iPITOT_SRC_TYP", 11);
  if (!_config_pref.isKey("dPITOT_MV_TRIM")) _config_pref.putDouble("dPITOT_MV_TRIM", 0.0);
  if (!_config_pref.isKey("dPITOT_SCALE")) _config_pref.putDouble("dPITOT_SCALE", 0.0);
  if (!_config_pref.isKey("iPITOT_ADC_CHAN")) _config_pref.putInt("iPITOT_ADC_CHAN", 1);

  if (!_config_pref.isKey("iBARO_SENS_TYP")) _config_pref.putInt("iBARO_SENS_TYP", BOSCH_BME280);
  if (!_config_pref.isKey("dFIXD_BARO_VAL")) _config_pref.putDouble("dFIXD_BARO_VAL", 101.3529);
  if (!_config_pref.isKey("dBARO_ALG_SCALE")) _config_pref.putDouble("dBARO_ALG_SCALE", 1.0);
  if (!_config_pref.isKey("dBARO_MV_TRIM")) _config_pref.putDouble("dBARO_MV_TRIM", 1.0);
  if (!_config_pref.isKey("dBARO_FINE_TUNE")) _config_pref.putDouble("dBARO_FINE_TUNE", 1.0);
  if (!_config_pref.isKey("dBARO_SCALE")) _config_pref.putDouble("dBARO_SCALE", 1.0);
  if (!_config_pref.isKey("dBARO_OFFSET")) _config_pref.putDouble("dBARO_OFFSET", 1.0);
  if (!_config_pref.isKey("dSEALEVEL_PRESS")) _config_pref.putDouble("dSEALEVEL_PRESS", 0.0);
  if (!_config_pref.isKey("iBARO_ADC_CHAN")) _config_pref.putInt("iBARO_ADC_CHAN", 4);

  if (!_config_pref.isKey("iTEMP_SENS_TYPE")) _config_pref.putInt("iTEMP_SENS_TYPE", BOSCH_BME280);
  if (!_config_pref.isKey("dFIXED_TEMP_VAL")) _config_pref.putDouble("dFIXED_TEMP_VAL", 21.0);
  if (!_config_pref.isKey("dTEMP_ALG_SCALE")) _config_pref.putDouble("dTEMP_ALG_SCALE", 1.0);
  if (!_config_pref.isKey("dTEMP_MV_TRIM")) _config_pref.putDouble("dTEMP_MV_TRIM", 1.0);
  if (!_config_pref.isKey("dTEMP_FINE_TUNE")) _config_pref.putDouble("dTEMP_FINE_TUNE", 1.0);

  if (!_config_pref.isKey("iRELH_SENS_TYP")) _config_pref.putInt("iRELH_SENS_TYP", BOSCH_BME280);
  if (!_config_pref.isKey("dFIXED_RELH_VAL")) _config_pref.putDouble("dFIXED_RELH_VAL", 36.0);
  if (!_config_pref.isKey("dRELH_ALG_SCALE")) _config_pref.putDouble("dRELH_ALG_SCALE", 1.0);
  if (!_config_pref.isKey("dRELH_MV_TRIM")) _config_pref.putDouble("dRELH_MV_TRIM", 1.0);
  if (!_config_pref.isKey("dRELH_FINE_TUNE")) _config_pref.putDouble("dRELH_FINE_TUNE", 1.0);
  if (!_config_pref.isKey("bSWIRL_ENBLD")) _config_pref.putBool("bSWIRL_ENBLD", false);

  _config_pref.end();

}




/***********************************************************
* @brief loadConfiguration
* @details read settings from ESP32 NVM and loads into global struct
* @note Replaces pre-compile macros in original config.h file
* @note Preferences Kay can not exceed 15 chars long
* @note Datatype prefix allows for reduced save method
* @note b = bool, i = int, d = double, s = string
***/ 
void DataHandler::loadConfig () {

  extern struct Configuration config;

  Messages _message;
  Preferences _config_pref;

  _message.serialPrintf("Loading Configuration \n");    
  
  _config_pref.begin("config", true);

  config.bSD_ENABLED = _config_pref.getBool("bSD_ENABLED", false);
  config.iMIN_PRESS_PCT = _config_pref.getInt("iMIN_PRESS_PCT", 80);
  config.dPIPE_RAD_FT = _config_pref.getDouble("dPIPE_RAD_FT", 0.328084);

  config.dVCC_3V3_TRIM = _config_pref.getDouble("dVCC_3V3_TRIM", 0.0);
  config.dVCC_5V_TRIM = _config_pref.getDouble("dVCC_5V_TRIM", 0.0);
  config.bFIXED_3_3V = _config_pref.getBool("bFIXED_3_3V", true);
  config.bFIXED_5V = _config_pref.getBool("bFIXED_5V", true);

  config.bBME280_ENBLD = _config_pref.getBool("bBME280_ENBLD", true);
  config.iBME280_ADDR = _config_pref.getInt("iBME280_ADDR", 118);
  config.iBME280_SCN_MS = _config_pref.getInt("iBME280_SCN_MS", 1000);

  config.bBME680_ENBLD = _config_pref.getBool("bBME680_ENBLD", false);
  config.iBME680_ADDR = _config_pref.getInt("iBME680_ADDR", 119);
  config.iBME680_SCN_MS = _config_pref.getInt("iBME680_SCN_MS", 1000);

  config.iADC_TYPE = _config_pref.getInt("iADC_TYPE", 11);
  config.iADC_I2C_ADDR = _config_pref.getInt("iADC_I2C_ADDR", 72);
  config.iADC_SCAN_DLY = _config_pref.getInt("iADC_SCAN_DLY", 1000);
  config.iADC_MAX_RETRY  = _config_pref.getInt("iADC_MAX_RETRY", 10);
  config.iADC_RANGE = _config_pref.getInt("iADC_RANGE", 32767);
  config.dADC_GAIN = _config_pref.getDouble("dADC_GAIN", 6.144);

  config.iMAF_SRC_TYPE = _config_pref.getInt("iMAF_SRC_TYPE", 11);
  config.iMAF_SENS_TYPE = _config_pref.getInt("iMAF_SENS_TYPE", 0);
  config.dMAF_MV_TRIM = _config_pref.getDouble("dMAF_MV_TRIM", 0.0);
  config.iMAF_ADC_CHAN = _config_pref.getInt("iMAF_ADC_CHAN", 0);

  config.iPREF_SENS_TYP = _config_pref.getInt("iPREF_SENS_TYP", 4);
  config.iPREF_SRC_TYP = _config_pref.getInt("iPREF_SRC_TYP", 11);
  config.iFIXED_PREF_VAL = _config_pref.getInt("iFIXED_PREF_VAL", 1);
  config.dPREF_MV_TRIM = _config_pref.getDouble("dPREF_MV_TRIM", 0.0);
  config.dPREF_ALG_SCALE = _config_pref.getDouble("dPREF_ALG_SCALE", 1.0);
  config.iPREF_ADC_CHAN = _config_pref.getInt("iPREF_ADC_CHAN", 1);

  config.iPDIFF_SENS_TYP = _config_pref.getInt("iPDIFF_SENS_TYP", 4);
  config.iPDIFF_SRC_TYP = _config_pref.getInt("iPDIFF_SRC_TYP", 11);
  config.iFIXD_PDIFF_VAL = _config_pref.getInt("iFIXD_PDIFF_VAL", 1);
  config.dPDIFF_MV_TRIM = _config_pref.getDouble("dPDIFF_MV_TRIM", 0.0);
  config.dPDIFF_SCALE = _config_pref.getDouble("dPDIFF_SCALE", 1.0);
  config.iPDIFF_ADC_CHAN = _config_pref.getInt("iPDIFF_ADC_CHAN", 2);
 
  config.iPITOT_SENS_TYP = _config_pref.getInt("iPITOT_SENS_TYP", SENSOR_DISABLED);
  config.iPITOT_SRC_TYP = _config_pref.getInt("iPITOT_SRC_TYP", 11);
  config.dPITOT_MV_TRIM = _config_pref.getDouble("dPITOT_MV_TRIM", 0.0);
  config.dPITOT_SCALE = _config_pref.getDouble("dPITOT_SCALE", 1.0);
  config.iPITOT_ADC_CHAN = _config_pref.getInt("iPITOT_ADC_CHAN", 3);

  config.iBARO_SENS_TYP = _config_pref.getInt("iBARO_SENS_TYP", BOSCH_BME280);
  config.dFIXD_BARO_VAL = _config_pref.getDouble("dFIXD_BARO_VAL", 101.3529);
  config.dBARO_ALG_SCALE =_config_pref.getDouble("dBARO_ALG_SCALE", 1.0);
  config.dBARO_MV_TRIM = _config_pref.getDouble("dBARO_MV_TRIM", 1.0);
  config.dBARO_FINE_TUNE = _config_pref.getDouble("dBARO_FINE_TUNE", 1.0);
  config.dBARO_SCALE = _config_pref.getDouble("dBARO_SCALE", 100);
  config.dBARO_OFFSET = _config_pref.getDouble("dBARO_OFFSET", 100);
  config.dSEALEVEL_PRESS = _config_pref.getDouble("dSEALEVEL_PRESS", 1016.90);
  config.iBARO_ADC_CHAN = _config_pref.getInt("iBARO_ADC_CHAN", 4);

  config.iTEMP_SENS_TYPE = _config_pref.getInt("iTEMP_SENS_TYPE", BOSCH_BME280);
  config.dFIXED_TEMP_VAL = _config_pref.getDouble("dFIXED_TEMP_VAL", 21.0);
  config.dTEMP_ALG_SCALE = _config_pref.getDouble("dTEMP_ALG_SCALE", 1.0);
  config.dTEMP_MV_TRIM = _config_pref.getDouble("dTEMP_MV_TRIM", 0.0);
  config.dTEMP_FINE_TUNE = _config_pref.getDouble("dTEMP_FINE_TUNE", 0.0);

  config.iRELH_SENS_TYP = _config_pref.getInt("iRELH_SENS_TYP", BOSCH_BME280);
  config.dFIXED_RELH_VAL = _config_pref.getDouble("dFIXED_RELH_VAL", 36.0);
  config.dRELH_ALG_SCALE = _config_pref.getDouble("dRELH_ALG_SCALE", 1.0);
  config.dRELH_MV_TRIM = _config_pref.getDouble("dRELH_MV_TRIM", 0.0);
  config.dRELH_FINE_TUNE = _config_pref.getDouble("dRELH_FINE_TUNE", 0.0);
  config.bSWIRL_ENBLD = _config_pref.getBool("bSWIRL_ENBLD", false);

  _config_pref.end();
}










/***********************************************************
* @brief initialiseSettings
* @note - Initialise settings in NVM if they do not exist
* @note Key must be 15 chars or shorter.
* @note Datatype prefix allows for reduced save method
* @note b = bool, i = int, d = double, s = string***/ 
void DataHandler::initialiseSettings () {

  extern struct BenchSettings settings;

  DataHandler _data;
  Messages _message;
  Preferences _settings_pref;

  _message.serialPrintf("Loading Bench Settings \n");    
  
  _settings_pref.begin("settings", false);

  // _settings_pref.remove("iDATA_FLTR_TYP"); // remove individual key
  // _settings_pref.remove("iTEMP_UNIT"); // remove individual key

  if (!_settings_pref.isKey("sWIFI_SSID")) _settings_pref.putString("sWIFI_SSID", "WIFI-SSID");
  if (!_settings_pref.isKey("sWIFI_PSWD")) _settings_pref.putString("sWIFI_PSWD", static_cast<String>("PASSWORD"));
  if (!_settings_pref.isKey("sWIFI_AP_SSID")) _settings_pref.putString("sWIFI_AP_SSID", static_cast<String>("DIYFB"));
  if (!_settings_pref.isKey("sWIFI_AP_PSWD")) _settings_pref.putString("sWIFI_AP_PSWD", static_cast<String>("123456789"));
  if (!_settings_pref.isKey("sHOSTNAME")) _settings_pref.putString("sHOSTNAME", static_cast<String>("diyfb"));

  if (!_settings_pref.isKey("iWIFI_TIMEOUT")) _settings_pref.putInt("iWIFI_TIMEOUT", 4000);
  if (!_settings_pref.isKey("iMAF_DIAMETER")) _settings_pref.putInt("iMAF_DIAMETER", 0);
  if (!_settings_pref.isKey("iREFRESH_RATE")) _settings_pref.putInt("iREFRESH_RATE", 500);
  if (!_settings_pref.isKey("iMIN_PRESSURE")) _settings_pref.putInt("iMIN_PRESSURE", 1);
  if (!_settings_pref.isKey("iMIN_FLOW_RATE")) _settings_pref.putInt("iMIN_FLOW_RATE", 1);

  if (!_settings_pref.isKey("iDATA_FLTR_TYP")) _settings_pref.putInt("iDATA_FLTR_TYP", NONE);
  if (!_settings_pref.isKey("iROUNDING_TYP")) _settings_pref.putInt("iROUNDING_TYP", NONE);

  if (!_settings_pref.isKey("iFLOW_DECI_ACC")) _settings_pref.putInt("iFLOW_DECI_ACC", 1);
  if (!_settings_pref.isKey("iGEN_DECI_ACC")) _settings_pref.putInt("iGEN_DECI_ACC", 2);
  if (!_settings_pref.isKey("iCYC_AV_BUFF")) _settings_pref.putInt("iCYC_AV_BUFF", 5);
  if (!_settings_pref.isKey("iMAF_MIN_VOLTS")) _settings_pref.putInt("iMAF_MIN_VOLTS", 1);

  if (!_settings_pref.isKey("sAPI_DELIM")) _settings_pref.putString("sAPI_DELIM", ":");
  if (!_settings_pref.isKey("iSERIAL_BAUD")) _settings_pref.putInt("iSERIAL_BAUD", 115200);
  if (!_settings_pref.isKey("iSHOW_ALARMS")) _settings_pref.putInt("iSHOW_ALARMS", true);
  if (!_settings_pref.isKey("iADJ_FLOW_DEP")) _settings_pref.putInt("iADJ_FLOW_DEP", 28);
  if (!_settings_pref.isKey("iSTD_REF")) _settings_pref.putInt("iSTD_REF", 1);
  if (!_settings_pref.isKey("iSTD_ADJ_FLOW")) _settings_pref.putInt("iSTD_ADJ_FLOW", 0);
  if (!_settings_pref.isKey("iDATAGRAPH_MAX")) _settings_pref.putInt("iDATAGRAPH_MAX", 0);
  if (!_settings_pref.isKey("iMAF_MIN_VOLTS")) _settings_pref.putInt("iMAF_MIN_VOLTS", 1);
  if (!_settings_pref.isKey("iTEMP_UNIT")) _settings_pref.putInt("iTEMP_UNIT", CELCIUS);

  if (!_settings_pref.isKey("dLIFT_INTERVAL")) _settings_pref.putDouble("dLIFT_INTERVAL", 1.5F);
  if (!_settings_pref.isKey("iBENCH_TYPE")) _settings_pref.putInt("iBENCH_TYPE", MAF);
  if (!_settings_pref.isKey("dCAL_FLW_RATE")) _settings_pref.putDouble("dCAL_FLW_RATE", 14.4F);
  if (!_settings_pref.isKey("dCAL_REF_PRESS")) _settings_pref.putDouble("dCAL_REF_PRESS", 10.0F);
  if (!_settings_pref.isKey("dORIFICE1_FLOW")) _settings_pref.putDouble("dORIFICE1_FLOW", 0.0F);
  if (!_settings_pref.isKey("dORIFICE1_PRESS")) _settings_pref.putDouble("dORIFICE1_PRESS", 0.0F);
  if (!_settings_pref.isKey("dORIFICE2_FLOW")) _settings_pref.putDouble("dORIFICE2_FLOW", 0.0F);
  if (!_settings_pref.isKey("dORIFICE2_PRESS")) _settings_pref.putDouble("dORIFICE2_PRESS", 0.0F);
  if (!_settings_pref.isKey("dORIFICE3_FLOW")) _settings_pref.putDouble("dORIFICE3_FLOW", 0.0F);
  if (!_settings_pref.isKey("dORIFICE3_PRESS")) _settings_pref.putDouble("dORIFICE3_PRESS", 0.0F);
  if (!_settings_pref.isKey("dORIFICE4_FLOW")) _settings_pref.putDouble("dORIFICE4_FLOW", 0.0F);
  if (!_settings_pref.isKey("dORIFICE4_PRESS")) _settings_pref.putDouble("dORIFICE4_PRESS", 0.0F);
  if (!_settings_pref.isKey("dORIFICE5_FLOW")) _settings_pref.putDouble("dORIFICE5_FLOW", 0.0F);
  if (!_settings_pref.isKey("dORIFICE5_PRESS")) _settings_pref.putDouble("dORIFICE5_PRESS", 0.0F);
  if (!_settings_pref.isKey("dORIFICE6_FLOW")) _settings_pref.putDouble("dORIFICE6_FLOW", 0.0F);
  if (!_settings_pref.isKey("dORIFICE6_PRESS")) _settings_pref.putDouble("dORIFICE6_PRESS", 0.0F);

  _settings_pref.end();

}











/***********************************************************
* @brief loadSettings
* @details read settings from NVM and loads into global struct
* @note Datatype prefix allows for reduced save method
* @note b = bool, i = int, d = double, s = string
***/ 
void DataHandler::loadSettings () {

  extern struct BenchSettings settings;

  DataHandler _data;
  Messages _message;
  Preferences _settings_pref;

  _message.serialPrintf("Loading Settings \n");    
  
  _settings_pref.begin("settings", true);

  settings.wifi_ssid = _settings_pref.getString("sWIFI_SSID", "WIFI-SSID");
  settings.wifi_pswd = _settings_pref.getString("sWIFI_PSWD", "PASSWORD");
  settings.wifi_ap_ssid = _settings_pref.getString("sWIFI_AP_SSID", "DIYFB" );
  settings.wifi_ap_pswd =_settings_pref.getString("sWIFI_AP_PSWD", "123456789" );
  settings.hostname = _settings_pref.getString("sHOSTNAME", "diyfb" );
  settings.wifi_timeout = _settings_pref.getInt("iWIFI_TIMEOUT", 4000 );
  settings.maf_housing_diameter = _settings_pref.getInt("iMAF_DIAMETER", 0 );
  settings.refresh_rate = _settings_pref.getInt("iREFRESH_RATE", 500 );
  settings.min_bench_pressure  = _settings_pref.getInt("iMIN_PRESSURE", 1 );
  settings.min_flow_rate = _settings_pref.getInt("iMIN_FLOW_RATE", 1 );
  settings.data_filter_type = _settings_pref.getInt("iDATA_FLTR_TYP", NONE );
  settings.rounding_type = _settings_pref.getInt("iROUNDING_TYP", NONE );
  settings.flow_decimal_length = _settings_pref.getInt("iFLOW_DECI_ACC", 1 );
  settings.gen_decimal_length = _settings_pref.getInt("iGEN_DECI_ACC", 2 );
  settings.cyc_av_buffer  = _settings_pref.getInt("iCYC_AV_BUFF", 5 );
  settings.maf_min_volts  = _settings_pref.getInt("iMAF_MIN_VOLTS", 0.1F );
  settings.api_delim = _settings_pref.getString("sAPI_DELIM", ":" );
  settings.serial_baud_rate = _settings_pref.getInt("iSERIAL_BAUD",  115200 );
  settings.show_alarms = _settings_pref.getInt("iSHOW_ALARMS",  true  );
  settings.adj_flow_depression = _settings_pref.getInt("iADJ_FLOW_DEP",  28  );
  settings.standardReference = _settings_pref.getInt("iSTD_REF", 1  );
  settings.std_adj_flow = _settings_pref.getInt("iSTD_ADJ_FLOW",  0 );
  settings.dataGraphMax = _settings_pref.getInt("iDATAGRAPH_MAX", 0 );
  settings.temp_unit = _settings_pref.getInt("iTEMP_UNIT", CELCIUS );
  settings.valveLiftInterval = _settings_pref.getDouble("dLIFT_INTERVAL", 1.5F  );
  settings.bench_type = _settings_pref.getInt("iBENCH_TYPE", MAF );
  settings.cal_flow_rate = _settings_pref.getDouble("dCAL_FLW_RATE", 14.4F );
  settings.cal_ref_press = _settings_pref.getDouble("dCAL_REF_PRESS", 10.0F );
  settings.orificeOneFlow = _settings_pref.getDouble("dORIFICE1_FLOW", 0.0F );
  settings.orificeOneDepression = _settings_pref.getDouble("dORIFICE1_PRESS", 0.0F );
  settings.orificeTwoFlow = _settings_pref.getDouble("dORIFICE2_FLOW", 0.0F );
  settings.orificeTwoDepression = _settings_pref.getDouble("dORIFICE2_PRESS", 0.0F );
  settings.orificeThreeFlow = _settings_pref.getDouble("dORIFICE3_FLOW", 0.0F );
  settings.orificeThreeDepression = _settings_pref.getDouble("dORIFICE3_PRESS", 0.0F );
  settings.orificeFourFlow = _settings_pref.getDouble("dORIFICE4_FLOW", 0.0F );
  settings.orificeFourDepression = _settings_pref.getDouble("dORIFICE4_PRESS", 0.0F );
  settings.orificeFiveFlow = _settings_pref.getDouble("dORIFICE5_FLOW", 0.0F );
  settings.orificeFiveDepression = _settings_pref.getDouble("dORIFICE5_PRESS", 0.0F );
  settings.orificeSixFlow = _settings_pref.getDouble("dORIFICE6_FLOW", 0.0F );
  settings.orificeSixDepression = _settings_pref.getDouble("dORIFICE6_PRESS",  0.0F);

  _settings_pref.end();

}






/***********************************************************
* @brief initialiseLiftData
* @note - Initialise settings in NVM if they do not exist
* @note Key must be 15 chars or shorter.
***/ 
void DataHandler::initialiseLiftData () {

  extern struct BenchSettings settings;

  DataHandler _data;
  Messages _message;
  Preferences _lift_data_pref;

  _message.serialPrintf("Initialising Lift Data \n");    
  
  _lift_data_pref.begin("liftData", false);

  if (!_lift_data_pref.isKey("LIFTDATA1")) _lift_data_pref.putDouble("LIFTDATA1", 0.0);
  if (!_lift_data_pref.isKey("LIFTDATA2")) _lift_data_pref.putDouble("LIFTDATA2", 0.0);
  if (!_lift_data_pref.isKey("LIFTDATA3")) _lift_data_pref.putDouble("LIFTDATA3", 0.0);
  if (!_lift_data_pref.isKey("LIFTDATA4")) _lift_data_pref.putDouble("LIFTDATA4", 0.0);
  if (!_lift_data_pref.isKey("LIFTDATA5")) _lift_data_pref.putDouble("LIFTDATA5", 0.0);
  if (!_lift_data_pref.isKey("LIFTDATA6")) _lift_data_pref.putDouble("LIFTDATA6", 0.0);
  if (!_lift_data_pref.isKey("LIFTDATA7")) _lift_data_pref.putDouble("LIFTDATA7", 0.0);
  if (!_lift_data_pref.isKey("LIFTDATA8")) _lift_data_pref.putDouble("LIFTDATA8", 0.0);
  if (!_lift_data_pref.isKey("LIFTDATA9")) _lift_data_pref.putDouble("LIFTDATA9", 0.0);
  if (!_lift_data_pref.isKey("LIFTDATA10")) _lift_data_pref.putDouble("LIFTDATA10", 0.0);
  if (!_lift_data_pref.isKey("LIFTDATA11")) _lift_data_pref.putDouble("LIFTDATA11", 0.0);
  if (!_lift_data_pref.isKey("LIFTDATA12")) _lift_data_pref.putDouble("LIFTDATA12", 0.0);

  _lift_data_pref.end();

}






/***********************************************************
* @brief loadLiftDataFile
* @details read lift data from liftdata.json file
***/ 
void DataHandler::loadLiftData () {

  extern struct ValveLiftData valveData;

  DataHandler _data;
  Messages _message;
  Preferences _lift_data_pref;

  _message.serialPrintf("Loading Lift Data \n");     


  _lift_data_pref.begin("liftData", false);

  valveData.LiftData1 = _lift_data_pref.getDouble("LIFTDATA1", 0.0);
  valveData.LiftData2 = _lift_data_pref.getDouble("LIFTDATA2", 0.0);
  valveData.LiftData3 = _lift_data_pref.getDouble("LIFTDATA3", 0.0);
  valveData.LiftData4 = _lift_data_pref.getDouble("LIFTDATA4", 0.0);
  valveData.LiftData5 = _lift_data_pref.getDouble("LIFTDATA5", 0.0);
  valveData.LiftData6 = _lift_data_pref.getDouble("LIFTDATA6", 0.0);
  valveData.LiftData7 = _lift_data_pref.getDouble("LIFTDATA7", 0.0);
  valveData.LiftData8 = _lift_data_pref.getDouble("LIFTDATA8", 0.0);
  valveData.LiftData9 = _lift_data_pref.getDouble("LIFTDATA9", 0.0);
  valveData.LiftData10 = _lift_data_pref.getDouble("LIFTDATA10", 0.0);
  valveData.LiftData11 = _lift_data_pref.getDouble("LIFTDATA11", 0.0);
  valveData.LiftData12 = _lift_data_pref.getDouble("LIFTDATA12", 0.0);

  _lift_data_pref.end();

}





/***********************************************************
* @brief clearLiftDataFile
* @details Delete and recreate default lift data file
***/
void DataHandler::clearLiftDataFile(AsyncWebServerRequest *request){

  DataHandler _data;
  
  if (SPIFFS.exists("/liftdata.json"))  {
    SPIFFS.remove("/liftdata.json");
  }
  
  _data.createLiftDataFile();

  _data.loadLiftData();

  request->redirect("/?view=graph");

}













/***********************************************************
 * @brief getFileListJSON
 * @details Get SPIFFS File List in JSON format
 ***/
String DataHandler::getFileListJSON()
{

  String jsonString;
  String fileName;
  size_t fileSize;

  StaticJsonDocument<1024> dataJson;

  Messages _message;
  Calculations _calculations;

  _message.statusPrintf("Filesystem contents: \n");
  FILESYSTEM.begin();
  File root = FILESYSTEM.open("/");
  File file = root.openNextFile();
  while (file)  {
    fileName = file.name();
    fileSize = file.size();
    dataJson[fileName] = String(fileSize);
    _message.statusPrintf("%s : %s \n", fileName, _calculations.byteDecode(fileSize));
    file = root.openNextFile();
  }
  // FILESYSTEM.end();

  serializeJson(dataJson, jsonString);
  return jsonString;
}



/***********************************************************
 * @brief buildIndexSSEJsonData
 * @details Package up index page data into JSON string
 ***/
String DataHandler::buildIndexSSEJsonData()
{

  extern struct DeviceStatus status;
  extern struct BenchSettings settings;
  extern struct SensorData sensorVal;
  extern struct CalibrationData calVal;

  Hardware _hardware;
  Calculations _calculations;
  Messages _message;

  String jsonString;

  StaticJsonDocument<DATA_JSON_SIZE> dataJson;

  // Reference pressure
  dataJson["PREF"] = sensorVal.PRefH2O;

  double flowComp = fabs(sensorVal.FlowCFM);
  double pRefComp = fabs(sensorVal.PRefH2O);

  // Flow Rate
  if ((flowComp > settings.min_flow_rate) && (pRefComp > settings.min_bench_pressure))  {

    switch (settings.rounding_type) {
      case NONE:
        dataJson["FLOW"] = sensorVal.FlowCFM;
        dataJson["MFLOW"] = sensorVal.FlowKGH;
        dataJson["AFLOW"] = sensorVal.FlowADJ;
        dataJson["SFLOW"] = sensorVal.FlowSCFM;
      break;

      // Round to whole value 
      case INTEGER:
        dataJson["FLOW"] = round(sensorVal.FlowCFM);
        dataJson["MFLOW"] = round(sensorVal.FlowKGH);
        dataJson["AFLOW"] = round(sensorVal.FlowADJ);
        dataJson["SFLOW"] = round(sensorVal.FlowSCFM);
      break;

      // Round to half (nearest 0.5)
      case HALF:
        dataJson["FLOW"] = round(sensorVal.FlowCFM * 2.0 ) / 2.0;
        dataJson["MFLOW"] = round(sensorVal.FlowKGH * 2.0) / 2.0;
        dataJson["AFLOW"] = round(sensorVal.FlowADJ * 2.0) / 2.0;
        dataJson["SFLOW"] = round(sensorVal.FlowSCFM * 2.0) / 2.0;
      break;

      default:
        dataJson["FLOW"] = 0.0;
        dataJson["MFLOW"] = 0.0;
        dataJson["AFLOW"] = 0.0;
        dataJson["SFLOW"] = 0.0;
      break;

    } 

  } else {

    dataJson["FLOW"] = 0.0;
    dataJson["MFLOW"] = 0.0;
    dataJson["AFLOW"] = 0.0;
    dataJson["SFLOW"] = 0.0;
  }




  // Flow depression value for AFLOW units
  dataJson["PADJUST"] = settings.adj_flow_depression;

  // Standard reference
  switch (settings.standardReference) {

    case ISO_1585:
      dataJson["iSTD_REF"] = "ISO-1585";
    break;

    case ISA :
      dataJson["iSTD_REF"] = "ISA";
    break;

    case ISO_13443:
      dataJson["iSTD_REF"] = "ISO-13443";
    break;

    case ISO_5011:
      dataJson["iSTD_REF"] = "ISO-5011";
    break;

    case ISO_2533:
      dataJson["iSTD_REF"] = "ISO-2533";
    break;

  }


  // Temperature deg C or F
  if (settings.temp_unit == CELCIUS) {
    dataJson["TEMP"] = sensorVal.TempDegC;
  } else {
    dataJson["TEMP"] = sensorVal.TempDegF;
  }


  // Bench Type for status pane
  switch (settings.bench_type){

    case MAF:
      dataJson["iBENCH_TYPE"] = "MAF";
    break;

    case ORIFICE:
      dataJson["iBENCH_TYPE"] = "ORIFICE";
    break;

    case VENTURI:
      dataJson["iBENCH_TYPE"] = "VENTURI";
    break;

    case PITOT:
      dataJson["iBENCH_TYPE"] = "PITOT";
    break;

  }




  dataJson["BARO"] = sensorVal.BaroHPA; // GUI  displays mbar (hPa)
  dataJson["RELH"] = sensorVal.RelH;

  // Pitot
  dataJson["PITOT"] = sensorVal.PitotVelocity;
  // dataJson["PITOT_DELTA"] = fabs(round(sensorVal.PitotDelta));
  dataJson["PITOT_DELTA"] = sensorVal.PitotH2O;
  
  if (calVal.pitot_cal_offset == 0) {
    dataJson["PITOT_COLOUR"] = GUI_COLOUR_UNSET;
  } else {
    dataJson["PITOT_COLOUR"] = GUI_COLOUR_SET;
  }

  // Differential pressure
  dataJson["PDIFF"] = sensorVal.PDiffH2O;
  
  if (calVal.pdiff_cal_offset == 0) {
    dataJson["PDIFF_COLOUR"] =  GUI_COLOUR_UNSET;
  } else {
    dataJson["PDIFF_COLOUR"] =  GUI_COLOUR_SET;
  }

  // Swirl (+/- rpm)
  dataJson["SWIRL"] = sensorVal.Swirl;

  // Flow Differential
  dataJson["FDIFF"] = sensorVal.FDiff;
  dataJson["FDIFFTYPEDESC"] = sensorVal.FDiffTypeDesc;

  if (1!=1) {  // TODO if message handler is active display the active message
    dataJson["STATUS_MESSAGE"] = status.statusMessage;
  } else { // else lets just show the uptime
    dataJson["STATUS_MESSAGE"] = "Uptime: " + String(_hardware.uptime()) + " (hh.mm)";      
  }

  // Active Orifice
  dataJson["ACTIVE_ORIFICE"] = status.activeOrifice;
  // Orifice Max Flow
  dataJson["ORIFICE_MAX_FLOW"] = status.activeOrificeFlowRate;
  // Orifice Calibration Depression
  dataJson["ORIFICE_CALIBRATED_DEPRESSION"] = status.activeOrificeTestPressure;

  serializeJson(dataJson, jsonString);

  return jsonString;
}








/***********************************************************
 * @brief buildMimicSSEJsonData
 * @details Package up mimic page data into JSON string
 ***/
String DataHandler::buildMimicSSEJsonData() {

  extern struct DeviceStatus status;
  extern struct BenchSettings settings;
  extern struct SensorData sensorVal;
  extern struct CalibrationData calVal;

  Hardware _hardware;
  Calculations _calculations;
  Messages _message;

  String jsonString;

  StaticJsonDocument<DATA_JSON_SIZE> dataJson;

  // Reference pressure
  dataJson["PREF"] = sensorVal.PRefH2O;

  double flowComp = fabs(sensorVal.FlowCFM);
  double pRefComp = fabs(sensorVal.PRefH2O);

  // Flow Rate
  if ((flowComp > settings.min_flow_rate) && (pRefComp > settings.min_bench_pressure))  {

    switch (settings.rounding_type) {
      case NONE:
        dataJson["FLOW"] = sensorVal.FlowCFM;
        dataJson["MFLOW"] = sensorVal.FlowKGH;
        dataJson["AFLOW"] = sensorVal.FlowADJ;
        dataJson["SFLOW"] = sensorVal.FlowSCFM;
      break;

      // Round to whole value 
      case INTEGER:
        dataJson["FLOW"] = round(sensorVal.FlowCFM);
        dataJson["MFLOW"] = round(sensorVal.FlowKGH);
        dataJson["AFLOW"] = round(sensorVal.FlowADJ);
        dataJson["SFLOW"] = round(sensorVal.FlowSCFM);
      break;

      // Round to half (nearest 0.5)
      case HALF:
        dataJson["FLOW"] = round(sensorVal.FlowCFM * 2.0 ) / 2.0;
        dataJson["MFLOW"] = round(sensorVal.FlowKGH * 2.0) / 2.0;
        dataJson["AFLOW"] = round(sensorVal.FlowADJ * 2.0) / 2.0;
        dataJson["SFLOW"] = round(sensorVal.FlowSCFM * 2.0) / 2.0;
      break;

      default:
        dataJson["FLOW"] = 0.0;
        dataJson["MFLOW"] = 0.0;
        dataJson["AFLOW"] = 0.0;
        dataJson["SFLOW"] = 0.0;
      break;
    }

  }


  return jsonString;
}










// /***********************************************************
//  * @brief Process File Upload
//  ***/
// void DataHandler::fileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
// {

//   Messages _message;
//   DataHandler _data;
//   extern struct DeviceStatus status;

//   int file_size = 0;

//   // _message.serialPrintf("File Upload");

//   if (!filename.startsWith("/")){
//     filename = "/" + filename;
//   }

//   uint32_t freespace = SPIFFS.totalBytes() - SPIFFS.usedBytes();

//   if (!index)  {
//     _message.serialPrintf("UploadStart: %s \n", filename.c_str());
//     request->_tempFile = SPIFFS.open(filename, "w");
//   }

//   if (len)  {
//     file_size += len;
//     if (file_size > freespace)    {
//       _message.serialPrintf("Upload failed, no Space: %s \n", freespace);
//     }    else    {
//       _message.serialPrintf("Writing file: '%s' index=%u len=%u \n", filename.c_str(), index, len);
//       request->_tempFile.write(data, len);
//     }
//   } 

//   if (final)  {
//     _message.serialPrintf("Upload Complete: %s, %u bytes\n", filename.c_str(), file_size);
//     request->_tempFile.close();

//     if (_data.checkUserFile(PINSFILE)){
//       _data.loadPinsData();
//       status.pinsLoaded = true;
//     } 
//     if (_data.checkUserFile(MAFFILE)) {
//       _data.loadMAFData();
//       status.mafLoaded = true;  
//     }
//     if (_data.checkUserFile(INDEXFILE)) {
//       status.GUIexists = true;  
//     }

//     request->redirect("/");
//   }
// }






/***********************************************************
 * @brief getRemote
 * @details read file from remote server
 * @param serverName remote URL of file to be loaded
 * @returns payload: contents of remote file
 * @note Currently no validation is carried out
 ***/
String DataHandler::getRemote(const char* serverName = "https://raw.githubusercontent.com/DeeEmm/DIY-Flow-Bench/refs/heads/DEV/ESP32/DIY-Flow-Bench/version.json") {
  HTTPClient http;

  Messages _message;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    _message.debugPrintf("Downloading remote file: %s/n", serverName);
    _message.debugPrintf("HTTP response code: %u /n", httpResponseCode);
    payload = http.getString();
  }
  else {
    _message.debugPrintf("Downloading failed: %s/n", serverName);
    _message.debugPrintf("HTTP response code: %u /n", httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}



/***********************************************************
 * @brief bootLoop
 * @details Temporary server Traps program pointer in do-while loop 
 * @note waits for missing files to be uploaded and errors to be cleared
 ***/
void DataHandler::bootLoop()
{
    extern struct DeviceStatus status;
    extern struct BenchSettings settings;
    extern struct SensorData sensorVal;

    Hardware _hardware;
    Calculations _calculations;
    Messages _message;
    Webserver _webserver;
    API _api;
    DataHandler _data;

    bool shouldReboot = false;

    // if the weberver is already running skip sever initialisation
    if (!status.webserverIsRunning) {

      _message.serialPrintf("Spinning up temporary web server\n");
      _message.serialPrintf("View browser to upload missing files\n");

      tempServer = new AsyncWebServer(80);
      // tempServerEvents = new AsyncEventSource("/events");

      // Show file handler
      tempServer->on("/api/file", HTTP_POST, [](AsyncWebServerRequest *request) {
          Messages _message;
          Webserver _webserver;
          extern struct Language language;
          _message.debugPrintf("/api/file \n");
          request->send_P(200, "text/html", language.LANG_INDEX_HTML, _webserver.processLandingPageTemplate); 
          }); 

      // save file upload
      tempServer->on("/api/file/save", HTTP_POST, [](AsyncWebServerRequest *request) {
          Messages _message;
          _message.debugPrintf("/api/file/save \n");
          // request->send(200);
          },
          _webserver.fileUpload); 

      // Show pins page
      tempServer->on("/pins", HTTP_POST, [](AsyncWebServerRequest *request) {
          Messages _message;
          Webserver _webserver;
          extern struct Language language;
          _message.debugPrintf("/pins \n");
          request->send_P(200, "text/html", language.LANG_INDEX_HTML, _webserver.processLandingPageTemplate); 
          }); 

      // Save pins form
      tempServer->on("/pins/save", HTTP_POST, [](AsyncWebServerRequest *request) {
          Messages _message;
          _message.debugPrintf("/pins/save \n");
          // request->send(200);
          }); 

      // Index page request handler

      // TODO differentiate between file upload and io settings page
      // if there is an io error we need to redirect to io config page
      // else we need to redirect to the index page

      tempServer->on("/", HTTP_ANY, [](AsyncWebServerRequest *request){
          Language language;
          Webserver _webserver;
              // We should store the upload page in PROGMEM and all it from there.
              // SAME thing for the io config page
              request->send_P(200, "text/html", language.LANG_INDEX_HTML, _webserver.processLandingPageTemplate); 
          });

      tempServer->onFileUpload(_webserver.fileUpload);
      // tempServer->addHandler(tempServerEvents);
      tempServer->begin();

      _message.serialPrintf("Waiting...\n");
    
    }


    do {
    // capture program pointer in loop and wait for files to be uploaded and errors to be cleared

        // Process API comms
        if (settings.api_enabled) {        
            if (millis() > status.apiPollTimer) {
                if (Serial.available() > 0) {
                    status.serialData = Serial.read();
                    _api.ParseMessage(status.serialData);
                }
            }                            
        }

        if (status.ioError) {
          if (_hardware.setPinMode() ) {
            status.ioError = false;
            status.doBootLoop = false;
            break;
          }
        }

        if (status.ioError == false) {
          status.doBootLoop = false; 
          break;
        } 

        if (status.doBootLoop == false) {
          break;
        }

        vTaskDelay( 1000 ); // No need to rush, we'll be stuck here a while...
    
    } while (status.doBootLoop == true);

    // if the weberver is already running, skip sever reset
    if (!status.webserverIsRunning) {
      tempServer->end();  // Stops the temp server and releases the port
      delay(1000);  // 1000ms delay to ensure the port is released
      tempServer->reset();
    }

}

