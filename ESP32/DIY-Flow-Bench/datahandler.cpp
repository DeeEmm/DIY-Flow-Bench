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
    if (config.SD_ENABLED) {

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
    _message.serialPrintf("Total space:      %s \n", byteDecode(status.spiffs_mem_size));
    _message.serialPrintf("Total space used: %s \n", byteDecode(status.spiffs_mem_used));

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



// DEPRECATED
// /***********************************************************
// * @name checkUserFile.cpp
// * @brief Loop through all files in SPIFFS to match filename prefix
// * @param filetype INT (MAFFILE or PINSFILE [default])
// * @returns true if file found
// * @see status.pinsFilename 
// * @see variable status.mafFilename 
// ***/
// bool DataHandler::checkUserFile(int filetype) {
      
// // TODO Change to accept prefix as arguement

//     DataHandler _data;
//     Messages _message;
//     extern struct DeviceStatus status;

//     FILESYSTEM.begin();
//     File root = FILESYSTEM.open("/");
//     File file = root.openNextFile();

//     std::string matchCONFIG = "config";
//     std::string matchPINS = "PINS";
//     std::string matchMAF = "MAF";
//     std::string matchINDEX = "index";
//     std::string spiffsFile;
//     std::string configFile;
//     std::string pinsFile;
//     std::string mafFile;
//     std::string indexFile;

//     // Check SPIFFS for pins and MAF files
//     while (file)  {
//       spiffsFile = file.name();

//       // Check config file
//       if (checkSubstring(spiffsFile.c_str(), matchCONFIG.c_str()) && (filetype == CONFIGFILE)) {
//         configFile = "/" + spiffsFile;
//         _message.serialPrintf("CONFIG file Found: %s\n", configFile.c_str() );  
//         status.pinsFilename = configFile.c_str();        
//         // loadConfiguration();
//         status.configLoaded = true;
//         return true;
//       }   

//       // Check PINS file      
//       if (checkSubstring(spiffsFile.c_str(), matchPINS.c_str()) && (filetype == PINSFILE)) {
//         pinsFile = "/" + spiffsFile;
//         _message.serialPrintf("PINS file Found: %s\n", pinsFile.c_str() );  
//         status.pinsFilename = pinsFile.c_str();        
//         // loadPinsData();
//         status.pinsLoaded = true;
//         return true;
//       }   
      
//       // Check MAF file
//       if (checkSubstring(spiffsFile.c_str(), matchMAF.c_str()) && (filetype == MAFFILE)) {
//         mafFile = "/" + spiffsFile;
//         _message.serialPrintf("MAF file Found: %s\n", mafFile.c_str() );  
//         status.mafFilename = mafFile.c_str();
//         // loadMAFData();
//         status.mafLoaded = true;
//         return true;
//       }
      
//       // // Check index file
//       // if (checkSubstring(spiffsFile.c_str(), matchINDEX.c_str()) && (filetype == INDEXFILE)) {
//       //   indexFile = "/" + spiffsFile;
//       //   _message.serialPrintf("Index file Found: %s\n", indexFile.c_str() );  
//       //   status.indexFilename = indexFile.c_str();
//       //   status.GUIexists = true;
//       //   return true;
//       // }

//       file = root.openNextFile();
//     }



//     return false;

// }



// /***********************************************************
// * @brief createConfig
// * @details Create basic minimum configuration json file
// * @note Called from DataHandler::begin() if settings.json not found
// ***/
// void DataHandler::createSettingsFile() {

//   extern struct BenchSettings settings;
//   Messages _message;
//   String jsonString;  
//   StaticJsonDocument<CONFIG_JSON_SIZE> configData;

//   _message.serialPrintf("Creating settings.json file... \n"); 
 
//   configData["PAGE_TITLE"] = settings.pageTitle;
//   configData["WIFI_SSID"] = settings.wifi_ssid;
//   configData["WIFI_PSWD"] = settings.wifi_pswd;
//   configData["WIFI_AP_SSID"] = settings.wifi_ap_ssid;
//   configData["WIFI_AP_PSWD"] = settings.wifi_ap_pswd;
//   configData["HOSTNAME"] = settings.hostname;
//   configData["WIFI_TIMEOUT"] = settings.wifi_timeout;
//   configData["MAF_HOUSING_DIA"] = settings.maf_housing_diameter;
//   configData["REFRESH_RATE"] = settings.refresh_rate;
//   configData["MIN_BENCH_PRESS"] = settings.min_bench_pressure;
//   configData["MIN_FLOW_RATE"] = settings.min_flow_rate;
//   configData["DATA_FILTER_TYP"] = settings.data_filter_type;
//   configData["ROUNDING_TYPE"] = settings.rounding_type;
//   configData["FLOW_DECI_ACC"] = settings.flow_decimal_length;
//   configData["GEN_DECI_ACC"] = settings.gen_decimal_length;
//   configData["CYCLIC_AV_BUFF"] = settings.cyc_av_buffer;
//   configData["MAF_MIN_VOLTS"] = settings.maf_min_volts;
//   configData["API_DELIM"] = settings.api_delim;
//   configData["SERIAL_BAUDRATE"] = settings.serial_baud_rate;
//   configData["ADJ_FLOW_DEP"] = settings.adj_flow_depression;
//   configData["STD_REF"] = settings.standardReference;
//   configData["STD_ADJ_FLOW"] = settings.std_adj_flow;
//   configData["DATAGRAPH_MAX"] = settings.dataGraphMax;
//   configData["TEMP_UNIT"] = settings.temp_unit;
//   configData["LIFT_INTERVAL"] = settings.valveLiftInterval;
//   configData["SHOW_ALARMS"] = settings.show_alarms;
//   configData["BENCH_TYPE"] = settings.bench_type;
//   configData["CAL_FLOW_RATE"] = settings.cal_flow_rate;
//   configData["CAL_REF_PRESS"] = settings.cal_ref_press;
//   configData["ORIFICE1_FLOW"] = settings.orificeOneFlow;
//   configData["ORIFICE1_PRESS"] = settings.orificeOneDepression;
//   configData["ORIFICE2_FLOW"] = settings.orificeTwoFlow;
//   configData["ORIFICE2_PRESS"] = settings.orificeThreeDepression;
//   configData["ORIFICE3_FLOW"] = settings.orificeThreeFlow;
//   configData["ORIFICE4_FLOW"] = settings.orificeFourFlow;
//   configData["ORIFICE4_PRESS"] = settings.orificeFourDepression;
//   configData["ORIFICE5_FLOW"] = settings.orificeFiveFlow;
//   configData["ORIFICE5_PRESS"] = settings.orificeFiveDepression;
//   configData["ORIFICE6_FLOW"] = settings.orificeSixFlow;
//   configData["ORIFICE7_TEST_PRESSURE"] = settings.orificeSixDepression;

//   serializeJsonPretty(configData, jsonString);
//   writeJSONFile(jsonString, "/settings.json", CONFIG_JSON_SIZE);

// }




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
***/ 
void DataHandler::initialiseConfig () {

  extern struct Configuration config;

  Messages _message;
  Preferences _config_pref;

  _config_pref.begin("config", false);

  if (_config_pref.isKey("SWIRL_ENABLED")) { // we've already initialised _config_pref
    _config_pref.end();
    return;
  }

  _message.serialPrintf("Initialising Configuration \n");    

  // _config_pref.clear(); // completely remove namepace
  // _config_pref.remove("ADC_I2C_ADDR"); // remove individual key

  if (!_config_pref.isKey("SD_ENABLED")) _config_pref.putBool("SD_ENABLED", false);
  if (!_config_pref.isKey("MIN_PRESS_PCNT")) _config_pref.putInt("MIN_PRESS_PCNT", 80);
  if (!_config_pref.isKey("PIPE_RAD_FT")) _config_pref.putDouble("PIPE_RAD_FT", 0.328084);

  if (!_config_pref.isKey("VCC_3V3_TRIM")) _config_pref.putDouble("VCC_3V3_TRIM", 0.0);
  if (!_config_pref.isKey("VCC_5V_TRIM")) _config_pref.putDouble("VCC_5V_TRIM", 0.0);
  if (!_config_pref.isKey("FIXED_3_3V_VAL")) _config_pref.putBool("FIXED_3_3V_VAL", true);
  if (!_config_pref.isKey("FIXED_5V_VAL")) _config_pref.putBool("FIXED_5V_VAL", true);

  if (!_config_pref.isKey("BME280_ENABLED")) _config_pref.putBool("BME280_ENABLED", true);
  if (!_config_pref.isKey("BME280_I2C_ADDR")) _config_pref.putInt("BME280_I2C_ADDR", 118);
  if (!_config_pref.isKey("BME280_SCAN_MS")) _config_pref.putInt("BME280_SCAN_MS", 1000);

  if (!_config_pref.isKey("BME680_ENABLED")) _config_pref.putBool("BME680_ENABLED", true);
  if (!_config_pref.isKey("BME680_I2C_ADDR")) _config_pref.putInt("BME680_I2C_ADDR", 119);
  if (!_config_pref.isKey("BME680_SCAN_MS")) _config_pref.putInt("BME680_SCAN_MS", 1000);

  if (!_config_pref.isKey("ADC_TYPE")) _config_pref.putInt("ADC_TYPE", 11);
  if (!_config_pref.isKey("ADC_I2C_ADDR")) _config_pref.putInt("ADC_I2C_ADDR", 72);
  if (!_config_pref.isKey("ADC_SCAN_DELAY")) _config_pref.putInt("ADC_SCAN_DELAY", 1000);
  if (!_config_pref.isKey("ADC_MAX_RETRIES")) _config_pref.putInt("ADC_MAX_RETRIES", 10);
  if (!_config_pref.isKey("ADC_RANGE")) _config_pref.putInt("ADC_RANGE", 32767);
  if (!_config_pref.isKey("ADC_GAIN")) _config_pref.putDouble("ADC_GAIN", 6.144);

  if (!_config_pref.isKey("MAF_SRC_TYPE")) _config_pref.putInt("MAF_SRC_TYPE", 11);
  if (!_config_pref.isKey("MAF_SENS_TYPE")) _config_pref.putString("MAF_SENS_TYPE", 0);
  if (!_config_pref.isKey("MAF_MV_TRIM")) _config_pref.putDouble("MAF_MV_TRIM", 0.0);
  if (!_config_pref.isKey("MAF_ADC_CHAN")) _config_pref.putInt("MAF_ADC_CHAN", 0);

  if (!_config_pref.isKey("PREF_SENS_TYPE")) _config_pref.putInt("PREF_SENS_TYPE", 4);
  if (!_config_pref.isKey("PREF_SRC_TYPE")) _config_pref.putInt("PREF_SRC_TYPE", 11);
  if (!_config_pref.isKey("FIXED_PREF_VAL")) _config_pref.putInt("FIXED_PREF_VAL", 1);
  if (!_config_pref.isKey("PREF_MV_TRIM")) _config_pref.putDouble("PREF_MV_TRIM", 0.0);
  if (!_config_pref.isKey("PREF_ALOG_SCALE")) _config_pref.putDouble("PREF_ALOG_SCALE", 0.0);
  if (!_config_pref.isKey("PREF_ADC_CHAN")) _config_pref.putInt("PREF_ADC_CHAN", 1);

  if (!_config_pref.isKey("PDIFF_SENS_TYPE")) _config_pref.putInt("PDIFF_SENS_TYPE", 4);
  if (!_config_pref.isKey("PDIFF_SRC_TYPE")) _config_pref.putInt("PDIFF_SRC_TYPE", 11);
  if (!_config_pref.isKey("FIXED_PDIFF_VAL")) _config_pref.putInt("FIXED_PDIFF_VAL", 1);
  if (!_config_pref.isKey("PDIFF_MV_TRIM")) _config_pref.putDouble("PDIFF_MV_TRIM", 0.0);
  if (!_config_pref.isKey("PDIFF_SCALE")) _config_pref.putDouble("PDIFF_SCALE", 0.0);
  if (!_config_pref.isKey("PDIFF_ADC_CHAN")) _config_pref.putInt("PDIFF_ADC_CHAN", 1);

  if (!_config_pref.isKey("PITOT_SENS_TYPE")) _config_pref.putInt("PITOT_SENS_TYPE", 4);
  if (!_config_pref.isKey("PITOT_SRC_TYPE")) _config_pref.putInt("PITOT_SRC_TYPE", 11);
  if (!_config_pref.isKey("PITOT_MV_TRIM")) _config_pref.putDouble("PITOT_MV_TRIM", 0.0);
  if (!_config_pref.isKey("PITOT_SCALE")) _config_pref.putDouble("PITOT_SCALE", 0.0);
  if (!_config_pref.isKey("PITOT_ADC_CHAN")) _config_pref.putInt("PITOT_ADC_CHAN", 1);

  if (!_config_pref.isKey("BARO_SENS_TYPE")) _config_pref.putInt("BARO_SENS_TYPE", BOSCH_BME280);
  if (!_config_pref.isKey("FIXED_BARO_VAL")) _config_pref.putDouble("FIXED_BARO_VAL", 101.3529);
  if (!_config_pref.isKey("BARO_ALOG_SCALE")) _config_pref.putDouble("BARO_ALOG_SCALE", 1.0);
  if (!_config_pref.isKey("BARO_MV_TRIM")) _config_pref.putDouble("BARO_MV_TRIM", 1.0);
  if (!_config_pref.isKey("BARO_FINE_TUNE")) _config_pref.putDouble("BARO_FINE_TUNE", 1.0);
  if (!_config_pref.isKey("BARO_SCALE")) _config_pref.putDouble("BARO_SCALE", 1.0);
  if (!_config_pref.isKey("BARO_OFFSET")) _config_pref.putDouble("BARO_OFFSET", 1.0);
  if (!_config_pref.isKey("SEALEVEL_PRESS")) _config_pref.putDouble("SEALEVEL_PRESS", 0.0);
  if (!_config_pref.isKey("BARO_ADC_CHAN")) _config_pref.putInt("BARO_ADC_CHAN", 4);

  if (!_config_pref.isKey("TEMP_SENS_TYPE")) _config_pref.putInt("TEMP_SENS_TYPE", BOSCH_BME280);
  if (!_config_pref.isKey("FIXED_TEMP_VAL")) _config_pref.putDouble("FIXED_TEMP_VAL", 21.0);
  if (!_config_pref.isKey("TEMP_ALOG_SCALE")) _config_pref.putDouble("TEMP_ALOG_SCALE", 1.0);
  if (!_config_pref.isKey("TEMP_MV_TRIM")) _config_pref.putDouble("TEMP_MV_TRIM", 1.0);
  if (!_config_pref.isKey("TEMP_FINE_TUNE")) _config_pref.putDouble("TEMP_FINE_TUNE", 1.0);

  if (!_config_pref.isKey("RELH_SENS_TYPE")) _config_pref.putInt("RELH_SENS_TYPE", BOSCH_BME280);
  if (!_config_pref.isKey("FIXED_RELH_VAL")) _config_pref.putDouble("FIXED_RELH_VAL", 36.0);
  if (!_config_pref.isKey("RELH_ALOG_SCALE")) _config_pref.putDouble("RELH_ALOG_SCALE", 1.0);
  if (!_config_pref.isKey("RELH_MV_TRIM")) _config_pref.putDouble("RELH_MV_TRIM", 1.0);
  if (!_config_pref.isKey("RELH_FINE_TUNE")) _config_pref.putDouble("RELH_FINE_TUNE", 1.0);
  if (!_config_pref.isKey("SWIRL_ENABLED")) _config_pref.putBool("SWIRL_ENABLED", false);

  _config_pref.end();

}




/***********************************************************
* @brief loadConfiguration
* @details read settings from ESP32 NVM and loads into global struct
* @note Replaces pre-compile macros in original config.h file
* @note Preferences Kay can not exceed 15 chars long
***/ 
void DataHandler::loadConfig () {

  extern struct Configuration config;

  Messages _message;
  Preferences _config_pref;

  _message.serialPrintf("Loading Configuration \n");    
  
  _config_pref.begin("config", true);

  config.SD_ENABLED = _config_pref.getBool("SD_ENABLED", false);
  config.MIN_PRESS_PCNT = _config_pref.getInt("MIN_PRESS_PCNT", 80);
  config.PIPE_RAD_FT = _config_pref.getDouble("PIPE_RAD_FT", 0.328084);

  config.VCC_3V3_TRIM = _config_pref.getDouble("VCC_3V3_TRIM", 0.0);
  config.VCC_5V_TRIM = _config_pref.getDouble("VCC_5V_TRIM", 0.0);
  config.FIXED_3_3V_VAL = _config_pref.getBool("FIXED_3_3V_VAL", true);
  config.FIXED_5V_VAL = _config_pref.getBool("FIXED_5V_VAL", true);

  config.BME280_ENABLED = _config_pref.getBool("BME280_ENABLED", true);
  config.BME280_I2C_ADDR = _config_pref.getInt("BME280_I2C_ADDR", 118);
  config.BME280_SCAN_MS = _config_pref.getInt("BME280_SCAN_MS", 1000);

  config.BME680_ENABLED = _config_pref.getBool("BME680_ENABLED", false);
  config.BME680_I2C_ADDR = _config_pref.getInt("BME680_I2C_ADDR", 119);
  config.BME680_SCAN_MS = _config_pref.getInt("BME680_SCAN_MS", 1000);

  config.ADC_TYPE = _config_pref.getInt("ADC_TYPE", 11);
  config.ADC_I2C_ADDR = _config_pref.getInt("ADC_I2C_ADDR", 72);
  config.ADC_SCAN_DELAY = _config_pref.getInt("ADC_SCAN_DELAY", 1000);
  config.ADC_MAX_RETRIES  = _config_pref.getInt("ADC_MAX_RETRIES", 10);
  config.ADC_RANGE = _config_pref.getInt("ADC_RANGE", 32767);
  config.ADC_GAIN = _config_pref.getDouble("ADC_GAIN", 6.144);

  config.MAF_SRC_TYPE = _config_pref.getInt("MAF_SRC_TYPE", 11);
  config.MAF_SENS_TYPE = _config_pref.getInt("MAF_SENS_TYPE", 0);
  config.MAF_MV_TRIM = _config_pref.getDouble("MAF_MV_TRIM", 0.0);
  config.MAF_ADC_CHAN = _config_pref.getInt("MAF_ADC_CHAN", 0);

  config.PREF_SENS_TYPE = _config_pref.getInt("PREF_SENS_TYPE", 4);
  config.PREF_SRC_TYPE = _config_pref.getInt("PREF_SRC_TYPE", 11);
  config.FIXED_PREF_VAL = _config_pref.getInt("FIXED_PREF_VAL", 1);
  config.PREF_MV_TRIM = _config_pref.getDouble("PREF_MV_TRIM", 0.0);
  config.PREF_ALOG_SCALE = _config_pref.getDouble("PREF_ALOG_SCALE", 1.0);
  config.PREF_ADC_CHAN = _config_pref.getInt("PREF_ADC_CHAN", 1);

  config.PDIFF_SENS_TYPE = _config_pref.getInt("PDIFF_SENS_TYPE", 4);
  config.PDIFF_SRC_TYPE = _config_pref.getInt("PDIFF_SRC_TYPE", 11);
  config.FIXED_PDIFF_VAL = _config_pref.getInt("FIXED_PDIFF_VAL", 1);
  config.PDIFF_MV_TRIM = _config_pref.getDouble("PDIFF_MV_TRIM", 0.0);
  config.PDIFF_SCALE = _config_pref.getDouble("PDIFF_SCALE", 1.0);
  config.PDIFF_ADC_CHAN = _config_pref.getInt("PDIFF_ADC_CHAN", 2);
 
  config.PITOT_SENS_TYPE = _config_pref.getInt("PITOT_SENS_TYPE", SENSOR_DISABLED);
  config.PITOT_SRC_TYPE = _config_pref.getInt("PITOT_SRC_TYPE", 11);
  config.PITOT_MV_TRIM = _config_pref.getDouble("PITOT_MV_TRIM", 0.0);
  config.PITOT_SCALE = _config_pref.getDouble("PITOT_SCALE", 1.0);
  config.PITOT_ADC_CHAN = _config_pref.getInt("PITOT_ADC_CHAN", 3);

  config.BARO_SENS_TYPE = _config_pref.getInt("BARO_SENS_TYPE", BOSCH_BME280);
  config.FIXED_BARO_VAL = _config_pref.getDouble("FIXED_BARO_VAL", 101.3529);
  config.BARO_ALOG_SCALE =_config_pref.getDouble("BARO_ALOG_SCALE", 1.0);
  config.BARO_MV_TRIM = _config_pref.getDouble("BARO_MV_TRIM", 1.0);
  config.BARO_FINE_TUNE = _config_pref.getDouble("BARO_FINE_TUNE", 1.0);
  config.BARO_SCALE = _config_pref.getDouble("BARO_SCALE", 100);
  config.BARO_OFFSET = _config_pref.getDouble("BARO_OFFSET", 100);
  config.SEALEVEL_PRESS = _config_pref.getDouble("SEALEVEL_PRESS", 1016.90);
  config.BARO_ADC_CHAN = _config_pref.getInt("BARO_ADC_CHAN", 4);

  config.TEMP_SENS_TYPE = _config_pref.getInt("TEMP_SENS_TYPE", BOSCH_BME280);
  config.FIXED_TEMP_VAL = _config_pref.getDouble("FIXED_TEMP_VAL", 21.0);
  config.TEMP_ALOG_SCALE = _config_pref.getDouble("TEMP_ALOG_SCALE", 1.0);
  config.TEMP_MV_TRIM = _config_pref.getDouble("TEMP_MV_TRIM", 0.0);
  config.TEMP_FINE_TUNE = _config_pref.getDouble("TEMP_FINE_TUNE", 0.0);

  config.RELH_SENS_TYPE = _config_pref.getInt("RELH_SENS_TYPE", BOSCH_BME280);
  config.FIXED_RELH_VAL = _config_pref.getDouble("FIXED_RELH_VAL", 36.0);
  config.RELH_ALOG_SCALE = _config_pref.getDouble("RELH_ALOG_SCALE", 1.0);
  config.RELH_MV_TRIM = _config_pref.getDouble("RELH_MV_TRIM", 0.0);
  config.RELH_FINE_TUNE = _config_pref.getDouble("RELH_FINE_TUNE", 0.0);
  config.SWIRL_ENABLED = _config_pref.getBool("SWIRL_ENABLED", false);

  _config_pref.end();
}










/***********************************************************
* @brief initialiseSettings
* @note - Initialise settings in NVM if they do not exist
* @note Key must be 15 chars or shorter.
***/ 
void DataHandler::initialiseSettings () {

  extern struct BenchSettings settings;

  DataHandler _data;
  Messages _message;
  Preferences _settings_pref;

  _message.serialPrintf("Loading Bench Settings \n");    
  
  _settings_pref.begin("settings", false);

  // _settings_pref.remove("DATA_FILTER_TYP"); // remove individual key
  // _settings_pref.remove("TEMP_UNIT"); // remove individual key

  if (!_settings_pref.isKey("WIFI_SSID")) _settings_pref.putString("WIFI_SSID", "WIFI-SSID");
  if (!_settings_pref.isKey("WIFI_PSWD")) _settings_pref.putString("WIFI_PSWD", static_cast<String>("PASSWORD"));
  if (!_settings_pref.isKey("WIFI_AP_SSID")) _settings_pref.putString("WIFI_AP_SSID", static_cast<String>("DIYFB"));
  if (!_settings_pref.isKey("WIFI_AP_PSWD")) _settings_pref.putString("WIFI_AP_PSWD", static_cast<String>("123456789"));
  if (!_settings_pref.isKey("HOSTNAME")) _settings_pref.putString("HOSTNAME", static_cast<String>("diyfb"));

  if (!_settings_pref.isKey("WIFI_TIMEOUT")) _settings_pref.putInt("WIFI_TIMEOUT", 4000);
  if (!_settings_pref.isKey("MAF_HOUSING_DIA")) _settings_pref.putInt("MAF_HOUSING_DIA", 0);
  if (!_settings_pref.isKey("REFRESH_RATE")) _settings_pref.putInt("REFRESH_RATE", 500);
  if (!_settings_pref.isKey("MIN_BENCH_PRESS")) _settings_pref.putInt("MIN_BENCH_PRESS", 1);
  if (!_settings_pref.isKey("MIN_FLOW_RATE")) _settings_pref.putInt("MIN_FLOW_RATE", 1);

  if (!_settings_pref.isKey("DATA_FILTER_TYP")) _settings_pref.putInt("DATA_FILTER_TYP", NONE);
  if (!_settings_pref.isKey("ROUNDING_TYPE")) _settings_pref.putInt("ROUNDING_TYPE", NONE);

  if (!_settings_pref.isKey("FLOW_DECI_ACC")) _settings_pref.putInt("FLOW_DECI_ACC", 1);
  if (!_settings_pref.isKey("GEN_DECI_ACC")) _settings_pref.putInt("GEN_DECI_ACC", 2);
  if (!_settings_pref.isKey("CYCLIC_AV_BUFF")) _settings_pref.putInt("CYCLIC_AV_BUFF", 5);
  if (!_settings_pref.isKey("MAF_MIN_VOLTS")) _settings_pref.putInt("MAF_MIN_VOLTS", 1);

  if (!_settings_pref.isKey("API_DELIM")) _settings_pref.putString("API_DELIM", ":");
  if (!_settings_pref.isKey("SERIAL_BAUDRATE")) _settings_pref.putInt("SERIAL_BAUDRATE", 115200);
  if (!_settings_pref.isKey("SHOW_ALARMS")) _settings_pref.putInt("SHOW_ALARMS", true);
  if (!_settings_pref.isKey("ADJ_FLOW_DEP")) _settings_pref.putInt("ADJ_FLOW_DEP", 28);
  if (!_settings_pref.isKey("STD_REF")) _settings_pref.putInt("STD_REF", 1);
  if (!_settings_pref.isKey("STD_ADJ_FLOW")) _settings_pref.putInt("STD_ADJ_FLOW", 0);
  if (!_settings_pref.isKey("DATAGRAPH_MAX")) _settings_pref.putInt("DATAGRAPH_MAX", 0);
  if (!_settings_pref.isKey("MAF_MIN_VOLTS")) _settings_pref.putInt("MAF_MIN_VOLTS", 1);
  if (!_settings_pref.isKey("TEMP_UNIT")) _settings_pref.putInt("TEMP_UNIT", CELCIUS);

  if (!_settings_pref.isKey("LIFT_INTERVAL")) _settings_pref.putDouble("LIFT_INTERVAL", 1.5F);
  if (!_settings_pref.isKey("BENCH_TYPE")) _settings_pref.putInt("BENCH_TYPE", MAF);
  if (!_settings_pref.isKey("CAL_FLOW_RATE")) _settings_pref.putDouble("CAL_FLOW_RATE", 14.4F);
  if (!_settings_pref.isKey("CAL_REF_PRESS")) _settings_pref.putDouble("CAL_REF_PRESS", 10.0F);
  if (!_settings_pref.isKey("ORIFICE1_FLOW")) _settings_pref.putDouble("ORIFICE1_FLOW", 0.0F);
  if (!_settings_pref.isKey("ORIFICE1_PRESS")) _settings_pref.putDouble("ORIFICE1_PRESS", 0.0F);
  if (!_settings_pref.isKey("ORIFICE2_FLOW")) _settings_pref.putDouble("ORIFICE2_FLOW", 0.0F);
  if (!_settings_pref.isKey("ORIFICE2_PRESS")) _settings_pref.putDouble("ORIFICE2_PRESS", 0.0F);
  if (!_settings_pref.isKey("ORIFICE3_FLOW")) _settings_pref.putDouble("ORIFICE3_FLOW", 0.0F);
  if (!_settings_pref.isKey("ORIFICE3_PRESS")) _settings_pref.putDouble("ORIFICE3_PRESS", 0.0F);
  if (!_settings_pref.isKey("ORIFICE4_FLOW")) _settings_pref.putDouble("ORIFICE4_FLOW", 0.0F);
  if (!_settings_pref.isKey("ORIFICE4_PRESS")) _settings_pref.putDouble("ORIFICE4_PRESS", 0.0F);
  if (!_settings_pref.isKey("ORIFICE5_FLOW")) _settings_pref.putDouble("ORIFICE5_FLOW", 0.0F);
  if (!_settings_pref.isKey("ORIFICE5_PRESS")) _settings_pref.putDouble("ORIFICE5_PRESS", 0.0F);
  if (!_settings_pref.isKey("ORIFICE6_FLOW")) _settings_pref.putDouble("ORIFICE6_FLOW", 0.0F);
  if (!_settings_pref.isKey("ORIFICE6_PRESS")) _settings_pref.putDouble("ORIFICE6_PRESS", 0.0F);

  _settings_pref.end();

}











/***********************************************************
* @brief loadSettings
* @details read settings settings.json file and loads into global struct
***/ 
void DataHandler::loadSettings () {

  extern struct BenchSettings settings;

  DataHandler _data;
  Messages _message;
  Preferences _settings_pref;

  _message.serialPrintf("Loading Configuration \n");    
  
  _settings_pref.begin("settings", true);

  settings.wifi_ssid = _settings_pref.getString("WIFI_SSID", "WIFI-SSID");
  settings.wifi_pswd = _settings_pref.getString("WIFI_PSWD", "PASSWORD");
  settings.wifi_ap_ssid = _settings_pref.getString("WIFI_AP_SSID", "DIYFB" );
  settings.wifi_ap_pswd =_settings_pref.getString("WIFI_AP_PSWD", "123456789" );
  settings.hostname = _settings_pref.getString("HOSTNAME", "diyfb" );
  settings.wifi_timeout = _settings_pref.getInt("WIFI_TIMEOUT", 4000 );
  settings.maf_housing_diameter = _settings_pref.getInt("MAF_HOUSING_DIA", 0 );
  settings.refresh_rate = _settings_pref.getInt("REFRESH_RATE", 500 );
  settings.min_bench_pressure  = _settings_pref.getInt("MIN_BENCH_PRESS", 1 );
  settings.min_flow_rate = _settings_pref.getInt("MIN_FLOW_RATE", 1 );
  settings.data_filter_type = _settings_pref.getInt("DATA_FILTER_TYP", NONE );
  settings.rounding_type = _settings_pref.getInt("ROUNDING_TYPE", NONE );
  settings.flow_decimal_length = _settings_pref.getInt("FLOW_DECI_ACC", 1 );
  settings.gen_decimal_length = _settings_pref.getInt("GEN_DECI_ACC", 2 );
  settings.cyc_av_buffer  = _settings_pref.getInt("CYCLIC_AV_BUFF", 5 );
  settings.maf_min_volts  = _settings_pref.getInt("MAF_MIN_VOLTS", 0.1F );
  settings.api_delim = _settings_pref.getString("API_DELIM", ":" );
  settings.serial_baud_rate = _settings_pref.getInt("SERIAL_BAUDRATE",  115200 );
  settings.show_alarms = _settings_pref.getInt("SHOW_ALARMS",  true  );
  settings.adj_flow_depression = _settings_pref.getInt("ADJ_FLOW_DEP",  28  );
  settings.standardReference = _settings_pref.getInt("STD_REF", 1  );
  settings.std_adj_flow = _settings_pref.getInt("STD_ADJ_FLOW",  0 );
  settings.dataGraphMax = _settings_pref.getInt("DATAGRAPH_MAX", 0 );
  settings.temp_unit = _settings_pref.getInt("TEMP_UNIT", CELCIUS );
  settings.valveLiftInterval = _settings_pref.getDouble("LIFT_INTERVAL", 1.5F  );
  settings.bench_type = _settings_pref.getInt("BENCH_TYPE", MAF );
  settings.cal_flow_rate = _settings_pref.getDouble("CAL_FLOW_RATE", 14.4F );
  settings.cal_ref_press = _settings_pref.getDouble("CAL_REF_PRESS", 10.0F );
  settings.orificeOneFlow = _settings_pref.getDouble("ORIFICE1_FLOW", 0.0F );
  settings.orificeOneDepression = _settings_pref.getDouble("ORIFICE1_PRESS", 0.0F );
  settings.orificeTwoFlow = _settings_pref.getDouble("ORIFICE2_FLOW", 0.0F );
  settings.orificeTwoDepression = _settings_pref.getDouble("ORIFICE2_PRESS", 0.0F );
  settings.orificeThreeFlow = _settings_pref.getDouble("ORIFICE3_FLOW", 0.0F );
  settings.orificeThreeDepression = _settings_pref.getDouble("ORIFICE3_PRESS", 0.0F );
  settings.orificeFourFlow = _settings_pref.getDouble("ORIFICE4_FLOW", 0.0F );
  settings.orificeFourDepression = _settings_pref.getDouble("ORIFICE4_PRESS", 0.0F );
  settings.orificeFiveFlow = _settings_pref.getDouble("ORIFICE5_FLOW", 0.0F );
  settings.orificeFiveDepression = _settings_pref.getDouble("ORIFICE5_PRESS", 0.0F );
  settings.orificeSixFlow = _settings_pref.getDouble("ORIFICE6_FLOW", 0.0F );
  settings.orificeSixDepression = _settings_pref.getDouble("ORIFICE6_PRESS",  0.0F);

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
 * @brief byteDecode
 * @param bytes size to be decoded
 * @details Byte Decode (returns string i.e '52 GB')
 ***/
String DataHandler::byteDecode(size_t bytes)
{
  if (bytes < 1024)
    return String(bytes) + " B";
  else if (bytes < (1024 * 1024))
    return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024))
    return String(bytes / 1024.0 / 1024.0) + " MB";
  else
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
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

  _message.statusPrintf("Filesystem contents: \n");
  FILESYSTEM.begin();
  File root = FILESYSTEM.open("/");
  File file = root.openNextFile();
  while (file)  {
    fileName = file.name();
    fileSize = file.size();
    dataJson[fileName] = String(fileSize);
    _message.statusPrintf("%s : %s \n", fileName, byteDecode(fileSize));
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
      dataJson["STD_REF"] = "ISO-1585";
    break;

    case ISA :
      dataJson["STD_REF"] = "ISA";
    break;

    case ISO_13443:
      dataJson["STD_REF"] = "ISO-13443";
    break;

    case ISO_5011:
      dataJson["STD_REF"] = "ISO-5011";
    break;

    case ISO_2533:
      dataJson["STD_REF"] = "ISO-2533";
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
      dataJson["BENCH_TYPE"] = "MAF";
    break;

    case ORIFICE:
      dataJson["BENCH_TYPE"] = "ORIFICE";
    break;

    case VENTURI:
      dataJson["BENCH_TYPE"] = "VENTURI";
    break;

    case PITOT:
      dataJson["BENCH_TYPE"] = "PITOT";
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

