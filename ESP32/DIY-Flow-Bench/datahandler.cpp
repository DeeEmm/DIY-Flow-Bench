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
#include <ArduinoJson.h>
#include <AsyncTCP.h>
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

    Hardware _hardware;
    Pins pins;
    Messages _message;
    Comms _comms;
    DataHandler _data;

    StaticJsonDocument<1024> pinData;
    // StaticJsonDocument<1024> mafData;

    // Need to set up the data environment...

    // Start serial comms
    this->beginSerial(); 

    _message.serialPrintf("\r\nDIY Flow Bench\n");                                         
    // Note RELEASE and BUILD_NUMBER defined at compile time
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
    this->loadSettings();
    this->loadLiftData();
    this->loadCalibrationData();

    // Check if config files exists. If not send to boot loop until it is uploaded
    if (SPIFFS.exists("/configuration.json")) {
      this->loadConfiguration();
    } else {
      status.doBootLoop = true;
    }

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
    // TODO - Only if MAF is enabled !!!!!
    // NOTE cannot determine MAF data until config is loaded.
    // Probably need to force a reboot after config is uploaded
    // to force reload of config then MAF
    // or we can just expect that the MAF file is missing and 
    // send user to upload screen as we do with index.html
    if (checkUserFile(MAFFILE)) {
      if (!SPIFFS.exists(status.mafFilename)) {
          status.doBootLoop = true;
          _message.serialPrintf("!! MAF file not found !!\n");  
      }
    } else {
          status.doBootLoop = true;
          _message.serialPrintf("!! MAF file not found !!\n");  
    }

    if (checkUserFile(INDEXFILE)) {
      if (!SPIFFS.exists(status.indexFilename)) {
          status.doBootLoop = true;
          _message.serialPrintf("!! index.html file not found !!\n");  
      }
    } else {
          status.doBootLoop = true;
          _message.serialPrintf("!! index.html file not found !!\n");  
    }

    // Initialise WiFi
    _comms.initaliseWifi();

    // Error Handler - critical files are missing !!
    // Manage on-boarding (index and config file uploading)
    // BootLoop method traps program pointer within loop until files are uplaoded
    if (status.doBootLoop == true) bootLoop();
    
    _hardware.initaliseIO();

    // Start Wire (I2C)
    Wire.begin (pins.SDA_PIN, pins.SCL_PIN); 
    Wire.setClock(100000);
    // Wire.setClock(300000); // ok for wemos D1
    // Wire.setClock(400000);


    // TODO Initialise SD card
    if (config.SD_CARD_IS_ENABLED) {

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

    std::string matchPINS = "PINS";
    std::string matchMAF = "MAF";
    std::string matchINDEX = "index";
    std::string spiffsFile;
    std::string pinsFile;
    std::string mafFile;
    std::string indexFile;

    // Check SPIFFS for pins and MAF files
    while (file)  {
      spiffsFile = file.name();

      if (checkSubstring(spiffsFile.c_str(), matchPINS.c_str()) && (filetype == PINSFILE)) {
        // PINS_*******.json file found
        pinsFile = "/" + spiffsFile;
        _message.serialPrintf("PINS file Found: %s\n", pinsFile.c_str() );  
        status.pinsFilename = pinsFile.c_str();        
        _data.loadPinsData();
        status.pinsLoaded = true;
        return true;
      }   
      
      if (checkSubstring(spiffsFile.c_str(), matchMAF.c_str()) && (filetype == MAFFILE)) {
        // MAF_********.json file found
        mafFile = "/" + spiffsFile;
        _message.serialPrintf("MAF file Found: %s\n", mafFile.c_str() );  
        status.mafFilename = mafFile.c_str();
        loadMAFData();
        status.mafLoaded = true;
        return true;
      }
      
      if (checkSubstring(spiffsFile.c_str(), matchINDEX.c_str()) && (filetype == INDEXFILE)) {
        // index.html file found
        indexFile = "/" + spiffsFile;
        _message.serialPrintf("index file Found: %s\n", indexFile.c_str() );  
        status.indexFilename = indexFile.c_str();
        status.GUIexists = true;
        return true;
      }

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
  configData["CONF_WIFI_SSID"] = settings.wifi_ssid;
  configData["CONF_WIFI_PSWD"] = settings.wifi_pswd;
  configData["CONF_WIFI_AP_SSID"] = settings.wifi_ap_ssid;
  configData["CONF_WIFI_AP_PSWD"] = settings.wifi_ap_pswd;
  configData["CONF_HOSTNAME"] = settings.hostname;
  configData["CONF_WIFI_TIMEOUT"] = settings.wifi_timeout;
  configData["CONF_MAF_HOUSING_DIAMETER"] = settings.maf_housing_diameter;
  configData["CONF_REFRESH_RATE"] = settings.refresh_rate;
  configData["CONF_MIN_BENCH_PRESSURE"] = settings.min_bench_pressure;
  configData["CONF_MIN_FLOW_RATE"] = settings.min_flow_rate;
  configData["DATA_FILTER_TYPE"] = settings.data_filter_type;
  configData["ROUNDING_TYPE"] = settings.rounding_type;
  configData["FLOW_DECIMAL_LENGTH"] = settings.flow_decimal_length;
  configData["GEN_DECIMAL_LENGTH"] = settings.gen_decimal_length;
  configData["CONF_CYCLIC_AVERAGE_BUFFER"] = settings.cyc_av_buffer;
  configData["CONF_MAF_MIN_VOLTS"] = settings.maf_min_volts;
  configData["CONF_API_DELIM"] = settings.api_delim;
  configData["CONF_SERIAL_BAUD_RATE"] = settings.serial_baud_rate;
  configData["ADJ_FLOW_DEPRESSION"] = settings.adj_flow_depression;
  configData["STANDARD_REFERENCE"] = settings.standardReference;
  configData["STD_ADJ_FLOW"] = settings.std_adj_flow;
  configData["DATAGRAPH_MAX"] = settings.dataGraphMax;
  configData["TEMP_UNIT"] = settings.temp_unit;
  configData["VALVE_LIFT_INTERVAL"] = settings.valveLiftInterval;
  configData["CONF_SHOW_ALARMS"] = settings.show_alarms;
  configData["BENCH_TYPE"] = settings.bench_type;
  configData["CONF_CAL_FLOW_RATE"] = settings.cal_flow_rate;
  configData["CONF_CAL_REF_PRESS"] = settings.cal_ref_press;
  configData["ORIFICE1_FLOW_RATE"] = settings.orificeOneFlow;
  configData["ORIFICE1_TEST_PRESSURE"] = settings.orificeOneDepression;
  configData["ORIFICE2_FLOW_RATE"] = settings.orificeTwoFlow;
  configData["ORIFICE2_TEST_PRESSURE"] = settings.orificeThreeDepression;
  configData["ORIFICE3_FLOW_RATE"] = settings.orificeThreeFlow;
  configData["ORIFICE4_FLOW_RATE"] = settings.orificeFourFlow;
  configData["ORIFICE4_TEST_PRESSURE"] = settings.orificeFourDepression;
  configData["ORIFICE5_FLOW_RATE"] = settings.orificeFiveFlow;
  configData["ORIFICE5_TEST_PRESSURE"] = settings.orificeFiveDepression;
  configData["ORIFICE6_FLOW_RATE"] = settings.orificeSixFlow;
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
* @brief loadConfiguration
* @details read settings from config.json file and loads into global struct
* @note Repalces original config.h file
***/ 
StaticJsonDocument<CONFIG_JSON_SIZE> DataHandler::loadConfiguration () {

  extern struct Configuration config;

  StaticJsonDocument<CONFIG_JSON_SIZE> configurationJSON;

  Messages _message;

  _message.serialPrintf("Loading Configuration \n");     

  if (SPIFFS.exists("/configuration.json"))  {

    configurationJSON = loadJSONFile("/configuration.json");

    // TEST print configuration.json
    // serializeJsonPretty(configurationJSON, Serial);

    // strcpy(config.wifi_ssid, configurationJSON["CONF_WIFI_SSID"]);
    // strcpy(config.wifi_pswd, configurationJSON["CONF_WIFI_PSWD"]);
    // config.orificeSixDepression = configurationJSON["ORIFICE6_TEST_PRESSURE"].as<double>();

    config.SD_CARD_IS_ENABLED = configurationJSON["SD_CARD_IS_ENABLED"].as<bool>();
    config.MIN_TEST_PRESSURE_PERCENTAGE = configurationJSON["MIN_TEST_PRESSURE_PERCENTAGE"].as<int>();
    config.PIPE_RADIUS_IN_FEET = configurationJSON["PIPE_RADIUS_IN_FEET"].as<int>();

    config.VCC_3V3_TRIMPOT = configurationJSON["VCC_3V3_TRIMPOT"].as<int>();
    config.VCC_5V_TRIMPOT = configurationJSON["VCC_5V_TRIMPOT"].as<int>();
    config.USE_FIXED_3_3V_VALUE =  configurationJSON["USE_FIXED_3_3V_VALUE"].as<bool>();
    config.USE_FIXED_5V_VALUE = configurationJSON["USE_FIXED_5V_VALUE"].as<bool>();

    config.BME280_IS_ENABLED = configurationJSON["BME280_IS_ENABLED"].as<bool>();
    config.BME280_I2C_ADDR = configurationJSON["BME280_I2C_ADDR"];
    config.BME280_SCAN_DELAY_MS =  configurationJSON["BME280_SCAN_DELAY_MS"].as<int>();

    config.BME680_IS_ENABLED = configurationJSON["BME680_IS_ENABLED"].as<bool>();
    config.BME680_I2C_ADDR = configurationJSON["BME680_I2C_ADDR"];
    config.BME680_SCAN_DELAY_MS =  configurationJSON["BME680_SCAN_DELAY_MS"].as<int>();

    config.ADC_TYPE =  configurationJSON["ADC_TYPE"].as<int>();
    config.ADC_I2C_ADDR = configurationJSON["ADC_I2C_ADDR"].as<int>();
    config.ADC_SCAN_DELAY_MS = configurationJSON["ADC_SCAN_DELAY_MS"].as<bool>();
    config.ADC_MAX_RETRIES  =  configurationJSON["ADC_MAX_RETRIES"].as<int>();
    config.ADC_MAX_RETRIES = configurationJSON["ADC_MAX_RETRIES"].as<int>();
    config.ADC_RANGE = configurationJSON["ADC_RANGE"].as<int>();
    config.ADC_GAIN = configurationJSON["ADC_GAIN"].as<int>();
    
    config.MAF_SRC_TYPE =  configurationJSON["MAF_SRC_TYPE"].as<int>();
    config.MAF_MV_TRIMPOT = configurationJSON["MAF_MV_TRIMPOT"].as<int>();
    config.MAF_ADC_CHANNEL = configurationJSON["MAF_ADC_CHANNEL"].as<int>();

    config.PREF_SENSOR_TYPE = configurationJSON["PREF_SENSOR_TYPE"].as<int>();
    config.FIXED_REF_PRESS_VALUE = configurationJSON["FIXED_REF_PRESS_VALUE"].as<bool>();
    config.PREF_MV_TRIMPOT = configurationJSON["PREF_MV_TRIMPOT"].as<int>();
    config.PREF_ANALOG_SCALE = configurationJSON["PREF_ANALOG_SCALE"].as<double>();
    config.PREF_ADC_CHANNEL = configurationJSON["PREF_ADC_CHANNEL"].as<int>();

    config.PDIFF_SENSOR_TYPE = configurationJSON["PDIFF_SENSOR_TYPE"].as<int>();
    config.FIXED_DIFF_PRESS_VALUE = configurationJSON["FIXED_DIFF_PRESS_VALUE"].as<int>();
    config.PDIFF_MV_TRIMPOT = configurationJSON["PDIFF_MV_TRIMPOT"].as<int>();
    config.PDIFF_ANALOG_SCALE = configurationJSON["PDIFF_ANALOG_SCALE"].as<double>();
    config.PDIFF_ADC_CHANNEL = configurationJSON["PDIFF_ADC_CHANNEL"].as<int>();

    config.PITOT_SENSOR_TYPE =  configurationJSON["PITOT_SENSOR_TYPE"].as<int>();
    config.PITOT_MV_TRIMPOT = configurationJSON["PITOT_MV_TRIMPOT"].as<int>();
    config.PITOT_ANALOG_SCALE = configurationJSON["PITOT_ANALOG_SCALE"].as<double>();
    config.PITOT_ADC_CHANNEL = configurationJSON["PITOT_ADC_CHANNEL"].as<int>();

    config.BARO_SENSOR_TYPE = configurationJSON["BARO_SENSOR_TYPE"];
    config.FIXED_BARO_VALUE = configurationJSON["FIXED_BARO_VALUE"].as<double>();
    config.BARO_ANALOG_SCALE = configurationJSON["BARO_ANALOG_SCALE"].as<double>();
    config.BARO_MV_TRIMPOT = configurationJSON["BARO_MV_TRIMPOT"].as<int>();
    config.BARO_FINE_ADJUST = configurationJSON["BARO_FINE_ADJUST"].as<double>();
    config.startupBaroScalingFactor = configurationJSON["startupBaroScalingFactor"].as<double>();
    config.startupBaroScalingOffset = configurationJSON["startupBaroScalingOffset"].as<double>();
    config.SEALEVELPRESSURE_HPA = configurationJSON["SEALEVELPRESSURE_HPA"].as<double>();

    config.TEMP_SENSOR_TYPE = configurationJSON["TEMP_SENSOR_TYPE"];
    config.FIXED_TEMP_VALUE = configurationJSON["FIXED_TEMP_VALUE"].as<double>();
    config.TEMP_ANALOG_SCALE = configurationJSON["TEMP_ANALOG_SCALE"].as<double>();
    config.TEMP_MV_TRIMPOT = configurationJSON["TEMP_MV_TRIMPOT"].as<int>();
    config.TEMP_FINE_ADJUST = configurationJSON["TEMP_FINE_ADJUST"].as<double>();

    config.RELH_SENSOR_TYPE = int(configurationJSON["RELH_SENSOR_TYPE"]);
    config.FIXED_RELH_VALUE = configurationJSON["FIXED_RELH_VALUE"].as<double>();
    config.RELH_ANALOG_SCALE = configurationJSON["RELH_ANALOG_SCALE"].as<double>();
    config.RELH_FINE_ADJUST = configurationJSON["RELH_FINE_ADJUST"].as<double>();

    config.SWIRL_IS_ENABLED = configurationJSON["SWIRL_IS_ENABLED"].as<bool>();
    
    // config.PLACEHOLDER = configurationJSON["PLACEHOLDER"])


  } else {
    _message.serialPrintf("Configuration file not found \n");
  }
  
  return configurationJSON;  

}










/***********************************************************
* @brief loadSettings
* @details read settings settings.json file and loads into global struct
***/ 
StaticJsonDocument<SETTINGS_JSON_SIZE> DataHandler::loadSettings () {

  extern struct BenchSettings settings;

  StaticJsonDocument<SETTINGS_JSON_SIZE> configData;
  DataHandler _data;
  Messages _message;

  _message.serialPrintf("Loading Bench Settings \n");     

  if (SPIFFS.exists("/settings.json"))  {

    configData = _data.loadJSONFile("/settings.json");

    strcpy(settings.wifi_ssid, configData["CONF_WIFI_SSID"]);
    strcpy(settings.wifi_pswd, configData["CONF_WIFI_PSWD"]);
    strcpy(settings.wifi_ap_ssid, configData["CONF_WIFI_AP_SSID"]);
    strcpy(settings.wifi_ap_pswd,configData["CONF_WIFI_AP_PSWD"]);
    strcpy(settings.hostname, configData["CONF_HOSTNAME"]);
    settings.wifi_timeout = configData["CONF_WIFI_TIMEOUT"].as<int>();
    settings.maf_housing_diameter = configData["CONF_MAF_HOUSING_DIAMETER"].as<int>();
    settings.refresh_rate = configData["CONF_REFRESH_RATE"].as<int>();
    settings.min_bench_pressure  = configData["CONF_MIN_BENCH_PRESSURE"].as<int>();
    settings.min_flow_rate = configData["CONF_MIN_FLOW_RATE"].as<int>();
    strcpy(settings.data_filter_type, configData["DATA_FILTER_TYPE"]);
    strcpy(settings.rounding_type, configData["ROUNDING_TYPE"]);
    settings.flow_decimal_length, configData["FLOW_DECIMAL_LENGTH"];
    settings.gen_decimal_length, configData["GEN_DECIMAL_LENGTH"];
    settings.cyc_av_buffer  = configData["CONF_CYCLIC_AVERAGE_BUFFER"].as<int>();
    settings.maf_min_volts  = configData["CONF_MAF_MIN_VOLTS"].as<int>();
    strcpy(settings.api_delim, configData["CONF_API_DELIM"]);
    settings.serial_baud_rate = configData["CONF_SERIAL_BAUD_RATE"].as<long>();
    settings.show_alarms = configData["CONF_SHOW_ALARMS"].as<bool>();
    configData["ADJ_FLOW_DEPRESSION"] = settings.adj_flow_depression;
    configData["STANDARD_REFERENCE"] = settings.standardReference;
    configData["STD_ADJ_FLOW"] = settings.std_adj_flow;
    configData["DATAGRAPH_MAX"] = settings.dataGraphMax;
    configData["TEMP_UNIT"] = settings.temp_unit;
    configData["VALVE_LIFT_INTERVAL"] = settings.valveLiftInterval;
    strcpy(settings.bench_type, configData["BENCH_TYPE"]);
    settings.cal_flow_rate = configData["CONF_CAL_FLOW_RATE"].as<double>();
    settings.cal_ref_press = configData["CONF_CAL_REF_PRESS"].as<double>();
    settings.orificeOneFlow = configData["ORIFICE1_FLOW_RATE"].as<double>();
    settings.orificeOneDepression = configData["ORIFICE1_TEST_PRESSURE"].as<double>();
    settings.orificeTwoFlow = configData["ORIFICE2_FLOW_RATE"].as<double>();
    settings.orificeTwoDepression = configData["ORIFICE2_TEST_PRESSURE"].as<double>();
    settings.orificeThreeFlow = configData["ORIFICE3_FLOW_RATE"].as<double>();
    settings.orificeThreeDepression = configData["ORIFICE3_TEST_PRESSURE"].as<double>();
    settings.orificeFourFlow = configData["ORIFICE4_FLOW_RATE"].as<double>();
    settings.orificeFourDepression = configData["ORIFICE4_TEST_PRESSURE"].as<double>();
    settings.orificeFiveFlow = configData["ORIFICE5_FLOW_RATE"].as<double>();
    settings.orificeFiveDepression = configData["ORIFICE5_TEST_PRESSURE"].as<double>();
    settings.orificeSixFlow = configData["ORIFICE6_FLOW_RATE"].as<double>();
    settings.orificeSixDepression = configData["ORIFICE6_TEST_PRESSURE"].as<double>();

  } else {
    _message.serialPrintf("Bench Settings file not found \n");
  }
  
  return configData;  

}







/***********************************************************
* @brief load MAF File
* @details read MAF data from MAF.json file
***/ 
void DataHandler::loadMAFData () {

  DataHandler _data;
  Messages _message;
  extern struct DeviceStatus status;

  StaticJsonDocument<MAF_JSON_SIZE> mafData;

  // mafData = _data.loadJSONFile(status.mafFilename);

  mafData = _data.loadJSONFile(status.mafFilename);

  strcpy(status.mafSensorType, mafData["sensor_type"]); // "ACDELCO_92281162"
  strcpy(status.mafOutputType, mafData["output_type"]); // "voltage / frequency"
  strcpy(status.mafUnits, mafData["maf_units"]); // "KG_H / MG_S"
  status.mafScaling = mafData["maf_scaling"]; // 0.1 / 0.01 / 0.001
  status.mafDiameter = mafData["maf_diameter"]; // 94 (mm)

  // Load MAF lookup table into JSON object
  status.mafJsonObject = mafData["maf_lookup_table"];

  // Print size of MAF data to serial
  _message.serialPrintf("MAF Data size: %u \n", mafData.memoryUsage()); 
  _message.serialPrintf("MAF Data JSON Object size: %u \n", status.mafJsonObject.memoryUsage()); 
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
  pins.SPEED_SENSOR_PIN = pinData["SPEED_SENSOR_PIN"].as<int>();
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
 * @brief getDataJSON
 * @details Package up current bench data into JSON string
 ***/
String DataHandler::getDataJSON()
{

  extern struct DeviceStatus status;
  extern struct BenchSettings settings;
  extern struct SensorData sensorVal;
  extern struct CalibrationData calVal;

  Hardware _hardware;
  Calculations _calculations;

  String jsonString;

  StaticJsonDocument<DATA_JSON_SIZE> dataJson;

  // Reference pressure
  dataJson["PREF"] = sensorVal.PRefH2O;

  double flowComp = fabs(sensorVal.FlowCFM);
  double pRefComp = fabs(sensorVal.PRefH2O);

  // Flow Rate
  if ((flowComp > settings.min_flow_rate) && (pRefComp > settings.min_bench_pressure))  {

    // Check if we need to round values
     if (strstr(String(settings.rounding_type).c_str(), String("NONE").c_str())) {
        dataJson["FLOW"] = sensorVal.FlowCFM;
        dataJson["MFLOW"] = sensorVal.FlowKGH;
        dataJson["AFLOW"] = sensorVal.FlowADJ;
        dataJson["SFLOW"] = sensorVal.FlowSCFM;
    // Round to whole value    
    } else if (strstr(String(settings.rounding_type).c_str(), String("INTEGER").c_str())) {
        dataJson["FLOW"] = round(sensorVal.FlowCFM);
        dataJson["MFLOW"] = round(sensorVal.FlowKGH);
        dataJson["AFLOW"] = round(sensorVal.FlowADJ);
        dataJson["SFLOW"] = round(sensorVal.FlowSCFM);
    // Round to half (nearest 0.5)
    } else if (strstr(String(settings.rounding_type).c_str(), String("HALF").c_str())) {
        dataJson["FLOW"] = round(sensorVal.FlowCFM * 2.0 ) / 2.0;
        dataJson["MFLOW"] = round(sensorVal.FlowKGH * 2.0) / 2.0;
        dataJson["AFLOW"] = round(sensorVal.FlowADJ * 2.0) / 2.0;
        dataJson["SFLOW"] = round(sensorVal.FlowSCFM * 2.0) / 2.0;
    }

  }  else  {
    dataJson["FLOW"] = 0.0;
    dataJson["MFLOW"] = 0.0;
    dataJson["AFLOW"] = 0.0;
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
  if (strstr(String(settings.temp_unit).c_str(), String("Celcius").c_str())){
    dataJson["TEMP"] = sensorVal.TempDegC;
  } else {
    dataJson["TEMP"] = sensorVal.TempDegF;
  }


  // Bench Type for status pane
  if (strstr(String(settings.bench_type).c_str(), String("MAF").c_str())) {
    dataJson["BENCH_TYPE"] = "MAF";
  } else if (strstr(String(settings.bench_type).c_str(), String("ORIFICE").c_str())) {
    dataJson["BENCH_TYPE"] = "ORIFICE";
  } else if (strstr(String(settings.bench_type).c_str(), String("VENTURI").c_str())) {
    dataJson["BENCH_TYPE"] = "VENTURI";
  } else if (strstr(String(settings.bench_type).c_str(), String("PITOT").c_str())) {
    dataJson["BENCH_TYPE"] = "PITOT";
  }


  dataJson["BARO"] = sensorVal.BaroHPA; // GUI  displays mbar (hPa)
  dataJson["RELH"] = sensorVal.RelH;

  // Pitot
  dataJson["PITOT"] = sensorVal.PitotVelocity;
  // dataJson["PITOT_DELTA"] = fabs(round(sensorVal.PitotDelta));
  dataJson["PITOT_DELTA"] = round(sensorVal.PitotH2O);
  
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
      tempServerEvents = new AsyncEventSource("/events");

      // Upload request handler
      tempServer->on("/api/file/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
          Messages _message;
          _message.debugPrintf("/api/file/upload \n");
          request->send(200);
          },
          _webserver.fileUpload); 

      // Index page request handler
      tempServer->on("/", HTTP_ANY, [](AsyncWebServerRequest *request){
          Language language;
          Webserver _webserver;
              request->send_P(200, "text/html", language.LANG_INDEX_HTML, _webserver.processLandingPageTemplate); 
          });

      tempServer->onFileUpload(_webserver.fileUpload);
      tempServer->addHandler(tempServerEvents);
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
        if ((status.pinsLoaded == true) && (status.mafLoaded == true) && (status.GUIexists == true) && (SPIFFS.exists("/configuration.json"))){
          status.doBootLoop = false; 
          break;
        } 
        // if (status.pinsFilename.isEmpty();

        vTaskDelay( 1 );
    
    } while (status.doBootLoop == true);


    // if the weberver is already running skip sever reset
    if (!status.webserverIsRunning) {
      tempServer->end();  // Stops the server and releases the port
      delay(1000);  // 1000ms delay to ensure the port is released
      tempServer->reset();
    }

}



