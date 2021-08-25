/***********************************************************
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
#include "webserver.h"
#include <SPIFFS.h>
#include LANGUAGE_FILE


Settings::Settings() {

}


/***********************************************************
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
  config.leak_test_threshold = configData["CONF_LEAK_TEST_THRESHOLD"].as<int>();
  config.cal_ref_press = configData["CONF_CAL_REF_PRESS"].as<float>();
  config.cal_flow_rate = configData["CONF_CAL_FLOW_RATE"].as<float>();
//  config.cal_offset = configData["CONF_CAL_OFFSET"].as<float>();

}





/***********************************************************
* loadConfig
* read configuration data from config.json file
***/ 
StaticJsonDocument<1024> Settings::LoadConfig () {

  Webserver _webserver;
  Messages _message;
  _message.DebugPrintLn("Settings::LoadConfig"); 
  
  StaticJsonDocument<1024> configData;
  configData = _webserver.loadJSONFile("/config.json");
  parseConfigData(configData);
  return configData;
}





/***********************************************************
* createConfig
* 
* Create configuration json file
* Called from Webserver::Initialise() if config.json not found
***/
void Settings::createConfigFile () {

  extern struct ConfigSettings config;
  Webserver _webserver;
  Messages _message;
  String jsonString;  
  StaticJsonDocument<1024> configData;

  _message.DebugPrintLn("Creating config.json file..."); 
 
  configData["CONF_WIFI_SSID"] = config.wifi_ssid;
  configData["CONF_WIFI_PSWD"] = config.wifi_pswd;
  configData["CONF_WIFI_AP_SSID"] = config.wifi_ap_ssid;
  configData["CONF_WIFI_AP_PSWD"] = config.wifi_ap_pswd;
  configData["CONF_HOSTNAME"] = config.hostname;
  configData["CONF_WIFI_TIMEOUT"] = config.wifi_timeout;
  configData["CONF_REFRESH_RATE"] = config.refresh_rate;
  configData["CONF_MIN_BENCH_PRESSURE"] = config.min_bench_pressure;
  configData["CONF_MIN_FLOW_RATE"] = config.min_flow_rate;
  configData["CONF_CYCLIC_AVERAGE_BUFFER"] = config.cyc_av_buffer;
  configData["CONF_MAF_MIN_MILLIVOLTS"] = config.maf_min_millivolts;
  configData["CONF_API_DELIM"] = config.api_delim;
  configData["CONF_SERIAL_BAUD_RATE"] = config.serial_baud_rate;
  configData["CONF_LEAK_TEST_TOLERANCE"] = config.leak_test_tolerance;
  configData["CONF_LEAK_TEST_THRESHOLD"] = config.leak_test_threshold;
  configData["CONF_CAL_REF_PRESS"] = config.cal_ref_press;
  configData["CONF_CAL_FLOW_RATE"] = config.cal_flow_rate;
  
  serializeJsonPretty(configData, jsonString);
  _webserver.writeJSONFile(jsonString, "/config.json");
  

}


/***********************************************************
* saveConfig
* write configuration data to config.json file
***/
void Settings::saveConfig (StaticJsonDocument<1024> configData) {

  Messages _message;
  Webserver _webserver;
  
  String jsonString;
  
  parseConfigData(configData);  
  
  // We don't want to store the header data in the file, so lets remove it
  configData.remove("HEADER");
  
  _message.Handler(LANG_SAVING_CONFIG);
  _message.DebugPrintLn("Configuration Data:");
  serializeJson(configData, Serial);
  _message.DebugPrintLn("Saved to /config.json");
  
  serializeJsonPretty(configData, jsonString);
  _webserver.writeJSONFile(jsonString, "/config.json");
  
  // Clear down the status Message
  _message.Handler(LANG_NO_ERROR);

}
