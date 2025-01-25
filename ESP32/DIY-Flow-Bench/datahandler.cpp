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

    // Preferences _prefs;
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
    
    _hardware.initialisePins();
    _hardware.loadPinsData();

    int pinError = _hardware.setPinMode();

    if (pinError == -1) {
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

    // _hardware.save_ADC_Reg(); // ADC WiFi kludge

    // Initialise WiFi
    _comms.initaliseWifi();

    // BootLoop method traps program pointer within loop until files are uploaded
    if (status.doBootLoop == true) bootLoop();


    // Start Wire (I2C) 
    Wire.begin (pins.SDA, pins.SCL); 
    Wire.setClock(100000);

    // TODO Initialise SD card
    if (config.bSD_ENABLED) {

    // test code from https://github.com/espressif/arduino-esp32/blob/master/libraries/SD/examples/SD_Test/SD_Test.ino

    // _message.serialPrintf("=== SDCARD File system info === \n");

    // int sck = SD_SCK;
    // int miso = SD_MISO;
    // int mosi = SD_MOSI;
    // int cs = SD_CS;

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






/***********************************************************
 * @brief loadJSONFile
 * @details Loads JSON data from file
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
	// 	Serial.begin(SERIAL0_BAUD, SERIAL_8N1 , pins.SERIAL0_RX, pins.SERIAL0_TX); 
	// #endif
	
    Serial.begin(SERIAL0_BAUD);
    _message.serialPrintf("Serial started \n"); 

}




/***********************************************************
* @brief initaliseConfiguration
* @details initialise configiration settings in NVM
* @note Replaces pre-compile macros in original config.h file
* @note Preferences Key can not exceed 15 chars long
* @note Datatype prefix allows for reduced save method
* @note b = bool, i = int, d = double, s = string
***/ 
void DataHandler::initialiseConfig () {

  extern struct Configuration config;

  Messages _message;
  Preferences _prefs;

  _prefs.begin("config");

  // Check if last key exists in NVM
  if (_prefs.isKey("bSWIRL_ENBLD")) {
    // key already exists
    _prefs.end();
    return;
  }

  _message.serialPrintf("Initialising Configuration \n");    

  // _prefs.clear(); // completely remove namepace
  // _prefs.remove("iADC_I2C_ADDR"); // remove individual key

  if (!_prefs.isKey("bSD_ENABLED")) _prefs.putBool("bSD_ENABLED", false);
  if (!_prefs.isKey("iMIN_PRESS_PCT")) _prefs.putInt("iMIN_PRESS_PCT", 80);
  if (!_prefs.isKey("dPIPE_RAD_FT")) _prefs.putDouble("dPIPE_RAD_FT", 0.328084);

  if (!_prefs.isKey("dVCC_3V3_TRIM")) _prefs.putDouble("dVCC_3V3_TRIM", 0.0);
  if (!_prefs.isKey("dVCC_5V_TRIM")) _prefs.putDouble("dVCC_5V_TRIM", 0.0);

  if (!_prefs.isKey("bFIXED_3_3V")) _prefs.putBool("bFIXED_3_3V", true);
  if (!_prefs.isKey("bFIXED_5V")) _prefs.putBool("bFIXED_5V", true);

  if (!_prefs.isKey("bBME_TYPE")) _prefs.putBool("bBME_TYPE", true);
  if (!_prefs.isKey("iBME_ADDR")) _prefs.putInt("iBME_ADDR", 118);
  if (!_prefs.isKey("iBME_SCN_MS")) _prefs.putInt("iBME_SCN_MS", 1000);

  if (!_prefs.isKey("iADC_TYPE")) _prefs.putInt("iADC_TYPE", ADS1115);
  if (!_prefs.isKey("iADC_I2C_ADDR")) _prefs.putInt("iADC_I2C_ADDR", 72);

  if (!_prefs.isKey("iMAF_SENS_TYP")) _prefs.putInt("iMAF_SENS_TYP", SENSOR_DISABLED);
  if (!_prefs.isKey("iMAF_SRC_TYP")) _prefs.putInt("iMAF_SRC_TYP", ADS_ADC);
  if (!_prefs.isKey("dMAF_MV_TRIM")) _prefs.putDouble("dMAF_MV_TRIM", 0.0);

  if (!_prefs.isKey("iPREF_SENS_TYP")) _prefs.putInt("iPREF_SENS_TYP", MPXV7007);
  if (!_prefs.isKey("iPREF_SRC_TYP")) _prefs.putInt("iPREF_SRC_TYP", ADS_ADC);
  if (!_prefs.isKey("dPREF_MV_TRIM")) _prefs.putDouble("dPREF_MV_TRIM", 0.0);

  if (!_prefs.isKey("iPDIFF_SENS_TYP")) _prefs.putInt("iPDIFF_SENS_TYP", MPXV7007);
  if (!_prefs.isKey("iPDIFF_SRC_TYP")) _prefs.putInt("iPDIFF_SRC_TYP", ADS_ADC);
  if (!_prefs.isKey("dPDIFF_MV_TRIM")) _prefs.putDouble("dPDIFF_MV_TRIM", 0.0);

  if (!_prefs.isKey("iPITOT_SENS_TYP")) _prefs.putInt("iPITOT_SENS_TYP", MPXV7007);
  if (!_prefs.isKey("iPITOT_SRC_TYP")) _prefs.putInt("iPITOT_SRC_TYP", ADS_ADC);
  if (!_prefs.isKey("dPITOT_MV_TRIM")) _prefs.putDouble("dPITOT_MV_TRIM", 0.0);

  if (!_prefs.isKey("iBARO_SENS_TYP")) _prefs.putInt("iBARO_SENS_TYP", BOSCH_BME280);
  if (!_prefs.isKey("dBARO_MV_TRIM")) _prefs.putDouble("dBARO_MV_TRIM", 0.0);
  if (!_prefs.isKey("dBARO_FINE_TUNE")) _prefs.putDouble("dBARO_FINE_TUNE", 0.0);
  if (!_prefs.isKey("dSEALEVEL_PRESS")) _prefs.putDouble("dSEALEVEL_PRESS", 0.0);
  if (!_prefs.isKey("iBARO_ADC_CHAN")) _prefs.putInt("iBARO_ADC_CHAN", 4);

  if (!_prefs.isKey("iTEMP_SENS_TYP")) _prefs.putInt("iTEMP_SENS_TYP", BOSCH_BME280);
  if (!_prefs.isKey("dTEMP_MV_TRIM")) _prefs.putDouble("dTEMP_MV_TRIM", 0.0);
  if (!_prefs.isKey("dTEMP_FINE_TUNE")) _prefs.putDouble("dTEMP_FINE_TUNE", 0.0);

  if (!_prefs.isKey("iRELH_SENS_TYP")) _prefs.putInt("iRELH_SENS_TYP", BOSCH_BME280);
  if (!_prefs.isKey("dRELH_MV_TRIM")) _prefs.putDouble("dRELH_MV_TRIM", 0.0);
  if (!_prefs.isKey("dRELH_FINE_TUNE")) _prefs.putDouble("dRELH_FINE_TUNE", 0.0);
  if (!_prefs.isKey("bSWIRL_ENBLD")) _prefs.putBool("bSWIRL_ENBLD", false);

  // Add additional / new keys to bottom of list and update the key check

  _prefs.end();

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
  extern struct DeviceStatus status; 

  Messages _message;
  Preferences _prefs;

  _message.serialPrintf("Loading Configuration \n");    
  
  _prefs.begin("config");

  config.bSD_ENABLED = _prefs.getBool("bSD_ENABLED", false);
  config.iMIN_PRESS_PCT = _prefs.getInt("iMIN_PRESS_PCT", 80);
  config.dPIPE_RAD_FT = _prefs.getDouble("dPIPE_RAD_FT", 0.328084);

  config.dVCC_3V3_TRIM = _prefs.getDouble("dVCC_3V3_TRIM", 0.0);
  config.dVCC_5V_TRIM = _prefs.getDouble("dVCC_5V_TRIM", 0.0);

  config.bFIXED_3_3V = _prefs.getBool("bFIXED_3_3V", true);
  config.bFIXED_5V = _prefs.getBool("bFIXED_5V", true);

  config.iBME_TYP = _prefs.getInt("bBME_TYPE", BOSCH_BME280);
  config.iBME_ADDR = _prefs.getInt("iBME_ADDR", 118);

  config.iADC_TYPE = _prefs.getInt("iADC_TYPE", ADS1115);
  config.iADC_I2C_ADDR = _prefs.getInt("iADC_I2C_ADDR", 72);

  config.iMAF_SENS_TYP = _prefs.getInt("iMAF_SENS_TYP", SENSOR_DISABLED);
  config.iMAF_SRC_TYP = _prefs.getInt("iMAF_SRC_TYP", ADS_ADC);
  config.dMAF_MV_TRIM = _prefs.getDouble("dMAF_MV_TRIM", 0.0);

  config.iPREF_SENS_TYP = _prefs.getInt("iPREF_SENS_TYP", MPXV7007);
  config.iPREF_SRC_TYP = _prefs.getInt("iPREF_SRC_TYP", ADS_ADC);
  config.dPREF_MV_TRIM = _prefs.getDouble("dPREF_MV_TRIM", 0.0);

  config.iPDIFF_SENS_TYP = _prefs.getInt("iPDIFF_SENS_TYP", MPXV7007);
  config.iPDIFF_SRC_TYP = _prefs.getInt("iPDIFF_SRC_TYP", ADS_ADC);
  config.dPDIFF_MV_TRIM = _prefs.getDouble("dPDIFF_MV_TRIM", 0.0);
 
  config.iPITOT_SENS_TYP = _prefs.getInt("iPITOT_SENS_TYP", MPXV7007);
  config.iPITOT_SRC_TYP = _prefs.getInt("iPITOT_SRC_TYP", ADS_ADC);
  config.dPITOT_MV_TRIM = _prefs.getDouble("dPITOT_MV_TRIM", 0.0);

  config.iBARO_SENS_TYP = _prefs.getInt("iBARO_SENS_TYP", BOSCH_BME280);
  config.dBARO_MV_TRIM = _prefs.getDouble("dBARO_MV_TRIM", 1.0);
  config.dBARO_FINE_TUNE = _prefs.getDouble("dBARO_FINE_TUNE", 1.0);
  config.dSEALEVEL_PRESS = _prefs.getDouble("dSEALEVEL_PRESS", 1016.90);
  config.iBARO_ADC_CHAN = _prefs.getInt("iBARO_ADC_CHAN", 4);

  config.iTEMP_SENS_TYP = _prefs.getInt("iTEMP_SENS_TYP", BOSCH_BME280);
  config.dTEMP_MV_TRIM = _prefs.getDouble("dTEMP_MV_TRIM", 0.0);
  config.dTEMP_FINE_TUNE = _prefs.getDouble("dTEMP_FINE_TUNE", 0.0);

  config.iRELH_SENS_TYP = _prefs.getInt("iRELH_SENS_TYP", BOSCH_BME280);
  config.dRELH_MV_TRIM = _prefs.getDouble("dRELH_MV_TRIM", 0.0);
  config.dRELH_FINE_TUNE = _prefs.getDouble("dRELH_FINE_TUNE", 0.0);
  config.bSWIRL_ENBLD = _prefs.getBool("bSWIRL_ENBLD", false);

  status.nvmConfig = _prefs.freeEntries();
  _message.debugPrintf("Config NVM Free Entries: %u \n", status.nvmConfig); 


  _prefs.end();
}










/***********************************************************
* @brief initialiseSettings
* @note Initialise settings in NVM if they do not exist
* @note Key must be 15 chars or shorter.
* @note Datatype prefix allows for reduced save method
* @note b = bool, i = int, d = double, s = string***/ 
void DataHandler::initialiseSettings () {

  extern struct BenchSettings settings;

  DataHandler _data;
  Messages _message;
  Preferences _prefs;

  _message.serialPrintf("Initialising Bench Settings \n");    
  
  _prefs.begin("settings");

    // Check if last key exists in NVM
  if (_prefs.isKey("dORIFICE6_PRESS")) {
    // key already exists
    _prefs.end();
    return;
  }

  // _prefs.remove("iDATA_FLTR_TYP"); // remove individual key
  // _prefs.remove("iTEMP_UNIT"); // remove individual key

  if (!_prefs.isKey("sWIFI_SSID")) _prefs.putString("sWIFI_SSID", "WIFI-SSID");
  if (!_prefs.isKey("sWIFI_PSWD")) _prefs.putString("sWIFI_PSWD", static_cast<String>("PASSWORD"));
  if (!_prefs.isKey("sWIFI_AP_SSID")) _prefs.putString("sWIFI_AP_SSID", static_cast<String>("DIYFB"));
  if (!_prefs.isKey("sWIFI_AP_PSWD")) _prefs.putString("sWIFI_AP_PSWD", static_cast<String>("123456789"));
  if (!_prefs.isKey("sHOSTNAME")) _prefs.putString("sHOSTNAME", static_cast<String>("diyfb"));

  if (!_prefs.isKey("iWIFI_TIMEOUT")) _prefs.putInt("iWIFI_TIMEOUT", 4000);
  if (!_prefs.isKey("iMAF_DIAMETER")) _prefs.putInt("iMAF_DIAMETER", 0);
  if (!_prefs.isKey("iREFRESH_RATE")) _prefs.putInt("iREFRESH_RATE", 500);
  if (!_prefs.isKey("iMIN_PRESSURE")) _prefs.putDouble("iMIN_PRESSURE", 1);
  if (!_prefs.isKey("iMIN_FLOW_RATE")) _prefs.putInt("iMIN_FLOW_RATE", 1);

  if (!_prefs.isKey("iDATA_FLTR_TYP")) _prefs.putInt("iDATA_FLTR_TYP", NONE);
  if (!_prefs.isKey("iROUNDING_TYP")) _prefs.putInt("iROUNDING_TYP", NONE);

  if (!_prefs.isKey("iFLOW_DECI_ACC")) _prefs.putInt("iFLOW_DECI_ACC", 1);
  if (!_prefs.isKey("iGEN_DECI_ACC")) _prefs.putInt("iGEN_DECI_ACC", 2);
  if (!_prefs.isKey("iCYC_AV_BUFF")) _prefs.putInt("iCYC_AV_BUFF", 5);

  if (!_prefs.isKey("sAPI_DELIM")) _prefs.putString("sAPI_DELIM", ":");
  if (!_prefs.isKey("iSHOW_ALARMS")) _prefs.putInt("iSHOW_ALARMS", true);
  if (!_prefs.isKey("iADJ_FLOW_DEP")) _prefs.putInt("iADJ_FLOW_DEP", 28);
  if (!_prefs.isKey("iSTD_REF")) _prefs.putInt("iSTD_REF", 1);
  if (!_prefs.isKey("iSTD_ADJ_FLOW")) _prefs.putInt("iSTD_ADJ_FLOW", 1);
  if (!_prefs.isKey("iDATAGRAPH_MAX")) _prefs.putInt("iDATAGRAPH_MAX", 0);
  if (!_prefs.isKey("iTEMP_UNIT")) _prefs.putInt("iTEMP_UNIT", CELCIUS);

  if (!_prefs.isKey("dLIFT_INTERVAL")) _prefs.putDouble("dLIFT_INTERVAL", 1.5F);
  if (!_prefs.isKey("iBENCH_TYPE")) _prefs.putInt("iBENCH_TYPE", MAF_BENCH);

  // Add additional / new keys to bottom of list and update the key check

  _prefs.end();

}











/***********************************************************
* @brief loadSettings
* @details read settings from NVM and loads into global struct
* @note Datatype prefix allows for reduced save method
* @note b = bool, i = int, d = double, s = string
***/ 
void DataHandler::loadSettings () {

  extern struct BenchSettings settings; 
  extern struct DeviceStatus status;

  // DataHandler _data;
  Messages _message;
  Preferences _prefs;

  _message.serialPrintf("Loading Settings \n");    
  
  _prefs.begin("settings");

  settings.wifi_ssid = _prefs.getString("sWIFI_SSID");
  settings.wifi_pswd = _prefs.getString("sWIFI_PSWD");
  settings.wifi_ap_ssid = _prefs.getString("sWIFI_AP_SSID");
  settings.wifi_ap_pswd =_prefs.getString("sWIFI_AP_PSWD");
  settings.hostname = _prefs.getString("sHOSTNAME", "diyfb" );
  settings.wifi_timeout = _prefs.getInt("iWIFI_TIMEOUT", 4000 );
  settings.maf_housing_diameter = _prefs.getInt("iMAF_DIAMETER", 0 );
  settings.refresh_rate = _prefs.getInt("iREFRESH_RATE", 500 );
  settings.min_bench_pressure  = _prefs.getDouble("iMIN_PRESSURE", 1 );
  settings.min_flow_rate = _prefs.getInt("iMIN_FLOW_RATE", 1 );
  settings.data_filter_type = _prefs.getInt("iDATA_FLTR_TYP", NONE );
  settings.rounding_type = _prefs.getInt("iROUNDING_TYP", NONE );
  settings.flow_decimal_length = _prefs.getInt("iFLOW_DECI_ACC", 1 );
  settings.gen_decimal_length = _prefs.getInt("iGEN_DECI_ACC", 2 );
  settings.cyc_av_buffer  = _prefs.getInt("iCYC_AV_BUFF", 5 );
  settings.show_alarms = _prefs.getInt("iSHOW_ALARMS",  true  );
  settings.adj_flow_depression = _prefs.getInt("iADJ_FLOW_DEP",  28  );
  settings.standardReference = _prefs.getInt("iSTD_REF", 1  );
  settings.std_adj_flow = _prefs.getInt("iSTD_ADJ_FLOW",  1 );
  settings.dataGraphMax = _prefs.getInt("iDATAGRAPH_MAX", 0 );
  settings.temp_unit = _prefs.getInt("iTEMP_UNIT", CELCIUS );
  settings.valveLiftInterval = _prefs.getDouble("dLIFT_INTERVAL", 1.5F  );
  settings.bench_type = _prefs.getInt("iBENCH_TYPE", MAF_BENCH );


  status.nvmSettings = _prefs.freeEntries();
  _message.debugPrintf("Settings NVM Free Entries: %u \n", status.nvmSettings); 

  _prefs.end();

}






/***********************************************************
* @brief initialiseLiftData
* @note - Initialise settings in NVM if they do not exist
* @note Key must be 15 chars or shorter.
***/ 
void DataHandler::initialiseLiftData () {

  Messages _message;
  Preferences _prefs;

  _message.serialPrintf("Initialising Lift Data \n");    
  
  _prefs.begin("liftData");

  if (!_prefs.isKey("LIFTDATA1")) _prefs.putDouble("LIFTDATA1", 0.0);
  if (!_prefs.isKey("LIFTDATA2")) _prefs.putDouble("LIFTDATA2", 0.0);
  if (!_prefs.isKey("LIFTDATA3")) _prefs.putDouble("LIFTDATA3", 0.0);
  if (!_prefs.isKey("LIFTDATA4")) _prefs.putDouble("LIFTDATA4", 0.0);
  if (!_prefs.isKey("LIFTDATA5")) _prefs.putDouble("LIFTDATA5", 0.0);
  if (!_prefs.isKey("LIFTDATA6")) _prefs.putDouble("LIFTDATA6", 0.0);
  if (!_prefs.isKey("LIFTDATA7")) _prefs.putDouble("LIFTDATA7", 0.0);
  if (!_prefs.isKey("LIFTDATA8")) _prefs.putDouble("LIFTDATA8", 0.0);
  if (!_prefs.isKey("LIFTDATA9")) _prefs.putDouble("LIFTDATA9", 0.0);
  if (!_prefs.isKey("LIFTDATA10")) _prefs.putDouble("LIFTDATA10", 0.0);
  if (!_prefs.isKey("LIFTDATA11")) _prefs.putDouble("LIFTDATA11", 0.0);
  if (!_prefs.isKey("LIFTDATA12")) _prefs.putDouble("LIFTDATA12", 0.0);

  _prefs.end();

}






/***********************************************************
* @brief loadLiftDataFile
* @details read lift data from liftdata.json file
***/ 
void DataHandler::loadLiftData () {

  extern struct ValveLiftData valveData;

  DataHandler _data;
  Messages _message;
  Preferences _prefs;

  _message.serialPrintf("Loading Lift Data \n");     

  _prefs.begin("liftData");

  valveData.LiftData1 = _prefs.getDouble("LIFTDATA1", 0.0);
  valveData.LiftData2 = _prefs.getDouble("LIFTDATA2", 0.0);
  valveData.LiftData3 = _prefs.getDouble("LIFTDATA3", 0.0);
  valveData.LiftData4 = _prefs.getDouble("LIFTDATA4", 0.0);
  valveData.LiftData5 = _prefs.getDouble("LIFTDATA5", 0.0);
  valveData.LiftData6 = _prefs.getDouble("LIFTDATA6", 0.0);
  valveData.LiftData7 = _prefs.getDouble("LIFTDATA7", 0.0);
  valveData.LiftData8 = _prefs.getDouble("LIFTDATA8", 0.0);
  valveData.LiftData9 = _prefs.getDouble("LIFTDATA9", 0.0);
  valveData.LiftData10 = _prefs.getDouble("LIFTDATA10", 0.0);
  valveData.LiftData11 = _prefs.getDouble("LIFTDATA11", 0.0);
  valveData.LiftData12 = _prefs.getDouble("LIFTDATA12", 0.0);

  _prefs.end();

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

    case MAF_BENCH:
      dataJson["iBENCH_TYPE"] = "MAF";
    break;

    case ORIFICE_BENCH:
      dataJson["iBENCH_TYPE"] = "ORIFICE";
    break;

    case VENTURI_BENCH:
      dataJson["iBENCH_TYPE"] = "VENTURI";
    break;

    case PITOT_BENCH:
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

  // // Flow Differential
  dataJson["FDIFF"] = sensorVal.FDiff;
  dataJson["FDIFFTYPEDESC"] = sensorVal.FDiffTypeDesc;
  // Also need to update USER_OFFSET modal value

  // sensorVal.FDiffType = 2;


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

  // Decimal accuracy
  dataJson["FLOW_DECIMAL_ACCURACY"] = settings.flow_decimal_length;
  dataJson["GEN_DECIMAL_ACCURACY"] = settings.gen_decimal_length;

  serializeJson(dataJson, jsonString);

  return jsonString;
}








/***********************************************************
 * @brief buildMimicSSEJsonData
 * @details Package up mimic page data into JSON string to send via SSE
 ***/
String DataHandler::buildMimicSSEJsonData() {

  extern struct DeviceStatus status;
  // extern struct BenchSettings settings;
  extern struct SensorData sensorVal;
  // extern struct CalibrationData calVal;
  extern struct Configuration config;

  extern TaskHandle_t sensorDataTask;
  extern TaskHandle_t enviroDataTask;

  Hardware _hardware;
  Calculations _calculations;

  String jsonString;

  StaticJsonDocument<DATA_JSON_SIZE> dataJson;

  dataJson["MAF_ADC"] = _hardware.getADCRawData(config.iMAF_ADC_CHAN);
  dataJson["PREF_ADC"] = _hardware.getADCRawData(config.iPREF_ADC_CHAN);
  dataJson["PDIFF_ADC"] = _hardware.getADCRawData(config.iPDIFF_ADC_CHAN);
  dataJson["PITOT_ADC"] = _hardware.getADCRawData(config.iPITOT_ADC_CHAN);

  dataJson["MAF_VOLTS"] = sensorVal.MafVolts;
  dataJson["PREF_VOLTS"] = sensorVal.PRefVolts;
  dataJson["PDIFF_VOLTS"] = sensorVal.PDiffVolts;
  dataJson["PITOT_VOLTS"] = sensorVal.PitotVolts;

  dataJson["VCC_3V3_BUS"] = sensorVal.VCC_3V3_BUS;
  dataJson["VCC_5V_BUS"] = sensorVal.VCC_5V_BUS;

  dataJson["FLOW_KG_H"] = sensorVal.FlowKGH;
  dataJson["FLOW_MG_S"] = _calculations.convertMassFlowUnits(sensorVal.FlowKGH, KG_H, MG_S);
  dataJson["FLOW_CFM"] = sensorVal.FlowCFM;
  dataJson["FLOW_LPM"] = _calculations.convertVolumetricFlowUnits(sensorVal.FlowCFM, CFM, LPM);

  dataJson["LOOP_SCAN_TIME"] = status.loopScanTime;
  dataJson["BME_SCAN_COUNT"] = status.bmeScanCountAverage;
  dataJson["ADC_SCAN_COUNT"] = status.adcScanCountAverage;
  dataJson["BME_SCAN_FREQ"] = status.bmeScanTime;
  dataJson["ADC_SCAN_FREQ"] = status.adcScanTime;
  dataJson["FREE_HEAP"] = _calculations.byteDecode(ESP.getFreeHeap());
  dataJson["FREE_STACK"] = _calculations.byteDecode(ESP.getMaxAllocHeap());
  dataJson["BME_TASK_HWM"] = _calculations.byteDecode(uxTaskGetStackHighWaterMark(enviroDataTask));
  dataJson["ADC_TASK_HWM"] = _calculations.byteDecode(uxTaskGetStackHighWaterMark(sensorDataTask));

  serializeJson(dataJson, jsonString);

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
          request->send(200, "text/html", language.LANG_INDEX_HTML, _webserver.processLandingPageTemplate); 
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
          request->send(200, "text/html", language.LANG_INDEX_HTML, _webserver.processLandingPageTemplate); 
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
              request->send(200, "text/html", language.LANG_INDEX_HTML, _webserver.processLandingPageTemplate); 
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
          if (_hardware.setPinMode() == -1) {
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

