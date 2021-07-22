/****************************************
 * The DIY Flow Bench project
 * https://diyflowbench.com
 * 
 * settings.ino - configuration settings management 
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


/****************************************
 * DECLARE CONSTANTS
 ***/



/****************************************
 * DECLARE VARIABLES
 ***/




/****************************************
 * read config.json file
 ***/
String loadConfig(){

  extern ConfigSettings config;


  if (SPIFFS.exists("/config.txt")){
    File configFile = SPIFFS.open("/config.txt", FILE_READ);

    if (!configFile) {
        statusMessage = LANG_ERROR_LOADING_CONFIG;
        Serial.println("Failed to open config file for reading");
        configFile.close();

    } else {

        size_t size = configFile.size();
        if (size > 1024) {
          Serial.println("Config file size is too large");
          exit;
        }

        // Allocate the memory pool on the stack.
        // Use arduinojson.org/assistant to compute the capacity.
        StaticJsonDocument<1024> configData;

        // Parse the root object
        DeserializationError error = deserializeJson(configData, configFile);

        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
//          return;
        }

        config.wifi_ssid = configData["CONF_WIFI_SSID"].as<String>();
        config.wifi_pswd = configData["CONF_WIFI_PSWD"].as<String>();
        config.wifi_ap_pswd = configData["CONF_WIFI_AP_PSWD"].as<String>();
        config.wifi_ap_pswd = configData["CONF_WIFI_AP_PSWD"].as<String>();
        config.hostname = configData["CONF_HOSTNAME"].as<String>();
        config.wifi_timeout = configData["CONF_WIFI_TIMEOUT"];
        config.refresh_rate = configData["CONF_REFRESH_RATE"];

        configFile.close();
        String jsonString;
        serializeJson(configData, jsonString);
        return jsonString;

     
    }    
    configFile.close();
  } else {
    Serial.println("config.txt missing");
  }
}



/****************************************
 * write config.json file
 ***/
void saveConfig(uint8_t *data, size_t len){

  StaticJsonDocument<1024> configData;

  // Parse the root object
  DeserializationError error = deserializeJson(configData, data);


  config.wifi_ssid = configData["CONF_WIFI_SSID"].as<String>();
  config.wifi_pswd = configData["CONF_WIFI_PSWD"].as<String>();
  config.wifi_ap_ssid = configData["CONF_WIFI_AP_SSID"].as<String>();
  config.wifi_ap_pswd = configData["CONF_WIFI_AP_PSWD"].as<String>();
  config.hostname = configData["CONF_HOSTNAME"].as<String>();
  config.wifi_timeout = configData["CONF_WIFI_TIMEOUT"].as<int>();
  config.refresh_rate = configData["CONF_REFRESH_RATE"].as<int>();




//  DynamicJsonDocument configJson(1024);

//  configJson["CONF_API_DELIM"] = String(config.api_delim);
//  configJson["CONF_SERIAL_BAUD_RATE"] = String(config.serial_baud_rate);
//  configJson["CONF_MIN_FLOW_RATE"] = String(config.min_flow_rate);
//  configJson["CONF_MIN_BENCH_PRESSURE"] = String(config.min_bench_pressure);
//  configJson["CONF_MAF_MIN_MILLIVOLTS"] = String(config.maf_min_millivolts);

  // "CONF_REFRESH_RATE": 200,
  // "CONF_DATALOG_RATE": 200,
  // "CONF_SHOW_ALARMS": "true",
  // "CONF_LEAK_TEST_TOLERANCE": 2,
  // "CONF_CAL_REF_PRESS": 10,
  // "CONF_CAL_FLOW_RATE": 14.4


//  char jsonBuffer[1024];
  File configFile = SPIFFS.open("/config.txt", FILE_WRITE);
  serializeJsonPretty(configData, configFile);
  // if (!configFile.println(data)){
  //   //do nothing
  // }
  configFile.close();

}


