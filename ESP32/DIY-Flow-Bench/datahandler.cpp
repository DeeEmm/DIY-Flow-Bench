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
#include "API.h"


void DataHandler::begin() {

    extern struct BenchSettings settings;
    extern struct Language language;
    extern struct DeviceStatus status;
    extern struct Configuration config;

    Preferences _config_pref;
    Hardware _hardware;
    Pins pins;
    Messages _message;
    Comms _comms;
    DataHandler _data;

    StaticJsonDocument<1024> pinData;
    // StaticJsonDocument<1024> mafData;

    // Need to set up the data environment...

    // Load configuration
    this->initialiseConfig();
    this->loadConfig();

    _message.serialPrintf("_data: config.ADC_I2C_ADDR: %u \n", config.ADC_I2C_ADDR);

    // Start serial comms
    this->beginSerial(); 

    _message.serialPrintf("\r\nDIY Flow Bench\n");                                         
    // Note RELEASE and BUILD_NUMBER are defined at compile time within extra_scripts directive in user_actions_pre.py 
    _message.serialPrintf("DIYFB Version: %s \nBuild: %s \n", RELEASE, BUILD_NUMBER);                                         
    _message.serialPrintf("For help please visit the WIKI:\n");                                         
    _message.serialPrintf("https://github.com/DeeEmm/DIY-Flow-Bench/wiki\n");                                         

    // initialise SPIFFS Filesystem
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

    // Check if settings / calibration / liftdata json files exist. If not create them.
    if (!SPIFFS.exists("/settings.json")) createSettingsFile();
    if (!SPIFFS.exists("/liftdata.json")) createLiftDataFile();
    if (!SPIFFS.exists("/cal.json")) createCalibrationFile();

    // load config / calibration / liftdata files
    this->initialiseSettings();
    this->loadSettings();
    
    this->loadLiftData();
    
    this->loadCalibrationData();



    // // Check if config files exists. If not send to boot loop until it is uploaded
    // if (SPIFFS.exists("/configuration.json")) {
    //   this->loadConfiguration();
    // } else {
    //   status.doBootLoop = true;
    // }

    // // Check for CONFIG file
    // if (checkUserFile(CONFIGFILE)) {
    //   if (!SPIFFS.exists(status.pinsFilename)) {
    //       status.doBootLoop = true;
    //       _message.serialPrintf("!! CONFIG file not found !!\n");  
    //   }
    // } else {
    //       status.doBootLoop = true;
    //       _message.serialPrintf("!! CONFIG file not found !!\n");  
    // }

    // Check for PINS file
    if (checkUserFile(PINSFILE)) {
      if (!SPIFFS.exists(status.pinsFilename)) {
          status.doBootLoop = true;
          _message.serialPrintf("!! PINS file not found !!\n");  
      }
    } else {
          status.doBootLoop = true;
          _message.serialPrintf("!! PINS file not found !!\n");  
    }
    
    // Check for MAF file
    if (checkUserFile(MAFFILE)) {
      if (!SPIFFS.exists(status.mafFilename)) {
          status.doBootLoop = true;
          _message.serialPrintf("!! MAF file not found !!\n");  
      }
    } else {
          status.doBootLoop = true;
          _message.serialPrintf("!! MAF file not found !!\n");  
    }

    // Initialise WiFi
    _comms.initaliseWifi();

    // BootLoop method traps program pointer within loop until files are uploaded
    if (status.doBootLoop == true) bootLoop();

    // Load MAF / CONFIG / PINS files
    // if (status.configLoaded == true) _data.loadConfiguration();
    if (status.pinsLoaded == true) _data.loadPinsData();
    if (status.mafLoaded == true) _data.loadMAFData();
    
    _hardware.initaliseIO();

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
* @param secindString
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
* @name checkUserFile.cpp
* @brief Loop through all files in SPIFFS to match filename prefix
* @param filetype INT (MAFFILE or PINSFILE [default])
* @returns true if file found
* @see status.pinsFilename 
* @see variable status.mafFilename 
***/
bool DataHandler::checkUserFile(int filetype) {
      
// TODO Change to accept prefix as arguement

    DataHandler _data;
    Messages _message;
    extern struct DeviceStatus status;

    FILESYSTEM.begin();
    File root = FILESYSTEM.open("/");
    File file = root.openNextFile();

    std::string matchCONFIG = "config";
    std::string matchPINS = "PINS";
    std::string matchMAF = "MAF";
    std::string matchINDEX = "index";
    std::string spiffsFile;
    std::string configFile;
    std::string pinsFile;
    std::string mafFile;
    std::string indexFile;

    // Check SPIFFS for pins and MAF files
    while (file)  {
      spiffsFile = file.name();

      // Check config file
      if (checkSubstring(spiffsFile.c_str(), matchCONFIG.c_str()) && (filetype == CONFIGFILE)) {
        configFile = "/" + spiffsFile;
        _message.serialPrintf("CONFIG file Found: %s\n", configFile.c_str() );  
        status.pinsFilename = configFile.c_str();        
        // loadConfiguration();
        status.configLoaded = true;
        return true;
      }   

      // Check PINS file      
      if (checkSubstring(spiffsFile.c_str(), matchPINS.c_str()) && (filetype == PINSFILE)) {
        pinsFile = "/" + spiffsFile;
        _message.serialPrintf("PINS file Found: %s\n", pinsFile.c_str() );  
        status.pinsFilename = pinsFile.c_str();        
        // loadPinsData();
        status.pinsLoaded = true;
        return true;
      }   
      
      // Check MAF file
      if (checkSubstring(spiffsFile.c_str(), matchMAF.c_str()) && (filetype == MAFFILE)) {
        mafFile = "/" + spiffsFile;
        _message.serialPrintf("MAF file Found: %s\n", mafFile.c_str() );  
        status.mafFilename = mafFile.c_str();
        // loadMAFData();
        status.mafLoaded = true;
        return true;
      }
      
      // // Check index file
      // if (checkSubstring(spiffsFile.c_str(), matchINDEX.c_str()) && (filetype == INDEXFILE)) {
      //   indexFile = "/" + spiffsFile;
      //   _message.serialPrintf("Index file Found: %s\n", indexFile.c_str() );  
      //   status.indexFilename = indexFile.c_str();
      //   status.GUIexists = true;
      //   return true;
      // }

      file = root.openNextFile();
    }



    return false;

}



/***********************************************************
* @brief createConfig
* @details Create basic minimum configuration json file
* @note Called from DataHandler::begin() if settings.json not found
***/
void DataHandler::createSettingsFile() {

  extern struct BenchSettings settings;
  Messages _message;
  String jsonString;  
  StaticJsonDocument<CONFIG_JSON_SIZE> configData;

  _message.serialPrintf("Creating settings.json file... \n"); 
 
  configData["PAGE_TITLE"] = settings.pageTitle;
  configData["WIFI_SSID"] = settings.wifi_ssid;
  configData["WIFI_PSWD"] = settings.wifi_pswd;
  configData["WIFI_AP_SSID"] = settings.wifi_ap_ssid;
  configData["WIFI_AP_PSWD"] = settings.wifi_ap_pswd;
  configData["HOSTNAME"] = settings.hostname;
  configData["WIFI_TIMEOUT"] = settings.wifi_timeout;
  configData["MAF_HOUSING_DIA"] = settings.maf_housing_diameter;
  configData["REFRESH_RATE"] = settings.refresh_rate;
  configData["MIN_BENCH_PRESS"] = settings.min_bench_pressure;
  configData["MIN_FLOW_RATE"] = settings.min_flow_rate;
  configData["DATA_FILTER_TYP"] = settings.data_filter_type;
  configData["ROUNDING_TYPE"] = settings.rounding_type;
  configData["FLOW_DECI_ACC"] = settings.flow_decimal_length;
  configData["GEN_DECI_ACC"] = settings.gen_decimal_length;
  configData["CYCLIC_AV_BUFF"] = settings.cyc_av_buffer;
  configData["MAF_MIN_VOLTS"] = settings.maf_min_volts;
  configData["API_DELIM"] = settings.api_delim;
  configData["SERIAL_BAUD_RATE"] = settings.serial_baud_rate;
  configData["ADJ_FLOW_DEP"] = settings.adj_flow_depression;
  configData["STD_REF"] = settings.standardReference;
  configData["STD_ADJ_FLOW"] = settings.std_adj_flow;
  configData["DATAGRAPH_MAX"] = settings.dataGraphMax;
  configData["TEMP_UNIT"] = settings.temp_unit;
  configData["LIFT_INTERVAL"] = settings.valveLiftInterval;
  configData["SHOW_ALARMS"] = settings.show_alarms;
  configData["BENCH_TYPE"] = settings.bench_type;
  configData["CAL_FLOW_RATE"] = settings.cal_flow_rate;
  configData["CAL_REF_PRESS"] = settings.cal_ref_press;
  configData["ORIFICE1_FLOW"] = settings.orificeOneFlow;
  configData["ORIFICE1_PRESS"] = settings.orificeOneDepression;
  configData["ORIFICE2_FLOW"] = settings.orificeTwoFlow;
  configData["ORIFICE2_PRESS"] = settings.orificeThreeDepression;
  configData["ORIFICE3_FLOW"] = settings.orificeThreeFlow;
  configData["ORIFICE4_FLOW"] = settings.orificeFourFlow;
  configData["ORIFICE4_PRESS"] = settings.orificeFourDepression;
  configData["ORIFICE5_FLOW"] = settings.orificeFiveFlow;
  configData["ORIFICE5_PRESS"] = settings.orificeFiveDepression;
  configData["ORIFICE6_FLOW"] = settings.orificeSixFlow;
  configData["ORIFICE7_TEST_PRESSURE"] = settings.orificeSixDepression;

  serializeJsonPretty(configData, jsonString);
  writeJSONFile(jsonString, "/settings.json", CONFIG_JSON_SIZE);

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
* @brief createLiftDataFile
* @details Create blank lift data json file
* @note Called from DataHandler::begin() if liftdata.json not found
***/
void DataHandler::createLiftDataFile () {

  Messages _message;
  String jsonString;  
  StaticJsonDocument<LIFT_DATA_JSON_SIZE> liftData;

  _message.serialPrintf("Creating liftdata.json file... \n"); 

  liftData["LIFTDATA1"] = 0.0;
  liftData["LIFTDATA2"] = 0.0;
  liftData["LIFTDATA3"] = 0.0;
  liftData["LIFTDATA4"] = 0.0;
  liftData["LIFTDATA5"] = 0.0;
  liftData["LIFTDATA6"] = 0.0;
  liftData["LIFTDATA7"] = 0.0;
  liftData["LIFTDATA8"] = 0.0;
  liftData["LIFTDATA9"] = 0.0;
  liftData["LIFTDATA10"] = 0.0;
  liftData["LIFTDATA11"] = 0.0;
  liftData["LIFTDATA12"] = 0.0;

  serializeJsonPretty(liftData, jsonString);
  writeJSONFile(jsonString, "/liftdata.json", LIFT_DATA_JSON_SIZE);

}




/***********************************************************
* @brief createCalibration File
* @details Create configuration json file
* @note Called from DataHandler::begin() if cal.json not found
***/
void DataHandler::createCalibrationFile () {

  extern struct CalibrationData calVal;
  Messages _message;
  String jsonString;
  StaticJsonDocument<CAL_DATA_JSON_SIZE> calData;
  
  _message.debugPrintf("Creating cal.json file... \n"); 
  
  calData["FLOW_OFFSET"] = calVal.flow_offset;
  calData["USER_OFFSET"] = calVal.user_offset;
  calData["LEAK_CAL_BASELINE"] = calVal.leak_cal_baseline;
  calData["LEAK_CAL_BASELINE_REV"] = calVal.leak_cal_baseline_rev;
  calData["LEAK_CAL_OFFSET"] = calVal.leak_cal_offset;
  calData["LEAK_CAL_OFFSET_REV"] = calVal.leak_cal_offset_rev;

  serializeJsonPretty(calData, jsonString);

  File outputFile = SPIFFS.open("/cal.json", FILE_WRITE);
  serializeJsonPretty(calData, outputFile);
  outputFile.close();
  
}





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
  if (!_config_pref.isKey("MAF_SENS_TYPE")) _config_pref.putString("MAF_SENS_TYPE", "Not Set");
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
  
  _config_pref.begin("config", false);

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
  config.MAF_SENS_TYPE = _config_pref.getString("MAF_SENS_TYPE", "not set");
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
  Preferences _config_pref;

  _message.serialPrintf("Loading Bench Settings \n");    
  
  _config_pref.begin("settings", false);

  if (!_config_pref.isKey("WIFI_SSID")) _config_pref.putString("WIFI_SSID", "WIFI-SSID");
  if (!_config_pref.isKey("WIFI_PSWD")) _config_pref.putString("WIFI_PSWD", static_cast<String>("PASSWORD"));
  if (!_config_pref.isKey("WIFI_AP_SSID")) _config_pref.putString("WIFI_AP_SSID", static_cast<String>("DIYFB"));
  if (!_config_pref.isKey("WIFI_AP_PSWD")) _config_pref.putString("WIFI_AP_PSWD", static_cast<String>("123456789"));
  if (!_config_pref.isKey("HOSTNAME")) _config_pref.putString("HOSTNAME", static_cast<String>("diyfb"));

  if (!_config_pref.isKey("WIFI_TIMEOUT")) _config_pref.putInt("WIFI_TIMEOUT", 4000);
  if (!_config_pref.isKey("MAF_HOUSING_DIA")) _config_pref.putInt("MAF_HOUSING_DIA", 0);
  if (!_config_pref.isKey("REFRESH_RATE")) _config_pref.putInt("REFRESH_RATE", 500);
  if (!_config_pref.isKey("MIN_BENCH_PRESS")) _config_pref.putInt("MIN_BENCH_PRESS", 1);
  if (!_config_pref.isKey("MIN_FLOW_RATE")) _config_pref.putInt("MIN_FLOW_RATE", 1);

  if (!_config_pref.isKey("DATA_FILTER_TYP")) _config_pref.putString("DATA_FILTER_TYP", static_cast<String>("NONE"));
  if (!_config_pref.isKey("ROUNDING_TYPE")) _config_pref.putString("ROUNDING_TYPE", static_cast<String>("NONE"));

  if (!_config_pref.isKey("FLOW_DECI_ACC")) _config_pref.putInt("FLOW_DECI_ACC", 1);
  if (!_config_pref.isKey("GEN_DECI_ACC")) _config_pref.putInt("GEN_DECI_ACC", 2);
  if (!_config_pref.isKey("CYCLIC_AV_BUFF")) _config_pref.putInt("CYCLIC_AV_BUFF", 5);
  if (!_config_pref.isKey("MAF_MIN_VOLTS")) _config_pref.putInt("MAF_MIN_VOLTS", 1);

  if (!_config_pref.isKey("API_DELIM")) _config_pref.putString("API_DELIM", ":");
  if (!_config_pref.isKey("SERIAL_BAUD_RATE")) _config_pref.putInt("SERIAL_BAUD_RATE", 115200);
  if (!_config_pref.isKey("SHOW_ALARMS")) _config_pref.putInt("SHOW_ALARMS", true);
  if (!_config_pref.isKey("ADJ_FLOW_DEP")) _config_pref.putInt("ADJ_FLOW_DEP", 28);
  if (!_config_pref.isKey("STD_REF")) _config_pref.putInt("STD_REF", 1);
  if (!_config_pref.isKey("STD_ADJ_FLOW")) _config_pref.putInt("STD_ADJ_FLOW", 0);
  if (!_config_pref.isKey("DATAGRAPH_MAX")) _config_pref.putInt("DATAGRAPH_MAX", 0);
  if (!_config_pref.isKey("MAF_MIN_VOLTS")) _config_pref.putInt("MAF_MIN_VOLTS", 1);
  if (!_config_pref.isKey("TEMP_UNIT")) _config_pref.putString("TEMP_UNIT", "Celcius");

  if (!_config_pref.isKey("LIFT_INTERVAL")) _config_pref.putDouble("LIFT_INTERVAL", 1.5F);
  if (!_config_pref.isKey("BENCH_TYPE")) _config_pref.putInt("BENCH_TYPE", MAF);
  if (!_config_pref.isKey("CAL_FLOW_RATE")) _config_pref.putDouble("CAL_FLOW_RATE", 14.4F);
  if (!_config_pref.isKey("CAL_REF_PRESS")) _config_pref.putDouble("CAL_REF_PRESS", 10.0F);
  if (!_config_pref.isKey("ORIFICE1_FLOW")) _config_pref.putDouble("ORIFICE1_FLOW", 0.0F);
  if (!_config_pref.isKey("ORIFICE1_PRESS")) _config_pref.putDouble("ORIFICE1_PRESS", 0.0F);
  if (!_config_pref.isKey("ORIFICE2_FLOW")) _config_pref.putDouble("ORIFICE2_FLOW", 0.0F);
  if (!_config_pref.isKey("ORIFICE2_PRESS")) _config_pref.putDouble("ORIFICE2_PRESS", 0.0F);
  if (!_config_pref.isKey("ORIFICE3_FLOW")) _config_pref.putDouble("ORIFICE3_FLOW", 0.0F);
  if (!_config_pref.isKey("ORIFICE3_PRESS")) _config_pref.putDouble("ORIFICE3_PRESS", 0.0F);
  if (!_config_pref.isKey("ORIFICE4_FLOW")) _config_pref.putDouble("ORIFICE4_FLOW", 0.0F);
  if (!_config_pref.isKey("ORIFICE4_PRESS")) _config_pref.putDouble("ORIFICE4_PRESS", 0.0F);
  if (!_config_pref.isKey("ORIFICE5_FLOW")) _config_pref.putDouble("ORIFICE5_FLOW", 0.0F);
  if (!_config_pref.isKey("ORIFICE5_PRESS")) _config_pref.putDouble("ORIFICE5_PRESS", 0.0F);
  if (!_config_pref.isKey("ORIFICE6_FLOW")) _config_pref.putDouble("ORIFICE6_FLOW", 0.0F);
  if (!_config_pref.isKey("ORIFICE6_PRESS")) _config_pref.putDouble("ORIFICE6_PRESS", 0.0F);

  _config_pref.end();

}











/***********************************************************
* @brief loadSettings
* @details read settings settings.json file and loads into global struct
***/ 
void DataHandler::loadSettings () {

  extern struct BenchSettings settings;

  DataHandler _data;
  Messages _message;
  Preferences _config_pref;

  _message.serialPrintf("Loading Configuration \n");    
  
  _config_pref.begin("settings", false);


  // strcpy(_config_pref.getString("WIFI_SSID", "WIFI-SSID" ).toCharArray(), settings.wifi_ssid);
  settings.wifi_ssid = _config_pref.getString("WIFI_SSID", "WIFI-SSID");
  settings.wifi_pswd = _config_pref.getString("WIFI_PSWD", "PASSWORD");
  settings.wifi_ap_ssid = _config_pref.getString("WIFI_AP_SSID", "DIYFB" );
  settings.wifi_ap_pswd =_config_pref.getString("WIFI_AP_PSWD", "123456789" );
  settings.hostname = _config_pref.getString("HOSTNAME", "diyfb" );
  settings.wifi_timeout = _config_pref.getInt("WIFI_TIMEOUT", 4000 );
  settings.maf_housing_diameter = _config_pref.getInt("MAF_HOUSING_DIA", 0 );
  settings.refresh_rate = _config_pref.getInt("REFRESH_RATE", 500 );
  settings.min_bench_pressure  = _config_pref.getInt("MIN_BENCH_PRESS", 1 );
  settings.min_flow_rate = _config_pref.getInt("MIN_FLOW_RATE", 1 );
  settings.data_filter_type = _config_pref.getString("DATA_FILTER_TYP", "NONE" );
  settings.rounding_type = _config_pref.getString("ROUNDING_TYPE", "NONE" );
  settings.flow_decimal_length = _config_pref.getInt("FLOW_DECI_ACC", 1 );
  settings.gen_decimal_length = _config_pref.getInt("GEN_DECI_ACC", 2 );
  settings.cyc_av_buffer  = _config_pref.getInt("CYCLIC_AV_BUFF", 5 );
  settings.maf_min_volts  = _config_pref.getInt("MAF_MIN_VOLTS", 0.1F );
  settings.api_delim = _config_pref.getString("API_DELIM", ":" );
  settings.serial_baud_rate = _config_pref.getInt("SERIAL_BAUD_RATE",  115200 );
  settings.show_alarms = _config_pref.getInt("SHOW_ALARMS",  true  );
  settings.adj_flow_depression = _config_pref.getInt("ADJ_FLOW_DEP",  28  );
  settings.standardReference = _config_pref.getInt("STD_REF", 1  );
  settings.std_adj_flow = _config_pref.getInt("STD_ADJ_FLOW",  0 );
  settings.dataGraphMax = _config_pref.getInt("DATAGRAPH_MAX", 0 );
  settings.temp_unit = _config_pref.getString("TEMP_UNIT", "Celcius" );
  settings.valveLiftInterval = _config_pref.getDouble("LIFT_INTERVAL", 1.5F  );
  settings.bench_type = _config_pref.getInt("BENCH_TYPE", MAF );
  settings.cal_flow_rate = _config_pref.getDouble("CAL_FLOW_RATE", 14.4F );
  settings.cal_ref_press = _config_pref.getDouble("CAL_REF_PRESS", 10.0F );
  settings.orificeOneFlow = _config_pref.getDouble("ORIFICE1_FLOW", 0.0F );
  settings.orificeOneDepression = _config_pref.getDouble("ORIFICE1_PRESS", 0.0F );
  settings.orificeTwoFlow = _config_pref.getDouble("ORIFICE2_FLOW", 0.0F );
  settings.orificeTwoDepression = _config_pref.getDouble("ORIFICE2_PRESS", 0.0F );
  settings.orificeThreeFlow = _config_pref.getDouble("ORIFICE3_FLOW", 0.0F );
  settings.orificeThreeDepression = _config_pref.getDouble("ORIFICE3_PRESS", 0.0F );
  settings.orificeFourFlow = _config_pref.getDouble("ORIFICE4_FLOW", 0.0F );
  settings.orificeFourDepression = _config_pref.getDouble("ORIFICE4_PRESS", 0.0F );
  settings.orificeFiveFlow = _config_pref.getDouble("ORIFICE5_FLOW", 0.0F );
  settings.orificeFiveDepression = _config_pref.getDouble("ORIFICE5_PRESS", 0.0F );
  settings.orificeSixFlow = _config_pref.getDouble("ORIFICE6_FLOW", 0.0F );
  settings.orificeSixDepression = _config_pref.getDouble("ORIFICE6_PRESS",  0.0F);

  _config_pref.end();

}






// /***********************************************************
// * @brief loadSettings
// * @details read settings settings.json file and loads into global struct
// ***/ 
// StaticJsonDocument<SETTINGS_JSON_SIZE> DataHandler::loadSettings () {

//   extern struct BenchSettings settings;

//   StaticJsonDocument<SETTINGS_JSON_SIZE> configData;
//   DataHandler _data;
//   Messages _message;

//   _message.serialPrintf("Loading Bench Settings \n");     

//   if (SPIFFS.exists("/settings.json"))  {

//     configData = _data.loadJSONFile("/settings.json");

//     strcpy(settings.wifi_ssid, configData["WIFI_SSID"]);
//     strcpy(settings.wifi_pswd, configData["WIFI_PSWD"]);
//     strcpy(settings.wifi_ap_ssid, configData["WIFI_AP_SSID"]);
//     strcpy(settings.wifi_ap_pswd,configData["WIFI_AP_PSWD"]);
//     strcpy(settings.hostname, configData["HOSTNAME"]);
//     settings.wifi_timeout = configData["WIFI_TIMEOUT"].as<int>();
//     settings.maf_housing_diameter = configData["MAF_HOUSING_DIA"].as<int>();
//     settings.refresh_rate = configData["REFRESH_RATE"].as<int>();
//     settings.min_bench_pressure  = configData["MIN_BENCH_PRESS"].as<int>();
//     settings.min_flow_rate = configData["MIN_FLOW_RATE"].as<int>();
//     strcpy(settings.data_filter_type, configData["DATA_FILTER_TYP"]);
//     strcpy(settings.rounding_type, configData["ROUNDING_TYPE"]);
//     settings.flow_decimal_length, configData["FLOW_DECI_ACC"];
//     settings.gen_decimal_length, configData["GEN_DECI_ACC"];
//     settings.cyc_av_buffer  = configData["CYCLIC_AV_BUFF"].as<int>();
//     settings.maf_min_volts  = configData["MAF_MIN_VOLTS"].as<int>();
//     strcpy(settings.api_delim, configData["API_DELIM"]);
//     settings.serial_baud_rate = configData["SERIAL_BAUD_RATE"].as<long>();
//     settings.show_alarms = configData["SHOW_ALARMS"].as<bool>();
//     configData["ADJ_FLOW_DEP"] = settings.adj_flow_depression;
//     configData["STD_REF"] = settings.standardReference;
//     configData["STD_ADJ_FLOW"] = settings.std_adj_flow;
//     configData["DATAGRAPH_MAX"] = settings.dataGraphMax;
//     configData["TEMP_UNIT"] = settings.temp_unit;
//     configData["LIFT_INTERVAL"] = settings.valveLiftInterval;
//     strcpy(settings.bench_type, configData["BENCH_TYPE"]);
//     settings.cal_flow_rate = configData["CAL_FLOW_RATE"].as<double>();
//     settings.cal_ref_press = configData["CAL_REF_PRESS"].as<double>();
//     settings.orificeOneFlow = configData["ORIFICE1_FLOW"].as<double>();
//     settings.orificeOneDepression = configData["ORIFICE1_PRESS"].as<double>();
//     settings.orificeTwoFlow = configData["ORIFICE2_FLOW"].as<double>();
//     settings.orificeTwoDepression = configData["ORIFICE2_PRESS"].as<double>();
//     settings.orificeThreeFlow = configData["ORIFICE3_FLOW"].as<double>();
//     settings.orificeThreeDepression = configData["ORIFICE3_PRESS"].as<double>();
//     settings.orificeFourFlow = configData["ORIFICE4_FLOW"].as<double>();
//     settings.orificeFourDepression = configData["ORIFICE4_PRESS"].as<double>();
//     settings.orificeFiveFlow = configData["ORIFICE5_FLOW"].as<double>();
//     settings.orificeFiveDepression = configData["ORIFICE5_PRESS"].as<double>();
//     settings.orificeSixFlow = configData["ORIFICE6_FLOW"].as<double>();
//     settings.orificeSixDepression = configData["ORIFICE6_PRESS"].as<double>();

//   } else {
//     _message.serialPrintf("Bench Settings file not found \n");
//   }
  
//   return configData;  

// }







/***********************************************************
* @brief load MAF File
* @details read MAF data from MAF.json file
***/ 
void DataHandler::loadMAFData () {

  DataHandler _data;
  Messages _message;
  extern struct DeviceStatus status;

  StaticJsonDocument<MAF_JSON_SIZE> mafData;
  JsonObject mafJsonObject = mafData.to<JsonObject>();

  _message.serialPrintf("Loading MAF Data \n");

  // read JSON data direct from stream
  File jsonFile = SPIFFS.open(status.mafFilename, FILE_READ);
  deserializeJson(mafData, jsonFile);

  if (mafData.overflowed() == true) {
    _message.serialPrintf("MAF Data file - JsonDocument::overflowed()");
  } else {
    // serializeJsonPretty(mafData, Serial);
  }

  // populate global structs
  strcpy(status.mafSensorType, mafData["sensor_type"]); 
  strcpy(status.mafLink, mafData["forum_link"]); 
  strcpy(status.mafOutputType, mafData["output_type"]); 
  strcpy(status.mafUnits, mafData["maf_units"]);
  status.mafScaling = mafData["maf_scaling"]; 
  status.mafDiameter = mafData["maf_diameter"]; 

  // Load MAF lookup table into JSON object 
  mafJsonObject = mafData["maf_lookup_table"]; 

  // Prints MAF data to serial
  #ifdef VERBOSE_MAF
    for (JsonPair kv : mafJsonObject) {
      _message.verbosePrintf("JSON key: %s", kv.key().c_str());
      _message.verbosePrintf(" value: %s\n",kv.value().as<std::string>().c_str()); 
    }
  #endif

  // Print size of MAF data to serial
  _message.serialPrintf("MAF Data Memory Usage: %u \n", mafData.memoryUsage()); 
  _message.serialPrintf("MAF Data JSON Object Memory Usage: %u \n", mafJsonObject.memoryUsage()); 

  // get size of the MAF datatable (num rows)
  status.mafDataTableRows = mafJsonObject.size();
  _message.verbosePrintf("status.mafDataTableRows: %i\n", status.mafDataTableRows);


  u_int rowNum = 0;
  u_int key;
  u_int value;

  // Get JSON Object iterator
  JsonObject::iterator it = mafJsonObject.begin();

  // Walk through JSON object to populate vectors
  for (u_int rowNum = 0; rowNum < status.mafDataTableRows; rowNum++) { 

    key = stoi(it->key().c_str());
    value = stoi(it->value().as<std::string>());

    #ifdef VERBOSE_MAF
      _message.verbosePrintf("Vector rowNum: %i", rowNum);
      _message.verbosePrintf(" key: %i", key);
      _message.verbosePrintf(" value: %i\n", value);
    #endif

    status.mafLookupTable.push_back( { key , value } );

    it += 1;

    status.mafDataKeyMax = key;
    status.mafDataValMax = value;
    
  }

  _message.verbosePrintf("MAF Data Val Max: %lu\n", status.mafDataValMax);
  _message.verbosePrintf("MAF Data Key Max: %lu\n", status.mafDataKeyMax);

  status.mafLoaded = true;

}





/***********************************************************
* @brief loadLiftDataFile
* @details read lift data from liftdata.json file
***/ 
StaticJsonDocument<LIFT_DATA_JSON_SIZE> DataHandler::loadLiftData () {

  extern struct ValveLiftData valveData;
  StaticJsonDocument<LIFT_DATA_JSON_SIZE> liftData;
  DataHandler _data;
  Messages _message;

  _message.serialPrintf("Loading Lift Data \n");     

  if (SPIFFS.exists("/liftdata.json"))  {
    
    liftData = _data.loadJSONFile("/liftdata.json");
    
    valveData.LiftData1 = liftData["LIFTDATA1"].as<double>();
    valveData.LiftData2 = liftData["LIFTDATA2"].as<double>();
    valveData.LiftData3 = liftData["LIFTDATA3"].as<double>();
    valveData.LiftData4 = liftData["LIFTDATA4"].as<double>();
    valveData.LiftData5 = liftData["LIFTDATA5"].as<double>();
    valveData.LiftData6 = liftData["LIFTDATA6"].as<double>();
    valveData.LiftData7 = liftData["LIFTDATA7"].as<double>();
    valveData.LiftData8 = liftData["LIFTDATA8"].as<double>();
    valveData.LiftData9 = liftData["LIFTDATA9"].as<double>();
    valveData.LiftData10 = liftData["LIFTDATA10"].as<double>();
    valveData.LiftData11 = liftData["LIFTDATA11"].as<double>();
    valveData.LiftData12 = liftData["LIFTDATA12"].as<double>();

  } else {
    _message.serialPrintf("LiftData file not found \n");
  }
  
  return liftData;  

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
* @name loadCalibrationData
* @brief Read calibration data from calibration.json file
* @return calibrationData 
***/
StaticJsonDocument<1024> DataHandler::loadCalibrationData () {

  DataHandler _data;
  Messages _message;
  _message.serialPrintf("Loading Calibration Data \n");     

  StaticJsonDocument<1024> calibrationData;
  calibrationData = _data.loadJSONFile("/cal.json");
  parseCalibrationData(calibrationData);
  return calibrationData;
}






/***********************************************************
* @brief Parse Calibration Data
* @param calibrationData JSON document containing calibration data
***/
void DataHandler::parseCalibrationData(StaticJsonDocument<1024> calData) {

  extern struct CalibrationData calVal;

  calVal.flow_offset = calData["FLOW_OFFSET"];
  calVal.user_offset = calData["USER_OFFSET"];
  calVal.leak_cal_baseline = calData["LEAK_CAL_BASELINE"];
  calVal.leak_cal_baseline_rev = calData["LEAK_CAL_BASELINE_REV"];
  calVal.leak_cal_offset = calData["LEAK_CAL_OFFSET"];
  calVal.leak_cal_offset_rev = calData["LEAK_CAL_OFFSET_REV"];
  calVal.pdiff_cal_offset = calData["PDIFF_CAL_OFFSET"];
  calVal.pitot_cal_offset = calData["PITOT_CAL_OFFSET"];
}







/***********************************************************
* @name ParsePinsData
* @brief Updates pins struct from passed JSON data
* @param pinsData JSON document containing pins data
***/
void DataHandler::parsePinsData(StaticJsonDocument<1024> pinData) {

  extern struct CalibrationData calVal;
  extern struct Pins pins;
  extern struct DeviceStatus status;

  Messages _message;

  _message.serialPrintf("Parsing Pins Data \n");    

  status.boardType = pinData["BOARD_TYPE"].as<String>();

  // Store input pin values in struct
  pins.VCC_3V3_PIN = pinData["VCC_3V3_PIN"].as<int>();
  pins.VCC_5V_PIN = pinData["VCC_5V_PIN"].as<int>();
  pins.SPEED_SENS_PIN = pinData["SPEED_SENS_PIN"].as<int>();
  pins.ORIFICE_BCD_BIT1_PIN = pinData["ORIFICE_BCD_BIT1_PIN"].as<int>();
  pins.ORIFICE_BCD_BIT2_PIN = pinData["ORIFICE_BCD_BIT2_PIN"].as<int>();
  pins.ORIFICE_BCD_BIT3_PIN = pinData["ORIFICE_BCD_BIT3_PIN"].as<int>();
  pins.MAF_PIN = pinData["MAF_PIN"].as<int>();
  pins.REF_PRESSURE_PIN = pinData["PREF_PIN"].as<int>();
  pins.DIFF_PRESSURE_PIN = pinData["PDIFF_PIN"].as<int>();
  pins.PITOT_PIN = pinData["PITOT_PIN"].as<int>();
  pins.TEMPERATURE_PIN = pinData["TEMPERATURE_PIN"].as<int>();
  pins.HUMIDITY_PIN = pinData["HUMIDITY_PIN"].as<int>();
  pins.REF_BARO_PIN = pinData["REF_BARO_PIN"].as<int>();
  pins.SERIAL0_RX_PIN = pinData["SERIAL0_RX_PIN"].as<int>();
  pins.SERIAL2_RX_PIN = pinData["SERIAL2_RX_PIN"].as<int>();
  pins.SDA_PIN = pinData["SDA_PIN"].as<int>();
  pins.SCL_PIN = pinData["SCL_PIN"].as<int>();
  pins.SD_CS_PIN = pinData["SD_CS_PIN"].as<int>();
  pins.SD_MISO_PIN = pinData["SD_MISO_PIN"].as<int>();
  pins.SD_SCK_PIN = pinData["SD_SCK_PIN"].as<int>();
  pins.WEMOS_SPARE_PIN_1 = pinData["WEMOS_SPARE_PIN_1"].as<int>();

  // Store output pin values in struct
  pins.VAC_BANK_1_PIN = pinData["VAC_BANK_1_PIN"].as<int>();
  pins.VAC_BANK_2_PIN = pinData["VAC_BANK_2_PIN"].as<int>();
  pins.VAC_BANK_3_PIN = pinData["VAC_BANK_3_PIN"].as<int>();
  pins.VAC_SPEED_PIN = pinData["VAC_SPEED_PIN"].as<int>();
  pins.VAC_BLEED_VALVE_PIN = pinData["VAC_BLEED_VALVE_PIN"].as<int>();
  pins.AVO_STEP_PIN = pinData["AVO_STEP_PIN"].as<int>();
  pins.AVO_DIR_PIN = pinData["AVO_DIR_PIN"].as<int>();
  pins.FLOW_VALVE_STEP_PIN = pinData["FLOW_VALVE_STEP_PIN"].as<int>();
  pins.FLOW_VALVE_DIR_PIN = pinData["FLOW_VALVE_DIR_PIN"].as<int>();
  pins.SD_MOSI_PIN = pinData["SD_MOSI_PIN"].as<int>();
  pins.SERIAL0_TX_PIN = pinData["SERIAL0_TX_PIN"].as<int>();
  pins.SERIAL2_TX_PIN = pinData["SERIAL2_TX_PIN"].as<int>();
}





/***********************************************************
* @name loadPinsData
* @brief Read pins data from pins.json file
* @return pinsData 
***/
void DataHandler::loadPinsData () {

  DataHandler _data;
  Messages _message;
  Hardware _hardware;
  extern struct DeviceStatus status;

  _message.serialPrintf("Loading Pins Data \n");     

  StaticJsonDocument<1024> pinsFileData;
  pinsFileData = _data.loadJSONFile(status.pinsFilename);

  parsePinsData(pinsFileData);

  status.pinsLoaded = true;


  // TEST - save pins data to file
  // String output;
  // serializeJson(pinsFileData, output);
  // this->writeJSONFile(output,"/pintest.json", CONFIG_JSON_SIZE);
  // TEST - print pins data to serial
  // serializeJsonPretty(pinsFileData, Serial);

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
 * @brief buildSSEJsonData
 * @details Package up current bench data into JSON string
 ***/
String DataHandler::buildSSEJsonData()
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

    // Check if we need to round values
    if (settings.rounding_type.indexOf("NONE") > 0) {
        dataJson["FLOW"] = sensorVal.FlowCFM;
        dataJson["MFLOW"] = sensorVal.FlowKGH;
        dataJson["AFLOW"] = sensorVal.FlowADJ;
        dataJson["SFLOW"] = sensorVal.FlowSCFM;
        dataJson["MAF_VOLTS"] = sensorVal.MafVolts;
        dataJson["PREF_VOLTS"] = sensorVal.PRefVolts;
        dataJson["PDIFF_VOLTS"] = sensorVal.PDiffVolts;
        dataJson["PITOT_VOLTS"] = sensorVal.PitotVolts;
    // Round to whole value    
    } else if (settings.rounding_type.indexOf("INTEGER") > 0) {
        dataJson["FLOW"] = round(sensorVal.FlowCFM);
        dataJson["MFLOW"] = round(sensorVal.FlowKGH);
        dataJson["AFLOW"] = round(sensorVal.FlowADJ);
        dataJson["SFLOW"] = round(sensorVal.FlowSCFM);
        dataJson["MAF_VOLTS"] = sensorVal.MafVolts;
        dataJson["PREF_VOLTS"] = sensorVal.PRefVolts;
        dataJson["PDIFF_VOLTS"] = sensorVal.PDiffVolts;
        dataJson["PITOT_VOLTS"] = sensorVal.PitotVolts;
    // Round to half (nearest 0.5)
    } else if (settings.rounding_type.indexOf("HALF") > 0) {
        dataJson["FLOW"] = round(sensorVal.FlowCFM * 2.0 ) / 2.0;
        dataJson["MFLOW"] = round(sensorVal.FlowKGH * 2.0) / 2.0;
        dataJson["AFLOW"] = round(sensorVal.FlowADJ * 2.0) / 2.0;
        dataJson["SFLOW"] = round(sensorVal.FlowSCFM * 2.0) / 2.0;
        dataJson["PREF_VOLTS"] = sensorVal.PRefVolts;
        dataJson["MAF_VOLTS"] = sensorVal.MafVolts;
        dataJson["PDIFF_VOLTS"] = sensorVal.PDiffVolts;
        dataJson["PITOT_VOLTS"] = sensorVal.PitotVolts;
    }

  }  else  {
    dataJson["FLOW"] = 0.0;
    dataJson["MFLOW"] = 0.0;
    dataJson["AFLOW"] = 0.0;
    dataJson["SFLOW"] = 0.0;
    dataJson["MAF_VOLTS"] = 0.0;
    dataJson["PREF_VOLTS"] = 0.0;
    dataJson["PDIFF_VOLTS"] = 0.0;
    dataJson["PITOT_VOLTS"] = 0.0;
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
  if (settings.temp_unit.indexOf("Celcius") > 0) {
    dataJson["TEMP"] = sensorVal.TempDegC;
  } else {
    dataJson["TEMP"] = sensorVal.TempDegF;
  }


  // Bench Type for status pane
  if (settings.bench_type.indexOf("MAF") > 0) {
    dataJson["BENCH_TYPE"] = "MAF";
  } else if (settings.bench_type.indexOf("ORIFICE") > 0) {
    dataJson["BENCH_TYPE"] = "ORIFICE";
  } else if (settings.bench_type.indexOf("VENTURI") > 0) {
    dataJson["BENCH_TYPE"] = "VENTURI";
  } else if (settings.bench_type.indexOf("PITOT") > 0) {
    dataJson["BENCH_TYPE"] = "PITOT";
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
 * @details Temporary server presents upload form and waits for missing files to be present
 * @note Traps program pointer in do-while loop until pins and index files are uploaded
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

      // Upload request handler
      tempServer->on("/api/file/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
          Messages _message;
          _message.debugPrintf("/api/file/upload \n");
          // request->send(200);
          },
          _webserver.fileUpload); 

      // Index page request handler
      tempServer->on("/", HTTP_ANY, [](AsyncWebServerRequest *request){
          Language language;
          Webserver _webserver;
              request->send_P(200, "text/html", language.LANG_INDEX_HTML, _webserver.processLandingPageTemplate); 
          });

      tempServer->onFileUpload(_webserver.fileUpload);
      // tempServer->addHandler(tempServerEvents);
      tempServer->begin();

      _message.serialPrintf("Waiting...\n");
    
    }


    do {
    // capture program pointer in loop and wait for files to be uploaded

        // Process API comms
        if (settings.api_enabled) {        
            if (millis() > status.apiPollTimer) {
                if (Serial.available() > 0) {
                    status.serialData = Serial.read();
                    _api.ParseMessage(status.serialData);
                }
            }                            
        }

        // This is the escape function. When all files are present and loaded we can leave the loop
        // if ((status.configLoaded == true) && (status.pinsLoaded == true) && (status.mafLoaded == true) && (status.GUIexists == true) ){
        if ((status.configLoaded == true) && (status.pinsLoaded == true) && (status.mafLoaded == true)) {
          status.doBootLoop = false; 
          break;
        } 
        // if (status.pinsFilename.isEmpty();

        
        if (!status.configLoaded) {
          if (checkUserFile(CONFIGFILE) ) status.configLoaded = true ;
        }

        if (!status.pinsLoaded) {
         if (checkUserFile(PINSFILE) ) status.pinsLoaded = true ;
        }

        if (!status.mafLoaded) {
          if (checkUserFile(MAFFILE) ) status.mafLoaded = true ;
        }

        if (status.doBootLoop == false) {
          break;
        }

        // if (!status.GUIexists) {
        //   if (checkUserFile(INDEXFILE) ) status.GUIexists = true ;
        // }

        vTaskDelay( 1000 );
    
    } while (status.doBootLoop == true);


    // if the weberver is already running skip sever reset
    if (!status.webserverIsRunning) {
      tempServer->end();  // Stops the server and releases the port
      delay(1000);  // 1000ms delay to ensure the port is released
      tempServer->reset();
    }

}

