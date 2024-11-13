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

#include "structs.h"
#include "constants.h"
#include "hardware.h"
#include "messages.h"
#include "calculations.h"
#include "comms.h"
#include "webserver.h"
#include "API.h"
#include "version.h"


void DataHandler::begin() {

    extern struct ConfigSettings config;
    extern struct Language language;
    extern struct DeviceStatus status;

    Hardware _hardware;
    Pins pins;
    Messages _message;
    Comms _comms;

    StaticJsonDocument<1024> pinData;
    bool doBootLoop = false;

    // Need to set up the data environment...

    // Start serial comms
    this->beginSerial(); 

    _message.serialPrintf("\r\nDIY Flow Bench");                                         
    // _message.serialPrintf("\nDIYFB Version: %s \nBuild: %s \nGUI: %s \n", RELEASE, BUILD_NUMBER, GUI_BUILD_NUMBER);                                         
    _message.serialPrintf("\nDIYFB Version: %s \nBuild: %s \n", RELEASE, BUILD_NUMBER);                                         

    // initialise SPIFFS Filesystem
    _message.serialPrintf("Initialising File System \n"); // TODO:  Initialise messages once pins / serial is running
    if (SPIFFS.begin()) {
    } else {
        _message.serialPrintf("...Failed\n");
        #if defined FORMAT_FILESYSTEM_IF_FAILED
            SPIFFS.format();
            _message.serialPrintf("!! File System Formatted !!\n");
        #endif
    }

    // Check if config / calibration / liftdata json files exist. If not create them.
    if (!SPIFFS.exists("/config.json")) createConfigFile();
    if (!SPIFFS.exists("/liftdata.json")) createLiftDataFile();
    if (!SPIFFS.exists("/cal.json")) createCalibrationFile();

    // load json configuration files from SPIFFS memory
    this->loadConfig();
    this->loadLiftData();
    this->loadCalibrationData();

    if (SPIFFS.exists("/pins.json"))  {
        pinData = loadJSONFile("/pins.json");
        if (!pinData.containsKey("BOARD_TYPE")) doBootLoop = true;
    } else {
        doBootLoop = true;
    }

    // Check for index file
    if (!SPIFFS.exists("/index.html")) doBootLoop = true;

    // Initialise WiFi
    _comms.initaliseWifi();

    // Error Handler - critical files are missing
    // Manage on-boarding (index and config file uploading)
    if (doBootLoop == true) bootLoop();

    // Get a reference to the root object
    JsonObject jsonObj = pinData.as<JsonObject>();

    _hardware.assignIO(jsonObj);

    // Start Wire (I2C)
    Wire.begin (pins.SDA_PIN, pins.SCL_PIN); 
    Wire.setClock(100000);
    // Wire.setClock(300000); // ok for wemos D1
    // Wire.setClock(400000);


    // TODO Initialise SD card
    #ifdef SD_CARD_IS_ENABLED

    // test code from https://github.com/espressif/arduino-esp32/blob/master/libraries/SD/examples/SD_Test/SD_Test.ino

    _message.serialPrintf("=== SDCARD File system info === \n");

    int sck = SD_SCK_PIN;
    int miso = SD_MISO_PIN;
    int mosi = SD_MOSI_PIN;
    int cs = SD_CS_PIN;

    SPIClass spi = SPIClass(VSPI);

        SPI.begin(sck, miso, mosi, cs);
        if (!SD.begin(cs, spi, 2000000U)) {
        Serial.println("Card Mount Failed");

        } else {

        uint8_t cardType = SD.cardType();

        if (cardType == CARD_NONE) {
            Serial.println("No SD card attached");
        }

        Serial.print("SD Card Type: ");
        if (cardType == CARD_MMC) {
            Serial.println("MMC");
        } else if (cardType == CARD_SD) {
            Serial.println("SDSC");
        } else if (cardType == CARD_SDHC) {
            Serial.println("SDHC");
        } else {
            Serial.println("UNKNOWN");
        }

        uint64_t cardSize = SD.cardSize() / (1024 * 1024);
        Serial.printf("SD Card Size: %lluMB\n", cardSize);

        listDir(SD, "/", 0);
        createDir(SD, "/mydir");
        listDir(SD, "/", 0);
        removeDir(SD, "/mydir");
        listDir(SD, "/", 2);
        writeFile(SD, "/hello.txt", "Hello ");
        appendFile(SD, "/hello.txt", "World!\n");
        readFile(SD, "/hello.txt");
        deleteFile(SD, "/foo.txt");
        renameFile(SD, "/hello.txt", "/foo.txt");
        readFile(SD, "/foo.txt");
        testFileIO(SD, "/test.txt");
        Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
        Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
        }

    #endif

    
    // Display Filesystem Stats
    status.spiffs_mem_size = SPIFFS.totalBytes();
    status.spiffs_mem_used = SPIFFS.usedBytes();

    _message.serialPrintf("=== SPIFFS File system info === \n");
    _message.serialPrintf("Total space:      %s \n", byteDecode(status.spiffs_mem_size));
    _message.serialPrintf("Total space used: %s \n", byteDecode(status.spiffs_mem_used));

}




/***********************************************************
* @brief createConfig
* @details Create basic minimum configuration json file
* @note Called from DataHandler::begin() if config.json not found
***/
void DataHandler::createConfigFile () {

  extern struct ConfigSettings config;
  Messages _message;
  String jsonString;  
  StaticJsonDocument<CONFIG_JSON_SIZE> configData;

  _message.serialPrintf("Creating config.json file... \n"); 
 
  configData["PAGE_TITLE"] = config.pageTitle;
  configData["CONF_WIFI_SSID"] = config.wifi_ssid;
  configData["CONF_WIFI_PSWD"] = config.wifi_pswd;
  configData["CONF_WIFI_AP_SSID"] = config.wifi_ap_ssid;
  configData["CONF_WIFI_AP_PSWD"] = config.wifi_ap_pswd;
  configData["CONF_HOSTNAME"] = config.hostname;
  configData["CONF_WIFI_TIMEOUT"] = config.wifi_timeout;
  configData["CONF_MAF_HOUSING_DIAMETER"] = config.maf_housing_diameter;
  configData["CONF_REFRESH_RATE"] = config.refresh_rate;
  configData["CONF_MIN_BENCH_PRESSURE"] = config.min_bench_pressure;
  configData["CONF_MIN_FLOW_RATE"] = config.min_flow_rate;
  configData["DATA_FILTER_TYPE"] = config.data_filter_type;
  configData["ROUNDING_TYPE"] = config.rounding_type;
  configData["FLOW_DECIMAL_LENGTH"] = config.flow_decimal_length;
  configData["GEN_DECIMAL_LENGTH"] = config.gen_decimal_length;
  configData["CONF_CYCLIC_AVERAGE_BUFFER"] = config.cyc_av_buffer;
  configData["CONF_MAF_MIN_VOLTS"] = config.maf_min_volts;
  configData["CONF_API_DELIM"] = config.api_delim;
  configData["CONF_SERIAL_BAUD_RATE"] = config.serial_baud_rate;
  configData["ADJ_FLOW_DEPRESSION"] = config.adj_flow_depression;
  configData["STANDARD_REFERENCE"] = config.standardReference;
  configData["STD_ADJ_FLOW"] = config.std_adj_flow;
  configData["DATAGRAPH_MAX"] = config.dataGraphMax;
  configData["TEMP_UNIT"] = config.temp_unit;
  configData["VALVE_LIFT_INTERVAL"] = config.valveLiftInterval;
  configData["CONF_SHOW_ALARMS"] = config.show_alarms;
  configData["BENCH_TYPE"] = config.bench_type;
  configData["CONF_CAL_FLOW_RATE"] = config.cal_flow_rate;
  configData["CONF_CAL_REF_PRESS"] = config.cal_ref_press;
  configData["ORIFICE1_FLOW_RATE"] = config.orificeOneFlow;
  configData["ORIFICE1_TEST_PRESSURE"] = config.orificeOneDepression;
  configData["ORIFICE2_FLOW_RATE"] = config.orificeTwoFlow;
  configData["ORIFICE2_TEST_PRESSURE"] = config.orificeThreeDepression;
  configData["ORIFICE3_FLOW_RATE"] = config.orificeThreeFlow;
  configData["ORIFICE4_FLOW_RATE"] = config.orificeFourFlow;
  configData["ORIFICE4_TEST_PRESSURE"] = config.orificeFourDepression;
  configData["ORIFICE5_FLOW_RATE"] = config.orificeFiveFlow;
  configData["ORIFICE5_TEST_PRESSURE"] = config.orificeFiveDepression;
  configData["ORIFICE6_FLOW_RATE"] = config.orificeSixFlow;
  configData["ORIFICE7_TEST_PRESSURE"] = config.orificeSixDepression;

  serializeJsonPretty(configData, jsonString);
  writeJSONFile(jsonString, "/config.json", CONFIG_JSON_SIZE);

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
* @note Called from DataHandler::begin() if config.json not found
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
StaticJsonDocument<CONFIG_JSON_SIZE> DataHandler::loadJSONFile(String filename)
{

  Messages _message;

//   extern struct Language language;

  // Allocate the memory pool on the stack.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<CONFIG_JSON_SIZE> jsonData;

  if (SPIFFS.exists(filename))  {
    File jsonFile = SPIFFS.open(filename, FILE_READ);

    if (!jsonFile)    {
    //   _message.Handler(language.LANG_ERROR_LOADING_FILE);
      _message.statusPrintf("Failed to open file for reading \n");
    }    else    {
      size_t size = jsonFile.size();
      if (size > CONFIG_JSON_SIZE)    {

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
* @brief loadConfig
* @details read configuration data from config.json file and loads into global struct
***/ 
StaticJsonDocument<CONFIG_JSON_SIZE> DataHandler::loadConfig () {

  extern struct ConfigSettings config;

  StaticJsonDocument<CONFIG_JSON_SIZE> configData;
  DataHandler _data;
  Messages _message;

  _message.serialPrintf("Loading Configuration \n");     

  if (SPIFFS.exists("/config.json"))  {

    configData = _data.loadJSONFile("/config.json");

    strcpy(config.wifi_ssid, configData["CONF_WIFI_SSID"]);
    strcpy(config.wifi_pswd, configData["CONF_WIFI_PSWD"]);
    strcpy(config.wifi_ap_ssid, configData["CONF_WIFI_AP_SSID"]);
    strcpy(config.wifi_ap_pswd,configData["CONF_WIFI_AP_PSWD"]);
    strcpy(config.hostname, configData["CONF_HOSTNAME"]);
    config.wifi_timeout = configData["CONF_WIFI_TIMEOUT"].as<int>();
    config.maf_housing_diameter = configData["CONF_MAF_HOUSING_DIAMETER"].as<int>();
    config.refresh_rate = configData["CONF_REFRESH_RATE"].as<int>();
    config.min_bench_pressure  = configData["CONF_MIN_BENCH_PRESSURE"].as<int>();
    config.min_flow_rate = configData["CONF_MIN_FLOW_RATE"].as<int>();
    strcpy(config.data_filter_type, configData["DATA_FILTER_TYPE"]);
    strcpy(config.rounding_type, configData["ROUNDING_TYPE"]);
    config.flow_decimal_length, configData["FLOW_DECIMAL_LENGTH"];
    config.gen_decimal_length, configData["GEN_DECIMAL_LENGTH"];
    config.cyc_av_buffer  = configData["CONF_CYCLIC_AVERAGE_BUFFER"].as<int>();
    config.maf_min_volts  = configData["CONF_MAF_MIN_VOLTS"].as<int>();
    strcpy(config.api_delim, configData["CONF_API_DELIM"]);
    config.serial_baud_rate = configData["CONF_SERIAL_BAUD_RATE"].as<long>();
    config.show_alarms = configData["CONF_SHOW_ALARMS"].as<bool>();
    configData["ADJ_FLOW_DEPRESSION"] = config.adj_flow_depression;
    configData["STANDARD_REFERENCE"] = config.standardReference;
    configData["STD_ADJ_FLOW"] = config.std_adj_flow;
    configData["DATAGRAPH_MAX"] = config.dataGraphMax;
    configData["TEMP_UNIT"] = config.temp_unit;
    configData["VALVE_LIFT_INTERVAL"] = config.valveLiftInterval;
    strcpy(config.bench_type, configData["BENCH_TYPE"]);
    config.cal_flow_rate = configData["CONF_CAL_FLOW_RATE"].as<double>();
    config.cal_ref_press = configData["CONF_CAL_REF_PRESS"].as<double>();
    config.orificeOneFlow = configData["ORIFICE1_FLOW_RATE"].as<double>();
    config.orificeOneDepression = configData["ORIFICE1_TEST_PRESSURE"].as<double>();
    config.orificeTwoFlow = configData["ORIFICE2_FLOW_RATE"].as<double>();
    config.orificeTwoDepression = configData["ORIFICE2_TEST_PRESSURE"].as<double>();
    config.orificeThreeFlow = configData["ORIFICE3_FLOW_RATE"].as<double>();
    config.orificeThreeDepression = configData["ORIFICE3_TEST_PRESSURE"].as<double>();
    config.orificeFourFlow = configData["ORIFICE4_FLOW_RATE"].as<double>();
    config.orificeFourDepression = configData["ORIFICE4_TEST_PRESSURE"].as<double>();
    config.orificeFiveFlow = configData["ORIFICE5_FLOW_RATE"].as<double>();
    config.orificeFiveDepression = configData["ORIFICE5_TEST_PRESSURE"].as<double>();
    config.orificeSixFlow = configData["ORIFICE6_FLOW_RATE"].as<double>();
    config.orificeSixDepression = configData["ORIFICE6_TEST_PRESSURE"].as<double>();

  } else {
    _message.serialPrintf("Configuration file not found \n");
  }
  
  return configData;  

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
* loadCalibration
* Read calibration data from cal.json file
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
  extern struct ConfigSettings config;
  extern struct SensorData sensorVal;

  Hardware _hardware;
  Calculations _calculations;

  String jsonString;

  StaticJsonDocument<DATA_JSON_SIZE> dataJson;

  // Reference pressure
  dataJson["PREF"] = sensorVal.PRefH2O;

  double flowComp = fabs(sensorVal.FlowCFM);
  double pRefComp = fabs(sensorVal.PRefH2O);

  // Flow Rate
  if ((flowComp > config.min_flow_rate) && (pRefComp > config.min_bench_pressure))  {

    // Check if we need to round values
     if (strstr(String(config.rounding_type).c_str(), String("NONE").c_str())) {
        dataJson["FLOW"] = sensorVal.FlowCFM;
        dataJson["MFLOW"] = sensorVal.FlowKGH;
        dataJson["AFLOW"] = sensorVal.FlowADJ;
        dataJson["SFLOW"] = sensorVal.FlowSCFM;
    // Round to whole value    
    } else if (strstr(String(config.rounding_type).c_str(), String("INTEGER").c_str())) {
        dataJson["FLOW"] = round(sensorVal.FlowCFM);
        dataJson["MFLOW"] = round(sensorVal.FlowKGH);
        dataJson["AFLOW"] = round(sensorVal.FlowADJ);
        dataJson["SFLOW"] = round(sensorVal.FlowSCFM);
    // Round to half (nearest 0.5)
    } else if (strstr(String(config.rounding_type).c_str(), String("HALF").c_str())) {
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
  dataJson["PADJUST"] = config.adj_flow_depression;

  // Standard reference
  switch (config.standardReference) {

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
  if (strstr(String(config.temp_unit).c_str(), String("Celcius").c_str())){
    dataJson["TEMP"] = sensorVal.TempDegC;
  } else {
    dataJson["TEMP"] = sensorVal.TempDegF;
  }


  // Bench Type for status pane
  if (strstr(String(config.bench_type).c_str(), String("MAF").c_str())) {
    dataJson["BENCH_TYPE"] = "MAF";
  } else if (strstr(String(config.bench_type).c_str(), String("ORIFICE").c_str())) {
    dataJson["BENCH_TYPE"] = "ORIFICE";
  } else if (strstr(String(config.bench_type).c_str(), String("VENTURI").c_str())) {
    dataJson["BENCH_TYPE"] = "VENTURI";
  } else if (strstr(String(config.bench_type).c_str(), String("PITOT").c_str())) {
    dataJson["BENCH_TYPE"] = "PITOT";
  }


  dataJson["BARO"] = sensorVal.BaroHPA; // GUI  displays mbar (hPa)
  dataJson["RELH"] = sensorVal.RelH;

  // Pitot
  dataJson["PITOT"] = sensorVal.PitotH2O;

  // Differential pressure
  dataJson["PDIFF"] = sensorVal.PDiffH2O;

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
 * @brief bootLoop
 * @details Loop until files uploaded
 ***/
void DataHandler::bootLoop()
{

    extern struct DeviceStatus status;
    extern struct ConfigSettings config;
    extern struct SensorData sensorVal;

    Hardware _hardware;
    Calculations _calculations;
    Messages _message;
    Webserver _webserver;
    API _api;
    DataHandler _data;

    String jsonString;

    StaticJsonDocument<DATA_JSON_SIZE> dataJson;
    bool doLoop = true;
    bool shouldReboot = false;


    _message.serialPrintf("spin up temporary web server");

    tempServer = new AsyncWebServer(80);
    tempServerEvents = new AsyncEventSource("/events");

    // const TickType_t xDelay = 500 / portTICK_PERIOD_MS;

    // Index page request handler
    tempServer->on("/", HTTP_ANY, [](AsyncWebServerRequest *request){
        // extern struct DeviceStatus status;
        // if ((SPIFFS.exists("/index.html")) && (SPIFFS.exists("/pins.json"))) {
        //     request->send(SPIFFS, "/index.html", "text/html", false, processTemplate);
        // } else {
        //     // request->send_P(200, "text/html", LANDING_PAGE, processLandingPageTemplate); 
        // }
            request->send_P(200, "text/html", "HELLOEEEE"); 
        });

    // server->onFileUpload(processUpload);
    tempServer->addHandler(tempServerEvents);
    tempServer->begin();


    _message.serialPrintf("Doing bootloop...");

    do {
    // Display index.html and wait for files to be uploaded

        // Process API comms
        if (config.api_enabled) {        
            if (millis() > status.apiPollTimer) {
                if (Serial.available() > 0) {
                    status.serialData = Serial.read();
                    _api.ParseMessage(status.serialData);
                }
            }                            
        }

        if (millis() > status.browserUpdateTimer) {        
                status.browserUpdateTimer = millis() + STATUS_UPDATE_RATE; // Only reset timer when task execute
                // Push data to client using Server Side Events (SSE)
                jsonString = _data.getDataJSON();
                tempServerEvents->send(String(jsonString).c_str(),"JSON_DATA",millis()); // Is String causing message queue issue?

        }

        if (status.shouldReboot) {
            _message.serialPrintf("Rebooting...");
            delay(100);
            ESP.restart();
        }
        // vTaskDelay( xDelay );
        delay(50);
    
    }
    while (doLoop = true);

    // once files are uploaded ESP reboots ????
    tempServer->reset();

}