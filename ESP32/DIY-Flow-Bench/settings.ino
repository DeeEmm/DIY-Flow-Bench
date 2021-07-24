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
 * read config.json file
 ***/
String loadConfig(){

  extern ConfigSettings config;


  if (SPIFFS.exists("/config.json")){
    File configFile = SPIFFS.open("/config.json", FILE_READ);

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
    Serial.println("config.json missing");
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

  config.min_bench_pressure  = configData["CONF_MIN_BENCH_PRESSURE"].as<int>();
  config.min_flow_rate = configData["CONF_MIN_FLOW_RATE"].as<int>();
  config.cyc_av_buffer  = configData["CONF_CYCLIC_AVERAGE_BUFFER"].as<int>();
  config.maf_min_millivolts  = configData["CONF_MIN_MAF_MILLIVOLTS"].as<int>();
  config.api_delim = configData["CONF_API_DELIM"].as<String>();
  config.serial_baud_rate = configData["CONF_SERIAL_BAUD_RATE"].as<long>();
  config.show_alarms = configData["CONF_SHOW_ALARMS"].as<bool>();
  config.leak_test_tolerance = configData["CONF_LEAK_TEST_TOLERANCE"].as<int>();
  config.cal_ref_press = configData["CONF_CAL_REF_PRESS"].as<float>();
  config.cal_flow_rate = configData["CONF_CAL_FLOW_RATE"].as<float>();
  config.cal_offset = configData["CONF_CAL_OFFSET"].as<float>();


//  DynamicJsonDocument configJson(1024);


  // "CONF_DATALOG_RATE": 200,




//  char jsonBuffer[1024];
  File configFile = SPIFFS.open("/config.json", FILE_WRITE);
  serializeJsonPretty(configData, configFile);
  // if (!configFile.println(data)){
  //   Serial.println("config.json file wwrite failed");
  // }
  configFile.close();

}


