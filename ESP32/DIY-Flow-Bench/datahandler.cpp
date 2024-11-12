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

#include "structs.h"
#include "constants.h"

#include "version.h"



void DataHandler::begin() {

    extern struct ConfigSettings config;
    // extern struct Translator translate; // TODO: stuct currently part of lang file - need to move into JSON
    extern struct DeviceStatus status;

    // Need to set up the data environment...

    // initialise SPIFFS Filesystem
    // _message.serialPrintf("File System Initialisation...\n"); // TODO:  Initialise messages once pins / serial is running
    if (SPIFFS.begin()) {
        // _message.serialPrintf("Complete.\n");
    } else {
        // _message.serialPrintf("Failed.\n");
        #if defined FORMAT_FILESYSTEM_IF_FAILED
            SPIFFS.format();
            // _message.serialPrintf("!! File System Formatted !!\n");
        #endif
    }

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


    // Check if config / calibration / liftdata json files exist. If not create them.
    // NOTE Combined filesize cannot exceed SPIFFS partition
    if (!SPIFFS.exists("/config.json")) createConfigFile();
    if (!SPIFFS.exists("/liftdata.json")) createLiftDataFile();
    if (!SPIFFS.exists("/cal.json")) createCalibrationFile();


    // Load pin definition file if exists
    if (!SPIFFS.exists("/pins.json")) {


        
    }




    // Initialise JSON



    // Read in and populate global vars from config files (settings / config / language / MAFdata / etc)

    // Manage on-boarding (index and config file uploading)


}




/***********************************************************
* @brief createConfig
* @details Create basic minimum configuration json file
* @note Called from DataHandler::begin() if config.json not found
***/
void DataHandler::createConfigFile () {

  extern struct ConfigSettings config;
//   Messages _message;
  String jsonString;  
  StaticJsonDocument<CONFIG_JSON_SIZE> configData;

//   _message.serialPrintf("Creating config.json file... \n"); 
 
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

//   Messages _message;

  // StaticJsonDocument<dataSize> jsonData;
  DynamicJsonDocument jsonData(dataSize);
  DeserializationError error = deserializeJson(jsonData, data);
  if (!error)  {
    // _message.debugPrintf("Writing JSON file... \n");
    File outputFile = SPIFFS.open(filename, FILE_WRITE);
    serializeJsonPretty(jsonData, outputFile);
    outputFile.close();
  }  else  {
    // _message.statusPrintf("Webserver::writeJSONFile ERROR \n");
  }
}




/***********************************************************
* @brief createLiftDataFile
* @details Create blank lift data json file
* @note Called from DataHandler::begin() if liftdata.json not found
***/
void DataHandler::createLiftDataFile () {

//   Messages _message;
  String jsonString;  
  StaticJsonDocument<LIFT_DATA_JSON_SIZE> liftData;

//   _message.serialPrintf("Creating liftdata.json file... \n"); 

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
//   Messages _message;
  String jsonString;
  StaticJsonDocument<CAL_DATA_JSON_SIZE> calData;
  
//   _message.debugPrintf("Creating cal.json file... \n"); 
  
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

//   Messages _message;

//   extern struct Translator translate;

  // Allocate the memory pool on the stack.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<CONFIG_JSON_SIZE> jsonData;

  if (SPIFFS.exists(filename))  {
    File jsonFile = SPIFFS.open(filename, FILE_READ);

    if (!jsonFile)    {
    //   _message.Handler(translate.LANG_ERROR_LOADING_FILE);
    //   _message.statusPrintf("Failed to open file for reading \n");
    }    else    {
      size_t size = jsonFile.size();
      if (size > CONFIG_JSON_SIZE)    {

      }

      DeserializationError error = deserializeJson(jsonData, jsonFile);
      if (error)      {
        // _message.statusPrintf("loadJSONFile->deserializeJson() failed: %s \n", error.f_str());
      }

      jsonFile.close();
      return jsonData;

    }
    jsonFile.close();
  }  else  {
    // _message.statusPrintf("File missing \n");
  }

  return jsonData;
}