/****************************************
 * The DIY Flow Bench project
 * https://diyflowbench.com
 * 
 * settings.cpp - configuration settings management 
 *
 * Open source flow bench project to measure and display volumetric air flow using an ESP32 / Arduino.
 * 
 * For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions 
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 * 
 ***/
 
#include "settings.h"
#include "configuration.h"
#include "structs.h"
// #include "constants.h"
// #include "calibration.h"
#include <ArduinoJson.h>
#include "messages.h"
#include <SPIFFS.h>
#include LANGUAGE_FILE


Settings::Settings() {

}


/****************************************
 * Parse Config Data
 ***/
void Settings::parseConfigData(StaticJsonDocument<1024> configData) {

  extern struct ConfigSettings config;

  config.wifi_ssid = configData["CONF_WIFI_SSID"].as<String>();
  config.wifi_pswd = configData["CONF_WIFI_PSWD"].as<String>();
  config.wifi_ap_ssid = configData["CONF_WIFI_AP_SSID"].as<String>();
  config.wifi_ap_pswd = configData["CONF_WIFI_AP_PSWD"].as<String>();
  config.hostname = configData["CONF_HOSTNAME"].as<String>();
  config.wifi_timeout = configData["CONF_WIFI_TIMEOUT"].as<int>();
  config.refresh_rate = configData["CONF_REFRESH_RATE"].as<int>();
  config.min_bench_pressure  = configData["CONF_MIN_BENCH_PRESSURE"].as<int>();
  config.min_flow_rate = configData["CONF_MIN_FLOW_RATE"].as<int>();
  config.cyc_av_buffer  = configData["CONF_CYCLIC_AVERAGE_BUFFER"].as<int>();
  config.maf_min_millivolts  = configData["CONF_MAF_MIN_MILLIVOLTS"].as<int>();
  config.api_delim = configData["CONF_API_DELIM"].as<String>();
  config.serial_baud_rate = configData["CONF_SERIAL_BAUD_RATE"].as<long>();
//  config.show_alarms = configData["CONF_SHOW_ALARMS"].as<bool>();
  config.leak_test_tolerance = configData["CONF_LEAK_TEST_TOLERANCE"].as<int>();
  config.cal_ref_press = configData["CONF_CAL_REF_PRESS"].as<float>();
  config.cal_flow_rate = configData["CONF_CAL_FLOW_RATE"].as<float>();
//  config.cal_offset = configData["CONF_CAL_OFFSET"].as<float>();

}


/****************************************
 * Parse Calibration Data
 ***/
void Settings::parseCalibrationData(StaticJsonDocument<1024>  calibrationData) {

  extern struct CalibrationSettings calibrationSettings;  
  
  calibrationSettings.flow_offset = calibrationData["CAL_FLOW_OFFSET"].as<float>();

}


/****************************************
 * localConfig
 * read configuration data from config.json file
 ***/
 
StaticJsonDocument<1024> Settings::LoadConfig () {

  //extern ConfigSettings config;
  
  Messages _message;

  if (SPIFFS.exists("/config.json")){
    File configFile = SPIFFS.open("/config.json", FILE_READ);

    if (!configFile) {
        _message.Handler(LANG_ERROR_LOADING_CONFIG);        
        _message.DebugPrint("Failed to open config file for reading");
        configFile.close();
    } else {
        size_t size = configFile.size();
        if (size > 1024) {
          _message.DebugPrint("Config file size is too large");
          exit;
        }
        // Allocate the memory pool on the stack.
        // Use arduinojson.org/assistant to compute the capacity.
        StaticJsonDocument<1024> configData;
        // Parse the root object
        DeserializationError error = deserializeJson(configData, configFile);
        if (error) {
          _message.DebugPrint("deserializeJson() failed: "); 
          _message.DebugPrint(error.f_str());          
        }
        parseConfigData(configData);
        configFile.close();
        return configData;
        //or return a string...
        //String jsonString;        
        //serializeJson(configData, jsonString);
        //return jsonString;
            }    
    configFile.close();
  } else {
    _message.DebugPrint("config.json missing");
  }
}



/****************************************
 * loadCalibration
 * Read calibration data from calibration.json file
 ***/
String Settings::loadCalibration () {

  //extern CalibrationSettings calibration;
  Messages _message;

  if (SPIFFS.exists("/calibration.json")){
    File calibrationFile = SPIFFS.open("/calibration.json", FILE_READ);

    if (!calibrationFile) {
        _message.Handler(LANG_ERROR_LOADING_CONFIG);
        _message.DebugPrint("Failed to open config file for reading");
        calibrationFile.close();
    } else {
        size_t size = calibrationFile.size();
        if (size > 1024) {
          #ifdef DEBUG 
            _message.DebugPrint("Config file size is too large");
          #endif
          exit;
        }
        // Allocate the memory pool on the stack.
        // Use arduinojson.org/assistant to compute the capacity.
        StaticJsonDocument<1024> calibrationData;
        // Parse the root object
        DeserializationError error = deserializeJson(calibrationData, calibrationFile);
        if (error) {
          _message.DebugPrint("deserializeJson() failed: ");
          _message.DebugPrint(error.f_str());
        }
        parseConfigData(calibrationData);
        calibrationFile.close();
        String jsonString;
        serializeJson(calibrationData, jsonString);
        return jsonString;
    }    
    calibrationFile.close();
  } else {
    _message.DebugPrint("calibration.json missing");
  }
}



/****************************************
 * saveConfig
 * write configuration data to config.json file
 ***/
void Settings::saveConfig (char *data) {

  Messages _message;

  StaticJsonDocument<1024> configData;
  DeserializationError error = deserializeJson(configData, data);
  parseConfigData(configData);  
  // We don't want to store the header data in the file, so lets remove it
  configData.remove('HEADER');

  _message.Handler(LANG_SAVING_CONFIG);
  _message.DebugPrint((char*)data);

  File configFile = SPIFFS.open("/config.json", FILE_WRITE);
  serializeJsonPretty(configData, configFile);
  configFile.close();

}



/****************************************
 * saveCalibration 
 * write calibration data to calibration.json file
 ***/
void Settings::saveCalibration (char *data) {

  Messages _message;

  StaticJsonDocument<1024> calibrationData;
  DeserializationError error = deserializeJson(calibrationData, data);
  parseConfigData(calibrationData);

  _message.DebugPrint("Saving Calibration...");
  _message.DebugPrint((char*)data);  

  File calibrationFile = SPIFFS.open("/calibration.json", FILE_WRITE);
  serializeJsonPretty(calibrationData, calibrationFile);
  calibrationFile.close();

}


/****************************************
 * write JSON string to file
 ***/
void Settings::writeJSONFile(char *data, String filename) {

  Messages _message;

  StaticJsonDocument<1024> jsonData;
  DeserializationError error = deserializeJson(jsonData, data);
  parseConfigData(jsonData);

  _message.DebugPrint("Saving JSON file..."); 
  _message.DebugPrint((char*)data);  

  File outputFile = SPIFFS.open(filename, FILE_WRITE);
  serializeJsonPretty(jsonData, outputFile);
  outputFile.close();

}