/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file webserver.cpp
 * 
 * @brief Webserver class
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
#include <rom/rtc.h>
#include <SPI.h>
#include <SD.h>

#include "configuration.h"
#include "constants.h"
#include "structs.h"

#include <WiFi.h>
#include <ESPmDNS.h>
#include <esp_wifi.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "webserver.h"

#include "calibration.h"
#include "sensors.h"
#include "pins.h"
#include "hardware.h"
#include "messages.h"
#include "calculations.h"
#include LANGUAGE_FILE

// RTC_DATA_ATTR int bootCount; // flash mem

/***********************************************************
 * @brief INITIALISE SERVER
 * @note WiFi Error Codes
 * Value	Constant	Meaning
 * 0	WL_IDLE_STATUS	temporary status assigned when WiFi.begin() is called
 * 1	WL_NO_SSID_AVAIL	 when no SSID are available
 * 2	WL_SCAN_COMPLETED	scan networks is completed
 * 3	WL_CONNECTED	when connected to a WiFi network
 * 4	WL_CONNECT_FAILED	when the connection fails for all the attempts
 * 5	WL_CONNECTION_LOST	when the connection is lost
 * 6	WL_DISCONNECTED	when disconnected from a network
 * 
 ***/
void Webserver::begin()
{

  extern struct ConfigSettings config;
  extern struct Translator translate;
  extern DeviceStatus status;

  int wifiStatusCode;

  server = new AsyncWebServer(80);
  events = new AsyncEventSource("/events");

  Messages _message;
  Calibration _calibration;

  // Filesystem
  _message.serialPrintf("File System Initialisation...\n");
  if (SPIFFS.begin()) {
    _message.serialPrintf("Complete.\n");
  } else {
    _message.serialPrintf("Failed.\n");
    #if defined FORMAT_FILESYSTEM_IF_FAILED
        SPIFFS.format();
        _message.serialPrintf("!! File System Formatted !!\n");
    #endif
  }

  // Check if config and calibration json files exist. If not create them.
  if (!SPIFFS.exists("/config.json"))  {
    createConfigFile();
  }
  if (!SPIFFS.exists("/cal.json")) {
    _calibration.createCalibrationFile();
  }

  this->loadConfig();
  _calibration.loadCalibrationData();

  // Display Filesystem Stats
  status.spiffs_mem_size = SPIFFS.totalBytes();
  status.spiffs_mem_used = SPIFFS.usedBytes();

  _message.serialPrintf("===== File system info ===== \n");
  _message.serialPrintf("Total space:      %s \n", byteDecode(status.spiffs_mem_size));
  _message.serialPrintf("Total space used: %s \n", byteDecode(status.spiffs_mem_used));

  // SD Card
  #ifdef SD_CARD_IS_ENABLED
    _message.serialPrintf("Initialising SD Card... \n");

      if (!SD.begin(SD_CS)) {
        _message.serialPrintf("SD Card initialisation failed \n");
        while (true);
      } else {
        _message.serialPrintf("Complete.\n");
      }
  #endif

  // WiFi
  // if WiFi password is unedited or blank force AP mode
  if ((strstr(String(config.wifi_pswd).c_str(), String("PASSWORD").c_str())) || (String(config.wifi_pswd).c_str() == "")) {
    config.ap_mode = true;
  } 
  
  // Connect to WiFi
  if (config.ap_mode == false)  {
    // WiFi.useStaticBuffers(true);   
    this->resetWifi();
    WiFi.mode(WIFI_STA);
    // Set MAC address
    #ifdef MAC_ADDRESS
      uint8_t newMACAddress[] = MAC_ADDRESS;
      esp_wifi_set_mac(WIFI_IF_STA, &newMACAddress[0]);
    #endif
    // Display MAC Address
    _message.serialPrintf("WiFi MAC Address: %s \n", String(WiFi.macAddress()).c_str());  
    _message.serialPrintf("Connecting to WiFi \n");
    #ifdef STATIC_IP
      // Configures static IP address
      if (!WiFi.config(STATIC_IP, GATEWAY, SUBNET)) {
        Serial.println("STA Failed to configure");
      }
    #endif
    wifiStatusCode = this->getWifiConnection();
  }

  // Test for connection success else create an accesspoint
  if (wifiStatusCode == 3 && config.ap_mode == false) { 
    // STA Connection success
    _message.serialPrintf("\nConnected to %s \n", config.wifi_ssid);
    status.local_ip_address = WiFi.localIP().toString().c_str();
    _message.serialPrintf("IP address: %s \n", WiFi.localIP().toString().c_str());
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    
  }  else  { // Go into AP Mode
    if (config.ap_mode == true) { // AP mode is Default
      _message.serialPrintf("\nDefaulting to AP Mode \n");
    } else { // AP mode is Fallback
      _message.serialPrintf("\nFailed to connect to Wifi \n");
      _message.serialPrintf("Wifi Status message: ");
      _message.serialPrintf(String(wifiStatusCode).c_str());
      _message.serialPrintf("\n");
    }

    _message.serialPrintf("Creating WiFi Access Point:  %s  \n", config.wifi_ap_ssid); // NOTE: Default AP SSID / PW = DIYFB / 123456789
    WiFi.mode(WIFI_AP);
    WiFi.softAP(config.wifi_ap_ssid, config.wifi_ap_pswd);
    _message.serialPrintf("Access Point IP address: %s \n", WiFi.softAPIP().toString().c_str());
    status.apMode = true;
  }

  // Set up Multicast DNS
  if (!MDNS.begin(config.hostname))  {
    _message.serialPrintf("Error starting mDNS \n");
  }  else  {
    status.hostname = config.hostname;
    _message.serialPrintf("Multicast: http://%s.local \n", status.hostname);
  }

  // API request handlers [JSON confirmation response]
  server->on("/api/bench/on", HTTP_GET, [](AsyncWebServerRequest *request){
      Messages _message;
      Hardware _hardware;
      _message.Handler(translate.LANG_VAL_BENCH_RUNNING);
      _message.debugPrintf("Bench On \n");
      _hardware.benchOn(); 
      // request->send(200, "text/html", "{\"bench\":\"on\"}"); 
      });

  server->on("/api/bench/off", HTTP_GET, [](AsyncWebServerRequest *request){
      Messages _message;
      Hardware _hardware;
      _message.Handler(translate.LANG_VAL_BENCH_STOPPED);
      _message.debugPrintf("Bench Off \n");
      _hardware.benchOff(); 
      // request->send(200, "text/html", "{\"bench\":\"off\"}"); 
      });

  server->on("/api/debug/on", HTTP_GET, [](AsyncWebServerRequest *request){
      Messages _message;
      _message.Handler(translate.LANG_VAL_DEBUG_MODE);
      _message.debugPrintf("Debug Mode On\n");
      config.debug_mode = true;
      request->send(200, "text/html", "{\"debug\":\"on\"}"); });

  server->on("/api/debug/off", HTTP_GET, [](AsyncWebServerRequest *request){
      Messages _message;
      _message.Handler(translate.LANG_VAL_BLANK);
      _message.debugPrintf("Debug Mode Off\n");
      config.debug_mode = false;
      request->send(200, "text/html", "{\"debug\":\"off\"}"); });

  server->on("/api/dev/on", HTTP_GET, [](AsyncWebServerRequest *request) {
      Messages _message;
      _message.Handler(translate.LANG_VAL_DEV_MODE);
      _message.debugPrintf("Developer Mode On\n");
      config.dev_mode = true;
      request->send(200, "text/html", "{\"dev\":\"on\"}"); });

  server->on("/api/dev/off", HTTP_GET, [](AsyncWebServerRequest *request){
      Messages _message;
      _message.Handler(translate.LANG_VAL_BLANK);
      _message.debugPrintf("Developer Mode Off\n");
      config.dev_mode = false;
      request->send(200, "text/html", "{\"dev\":\"off\"}"); });

  server->on("/api/clear-message", HTTP_GET, [](AsyncWebServerRequest *request) {
      Messages _message;
      status.statusMessage = "";
      _message.Handler(translate.LANG_VAL_NO_ERROR);
      _message.debugPrintf("Clearing messages...\n");
       });

  server->on("/api/bench/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
      Messages _message;
      _message.Handler(translate.LANG_VAL_SYSTEM_REBOOTING);
      request->send(200, "text/html", "{\"reboot\":\"true\"}");
      ESP.restart(); 
      request->redirect("/"); });

  server->on("/api/bench/calibrate", HTTP_GET, [](AsyncWebServerRequest *request){
      Messages _message;
      Calibration _calibrate;
      Hardware _hardware;
      if (_hardware.benchIsRunning()) {
        _message.Handler(translate.LANG_VAL_CALIBRATING);
        _message.debugPrintf("Calibrating Flow...\n");
        request->send(200, "text/html", "{\"calibrate\":\"true\"}");
        _calibrate.setFlowOffset();         
      } else {
        _message.Handler(translate.LANG_VAL_RUN_BENCH_TO_CALIBRATE);
        request->send(200, "text/html", "{\"calibrate\":\"false\"}");
      }  
      request->redirect("/");});

  server->on("/api/bench/leakcal", HTTP_GET, [](AsyncWebServerRequest *request){
      Messages _message;
      Calibration _calibrate;
      Hardware _hardware;
      if (_hardware.benchIsRunning()) {
        _message.Handler(translate.LANG_VAL_LEAK_CALIBRATING);
        _message.debugPrintf("Calibrating Leak Test...\n");
        request->send(200, "text/html", "{\"leakcal\":\"true\"}");
        _calibrate.setLeakTest();
      } else {
        _message.Handler(translate.LANG_VAL_RUN_BENCH_TO_CALIBRATE);
        request->send(200, "text/html", "{\"leakcal\":\"false\"}");
      } 
      request->redirect("/"); });

  // Upload request handler
  server->on("/api/file/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
      Messages _message;
      _message.serialPrintf("Upload Request Called \n");
      request->redirect("/?view=upload"); },
      processUpload);

  // Download request handler
  server->on("/api/file/download", HTTP_GET, [](AsyncWebServerRequest *request){              
      Messages _message;
      String downloadFilename = request->url();
      downloadFilename.remove(0,18); // Strip the file path (first 18 chars)
      _message.debugPrintf("Request Download File: %s \n", downloadFilename);
      request->send(SPIFFS, downloadFilename, String(), true); });

  // Delete request handler
  server->on("/api/file/delete", HTTP_POST, [](AsyncWebServerRequest *request){              
      Messages _message;
      String fileToDelete;
      AsyncWebParameter *p = request->getParam("filename", true);
      fileToDelete = p->value();      
      // Don't delete index.html (you can overwrite it!!)
      // if (fileToDelete != "/index.html"){
        if(SPIFFS.exists(fileToDelete)){
          SPIFFS.remove(fileToDelete);
        }  else {
          _message.debugPrintf("Delete Failed: %s", fileToDelete);  
          _message.Handler(translate.LANG_VAL_DELETE_FAILED);    
        } 
        if (fileToDelete == "/index.html"){
          request->redirect("/");
        } else {
          request->redirect("/?view=upload");
        }
      
       });

  // Save Configuration Form
  server->on("/api/saveconfig", HTTP_POST, saveConfig);

  // Save Orifice Form
  server->on("/api/setorifice", HTTP_POST, setOrifice);

  server->rewrite("/index.html", "/");

  // Style sheet request handler
  server->on("/style.css", HTTP_ANY, [](AsyncWebServerRequest *request){ request->send(SPIFFS, "/style.css", "text/css"); });

  // Javascript file request handler
  server->on("/javascript.js", HTTP_ANY, [](AsyncWebServerRequest *request){ request->send(SPIFFS, "/javascript.js", "text/javascript"); });

  // Favicon rquest handler (icon hex dump is in constants.h)
  server->on("/favicon.ico", HTTP_ANY, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "image/x-icon", favicon_ico_gz, favicon_ico_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  
  // Index page request handler
  server->on("/", HTTP_ANY, [](AsyncWebServerRequest *request){
      if (SPIFFS.exists("/index.html")) {
        request->send(SPIFFS, "/index.html", "text/html", false, processTemplate);
       } else {
        request->send(200, "text/html", LANG_VAL_INDEX_HTML); 
      }});

  server->onFileUpload(processUpload);
  server->addHandler(events);
  server->begin();

  _message.Handler(translate.LANG_VAL_SERVER_RUNNING);
  _message.serialPrintf("Server Running \n");
}


/***********************************************************
 * @brief byteDecode
 * @param bytes size to be decoded
 * @details Byte Decode (returns string i.e '52 GB')
 ***/
String Webserver::byteDecode(size_t bytes)
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
 * @brief Process File Upload
 * @note Redirects browser back to Upload modal unless upload is index file
 ***/
void Webserver::processUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{

  Messages _message;
  extern struct FileUploadData fileUploadData;
  String redirectURL;

  if (SPIFFS.exists(filename))  {
    SPIFFS.remove(filename);
  }
  uint32_t freespace = SPIFFS.totalBytes() - SPIFFS.usedBytes();

  if (!filename.startsWith("/"))
    filename = "/" + filename;
  if (!index && !fileUploadData.upload_error)  {
    _message.debugPrintf("UploadStart: %s \n", filename);
    request->_tempFile = SPIFFS.open(filename, "w");
  }

  // Set redirect to file Upload modal unless uploading the index file
  if (filename == String("/index.html.gz") || filename == String("/index.html"))  {
    redirectURL = "/";
  }  else  {
    redirectURL = "/?view=upload";
  }

  if (len)  {
    fileUploadData.file_size += len;
    if (fileUploadData.file_size > freespace)    {
      // TODO: _message.statusPrintf("Upload rejected, not enough space \n");
      fileUploadData.upload_error = true;
    }    else    {
      // TODO: _message.statusPrintf("Writing file: '%s' index=%s len=%s \n", filename, index, len);
      request->_tempFile.write(data, len);
    }
  } 

  if (final)  {
    _message.statusPrintf("UploadEnd: %s,%s \n", filename, fileUploadData.file_size);
    request->_tempFile.close();
    request->redirect(redirectURL);
  }
}




/***********************************************************
* @brief Parse Config Settings
* @param configData JSON document containing config data
***/
void Webserver::parseConfigSettings(StaticJsonDocument<CONFIG_JSON_SIZE> configData) {

  extern struct ConfigSettings config;

  strcpy(config.wifi_ssid, configData["CONF_WIFI_SSID"]);
  strcpy(config.wifi_pswd, configData["CONF_WIFI_PSWD"]);
  strcpy(config.wifi_ap_ssid, configData["CONF_WIFI_AP_SSID"]);
  strcpy(config.wifi_ap_pswd,configData["CONF_WIFI_AP_PSWD"]);
  strcpy(config.hostname, configData["CONF_HOSTNAME"]);
  config.wifi_timeout = configData["CONF_WIFI_TIMEOUT"].as<int>();
  config.refresh_rate = configData["CONF_REFRESH_RATE"].as<int>();
  config.min_bench_pressure  = configData["CONF_MIN_BENCH_PRESSURE"].as<int>();
  config.min_flow_rate = configData["CONF_MIN_FLOW_RATE"].as<int>();
  config.cyc_av_buffer  = configData["CONF_CYCLIC_AVERAGE_BUFFER"].as<int>();
  config.maf_min_volts  = configData["CONF_MAF_MIN_VOLTS"].as<int>();
  strcpy(config.api_delim, configData["CONF_API_DELIM"]);
  config.serial_baud_rate = configData["CONF_SERIAL_BAUD_RATE"].as<long>();
  config.show_alarms = configData["CONF_SHOW_ALARMS"].as<bool>();
  configData["ADJ_FLOW_DEPRESSION"] = config.adj_flow_depression;
  configData["TEMP_UNIT"] = config.temp_unit;
  configData["VALVE_LIFT_INTERVAL"] = config.valveLiftInterval;

  config.cal_flow_rate = configData["CONF_CAL_FLOW_RATE"].as<double>();
  config.cal_ref_press = configData["CONF_CAL_REF_PRESS"].as<double>();
  config.leak_test_tolerance = configData["CONF_LEAK_TEST_TOLERANCE"].as<int>();
  config.leak_test_threshold = configData["CONF_LEAK_TEST_THRESHOLD"].as<int>();

  config.OrificeOneFlow = configData["ORIFICE1_FLOW_RATE"].as<double>();
  config.OrificeOneDepression = configData["ORIFICE1_TEST_PRESSURE"].as<double>();
  config.OrificeTwoFlow = configData["ORIFICE2_FLOW_RATE"].as<double>();
  config.OrificeTwoDepression = configData["ORIFICE2_TEST_PRESSURE"].as<double>();
  config.OrificeThreeFlow = configData["ORIFICE3_FLOW_RATE"].as<double>();
  config.OrificeThreeDepression = configData["ORIFICE3_TEST_PRESSURE"].as<double>();
  config.OrificeFourFlow = configData["ORIFICE4_FLOW_RATE"].as<double>();
  config.OrificeFourDepression = configData["ORIFICE4_TEST_PRESSURE"].as<double>();
  config.OrificeFiveFlow = configData["ORIFICE5_FLOW_RATE"].as<double>();
  config.OrificeFiveDepression = configData["ORIFICE5_TEST_PRESSURE"].as<double>();
  config.OrificeSixFlow = configData["ORIFICE6_FLOW_RATE"].as<double>();
  config.OrificeSixDepression = configData["ORIFICE6_TEST_PRESSURE"].as<double>();

}



/***********************************************************
* @brief loadConfig
* @details read configuration data from config.json file
***/ 
StaticJsonDocument<CONFIG_JSON_SIZE> Webserver::loadConfig () {

  Messages _message;
  
  _message.serialPrintf("Loading Configuration... \n");     
  StaticJsonDocument<CONFIG_JSON_SIZE> configData;
  configData = loadJSONFile("/config.json");
  parseConfigSettings(configData);
  
  return configData;  

}





/***********************************************************
* @brief createConfig
* @details Create basic minimum configuration json file
* @note Called from Webserver::Initialise() if config.json not found
***/
void Webserver::createConfigFile () {

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
  configData["CONF_REFRESH_RATE"] = config.refresh_rate;
  configData["CONF_MIN_BENCH_PRESSURE"] = config.min_bench_pressure;
  configData["CONF_MIN_FLOW_RATE"] = config.min_flow_rate;
  configData["CONF_CYCLIC_AVERAGE_BUFFER"] = config.cyc_av_buffer;
  configData["CONF_MAF_MIN_VOLTS"] = config.maf_min_volts;
  configData["CONF_API_DELIM"] = config.api_delim;
  configData["CONF_SERIAL_BAUD_RATE"] = config.serial_baud_rate;
  configData["ADJ_FLOW_DEPRESSION"] = config.adj_flow_depression;
  configData["TEMP_UNIT"] = config.temp_unit;
  configData["CONF_SHOW_ALARMS"] = config.show_alarms;

  configData["CONF_CAL_FLOW_RATE"] = config.cal_flow_rate;
  configData["CONF_CAL_REF_PRESS"] = config.cal_ref_press;
  configData["CONF_LEAK_TEST_TOLERANCE"] = config.leak_test_tolerance;
  configData["CONF_LEAK_TEST_THRESHOLD"] = config.leak_test_threshold;

  configData["ORIFICE1_FLOW_RATE"] = config.OrificeOneFlow;
  configData["ORIFICE1_TEST_PRESSURE"] = config.OrificeOneDepression;
  configData["ORIFICE2_FLOW_RATE"] = config.OrificeTwoFlow;
  configData["ORIFICE2_TEST_PRESSURE"] = config.OrificeThreeDepression;
  configData["ORIFICE3_FLOW_RATE"] = config.OrificeThreeFlow;
  configData["ORIFICE4_FLOW_RATE"] = config.OrificeFourFlow;
  configData["ORIFICE4_TEST_PRESSURE"] = config.OrificeFourDepression;
  configData["ORIFICE5_FLOW_RATE"] = config.OrificeFiveFlow;
  configData["ORIFICE5_TEST_PRESSURE"] = config.OrificeFiveDepression;
  configData["ORIFICE6_FLOW_RATE"] = config.OrificeSixFlow;
  configData["ORIFICE7_TEST_PRESSURE"] = config.OrificeSixDepression;

  serializeJsonPretty(configData, jsonString);
  writeJSONFile(jsonString, "/config.json");

}



/***********************************************************
 * @brief saveConfig
 * @details Saves configuration data to config.json file
 * @note Creates file if it does not exist
 * @note Redirects browser to file list
 * 
 ***/
void Webserver::saveConfig(AsyncWebServerRequest *request)
{

  Messages _message;
  Webserver _webserver;

  StaticJsonDocument<CONFIG_JSON_SIZE> configData;
  extern struct ConfigSettings config;
  String jsonString;
  
  int params = request->params();

  _message.debugPrintf("Saving Configuration... \n");

  // Convert POST vars to JSON 
  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
      configData[p->name().c_str()] = p->value().c_str();
  }

  // Update Config Vars
  strcpy(config.wifi_ssid, configData["CONF_WIFI_SSID"]);
  strcpy(config.wifi_pswd, configData["CONF_WIFI_PSWD"]);
  strcpy(config.wifi_ap_ssid, configData["CONF_WIFI_AP_SSID"]);
  strcpy(config.wifi_ap_pswd,configData["CONF_WIFI_AP_PSWD"]);
  strcpy(config.hostname, configData["CONF_HOSTNAME"]);
  config.wifi_timeout = configData["CONF_WIFI_TIMEOUT"].as<int>();
  config.refresh_rate = configData["CONF_REFRESH_RATE"].as<int>();
  config.min_bench_pressure  = configData["CONF_MIN_BENCH_PRESSURE"].as<int>();
  config.min_flow_rate = configData["CONF_MIN_FLOW_RATE"].as<int>();
  config.cyc_av_buffer  = configData["CONF_CYCLIC_AVERAGE_BUFFER"].as<int>();
  config.maf_min_volts  = configData["CONF_MAF_MIN_VOLTS"].as<int>();
  strcpy(config.api_delim, configData["CONF_API_DELIM"]);
  config.serial_baud_rate = configData["CONF_SERIAL_BAUD_RATE"].as<long>();
  config.show_alarms = configData["CONF_SHOW_ALARMS"].as<bool>();
  config.adj_flow_depression = configData["ADJ_FLOW_DEPRESSION"].as<int>();
  strcpy(config.temp_unit, configData["TEMP_UNIT"]);
  config.valveLiftInterval = configData["VALVE_LIFT_INTERVAL"].as<double>();

  config.cal_flow_rate = configData["CONF_CAL_FLOW_RATE"].as<double>();
  config.cal_ref_press = configData["CONF_CAL_REF_PRESS"].as<double>();
  config.leak_test_tolerance = configData["CONF_LEAK_TEST_TOLERANCE"].as<int>();
  config.leak_test_threshold = configData["CONF_LEAK_TEST_THRESHOLD"].as<int>();

  config.OrificeOneFlow = configData["ORIFICE1_FLOW_RATE"].as<double>();
  config.OrificeOneDepression = configData["ORIFICE1_TEST_PRESSURE"].as<double>();
  config.OrificeTwoFlow = configData["ORIFICE2_FLOW_RATE"].as<double>();
  config.OrificeTwoDepression = configData["ORIFICE2_TEST_PRESSURE"].as<double>();
  config.OrificeThreeFlow = configData["ORIFICE3_FLOW_RATE"].as<double>();
  config.OrificeThreeDepression = configData["ORIFICE3_TEST_PRESSURE"].as<double>();
  config.OrificeFourFlow = configData["ORIFICE4_FLOW_RATE"].as<double>();
  config.OrificeFourDepression = configData["ORIFICE4_TEST_PRESSURE"].as<double>();
  config.OrificeFiveFlow = configData["ORIFICE5_FLOW_RATE"].as<double>();
  config.OrificeFiveDepression = configData["ORIFICE5_TEST_PRESSURE"].as<double>();
  config.OrificeSixFlow = configData["ORIFICE6_FLOW_RATE"].as<double>();
  config.OrificeSixDepression = configData["ORIFICE6_TEST_PRESSURE"].as<double>();

  // save settings to config file
  serializeJsonPretty(configData, jsonString);
  if (SPIFFS.exists("/config.json"))  {
    SPIFFS.remove("/config.json");
  }
  _webserver.writeJSONFile(jsonString, "/config.json");

  _message.debugPrintf("Configuration Saved \n");

  request->redirect("/?view=config");

}


/***********************************************************
 * @brief setOrifice
 * @details Sets selected orifice and loads orifice data
 * @todo //TODO auto orifice decode
 ***/
void Webserver::setOrifice(AsyncWebServerRequest *request)
{

  Messages _message;
  Webserver _webserver;

  int params = request->params();

  // Convert POST vars to JSON 
  for(int i=0;i<params;i++){

    // Test to see which radio is selected
    
    AsyncWebParameter* p = request->getParam(i);
      // configData[p->name().c_str()] = p->value().c_str();

    // Then load orifice data into memory 
  }

}






/***********************************************************
 * @brief getFileListJSON
 * @details Get SPIFFS File List in JSON format
 ***/
String Webserver::getFileListJSON()
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
String Webserver::getDataJSON()
{

  extern struct DeviceStatus status;
  extern struct ConfigSettings config;
  extern struct SensorData sensorVal;

  Hardware _hardware;
  Calculations _calculations;

  String jsonString;

  StaticJsonDocument<1500> dataJson;

  // Reference pressure
  dataJson["PREF"] = sensorVal.PRefH2O;

  // Flow Rate
  if (sensorVal.FlowCFM > config.min_flow_rate)  {
    dataJson["FLOW"] = sensorVal.FlowCFM;
  }  else  {
    dataJson["FLOW"] = 0;
  }

  // Adjusted Flow Rate
  dataJson["AFLOW"] = sensorVal.FlowADJ;
  dataJson["PADJUST"] = config.adj_flow_depression;

  // Temperature deg C or F
  if (strstr(String(config.temp_unit).c_str(), String("Celcius").c_str())){
    dataJson["TEMP"] = sensorVal.TempDegC;
  } else {
    dataJson["TEMP"] = sensorVal.TempDegF;
  }
  dataJson["BARO"] = sensorVal.BaroKPA;
  dataJson["RELH"] = sensorVal.RelH;

  // Pitot
  dataJson["PITOT"] = sensorVal.PitotKPA;

  // Differential pressure
  dataJson["PDIFF"] = sensorVal.PDiffKPA;

  // Swirl (+/- rpm)
  dataJson["SWIRL"] = sensorVal.Swirl;

  if (1!=1) {  // TODO if message handler is active display the active message
    dataJson["STATUS_MESSAGE"] = status.statusMessage;
  } else { // else lets just show the uptime
    dataJson["STATUS_MESSAGE"] = "Uptime: " + String(_hardware.uptime()) + " (hh.mm)";      
  }

  serializeJson(dataJson, jsonString);

  return jsonString;
}



/***********************************************************
 * @brief writeJSONFile
 * @details write JSON string to file
 ***/
void Webserver::writeJSONFile(String data, String filename)
{

  Messages _message;

  StaticJsonDocument<CONFIG_JSON_SIZE> jsonData;
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
StaticJsonDocument<CONFIG_JSON_SIZE> Webserver::loadJSONFile(String filename)
{

  Messages _message;

  extern struct Translator translate;

  // Allocate the memory pool on the stack.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<CONFIG_JSON_SIZE> jsonData;

  if (SPIFFS.exists(filename))  {
    File jsonFile = SPIFFS.open(filename, FILE_READ);

    if (!jsonFile)    {
      _message.Handler(translate.LANG_VAL_ERROR_LOADING_FILE);
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
 * @brief processTemplate
 * @details Replaces template placeholders with variable values
 * @param &var HTML payload 
 * @note %PLACEHOLDER_FORMAT%
 * @note using IF statements for this sucks but C++ switch statement cannot handle text operators
 ***/
String Webserver::processTemplate(const String &var)
{

  extern struct DeviceStatus status;
  extern struct ConfigSettings config;
  extern struct CalibrationData calVal;

  // Config Info
  if (var == "RELEASE") return RELEASE;
  if (var == "BUILD_NUMBER") return BUILD_NUMBER;
  if (var == "SPIFFS_MEM_SIZE") return String(status.spiffs_mem_size);
  if (var == "SPIFFS_MEM_USED") return String(status.spiffs_mem_used);
  if (var == "LOCAL_IP_ADDRESS") return String(status.local_ip_address);
  if (var == "HOSTNAME") return String(status.hostname);
  if (var == "UPTIME") return String(esp_timer_get_time()/1000);
  if (var == "BENCHTYPE") return String(status.benchType);
  if (var == "BOARDTYPE") return String(status.boardType);
  if (var == "BOOT_TIME") return String(status.boot_time);

  // Sensor Values
  if (var == "MAF_SENSOR") return String(status.mafSensor);
  if (var == "PREF_SENSOR") return String(status.prefSensor);
  if (var == "TEMP_SENSOR") return String(status.tempSensor);
  if (var == "RELH_SENSOR") return String(status.relhSensor);
  if (var == "BARO_SENSOR") return String(status.baroSensor);
  if (var == "PITOT_SENSOR") return String(status.pitotSensor);
  if (var == "PDIFF_SENSOR") return String(status.pdiffSensor);
  if (var == "STATUS_MESSAGE") return String(status.statusMessage);

  // Orifice Selector
  #ifdef ORIFICE_STYLE_BENCH
    String orificeList;
    if (var == "ORIFICE_LIST"){ 
      orificeList =  R"END(
      <form method="POST" action="/api/setorifice">
        <div class="switch-field">
          <input type="radio" id="orifice-one" name="switch-one" value="1" checked/>
          <label for="orifice-one">1</label>
          <input type="radio" id="orifice-two" name="switch-one" value="2" />
          <label for="orifice-two">2</label>
          <input type="radio" id="orifice-three" name="switch-one" value="3"/>
          <label for="orifice-three">3</label>
          <input type="radio" id="orifice-four" name="switch-one" value="4" />
          <label for="orifice-four">4</label>
          <input type="radio" id="orifice-five" name="switch-one" value="5" />
          <label for="orifice-five">5</label>
          <input type="radio" id="orifice-six" name="switch-one" value="6" />
          <label for="orifice-six">6</label>
        </div>
        <div id="orifice-data">Orifice #%SELECTED_ORIFICE% Flow Max: %ORIFICE_MAX_FLOW%cfm @ %ORIFICE_CALIBRATED_DEPRESSION%"</div>
      </form>
      )END" ;

      return orificeList;
    }
  #endif

  // Orifice plates
  if (var == "ORIFICE1_FLOW_RATE") return String(config.OrificeOneFlow);
  if (var == "ORIFICE1_TEST_PRESSURE") return String(config.OrificeOneDepression);
  if (var == "ORIFICE2_FLOW_RATE") return String(config.OrificeTwoFlow);
  if (var == "ORIFICE2_TEST_PRESSURE") return String(config.OrificeTwoDepression);
  if (var == "ORIFICE3_FLOW_RATE") return String(config.OrificeThreeFlow);
  if (var == "ORIFICE3_TEST_PRESSURE") return String(config.OrificeThreeDepression);
  if (var == "ORIFICE4_FLOW_RATE") return String(config.OrificeFourFlow);
  if (var == "ORIFICE4_TEST_PRESSURE") return String(config.OrificeFourDepression);
  if (var == "ORIFICE5_FLOW_RATE") return String(config.OrificeFiveFlow);
  if (var == "ORIFICE5_TEST_PRESSURE") return String(config.OrificeFiveDepression);
  if (var == "ORIFICE6_FLOW_RATE") return String(config.OrificeSixFlow);
  if (var == "ORIFICE6_TEST_PRESSURE") return String(config.OrificeSixDepression);


  
   // Lift Profile
  if (floor(config.valveLiftInterval) == config.valveLiftInterval) {

    // it's an integer so lets truncate fractional part
    int liftInterval = config.valveLiftInterval;
    if (var == "lift1") return String(1 * liftInterval);
    if (var == "lift2") return String(2 * liftInterval);
    if (var == "lift3") return String(3 * liftInterval);
    if (var == "lift4") return String(4 * liftInterval);
    if (var == "lift5") return String(5 * liftInterval);
    if (var == "lift6") return String(6 * liftInterval);
    if (var == "lift7") return String(7 * liftInterval);
    if (var == "lift8") return String(8 * liftInterval);
    if (var == "lift9") return String(9 * liftInterval);
    if (var == "lift10") return String(10 * liftInterval);
    if (var == "lift11") return String(11 * liftInterval);
    if (var == "lift12") return String(12 * liftInterval);

  } else {
    // Display the double
    if (var == "lift1") return String(1 * config.valveLiftInterval);
    if (var == "lift2") return String(2 * config.valveLiftInterval);
    if (var == "lift3") return String(3 * config.valveLiftInterval);
    if (var == "lift4") return String(4 * config.valveLiftInterval);
    if (var == "lift5") return String(5 * config.valveLiftInterval);
    if (var == "lift6") return String(6 * config.valveLiftInterval);
    if (var == "lift7") return String(7 * config.valveLiftInterval);
    if (var == "lift8") return String(8 * config.valveLiftInterval);
    if (var == "lift9") return String(9 * config.valveLiftInterval);
    if (var == "lift10") return String(10 * config.valveLiftInterval);
    if (var == "lift11") return String(11 * config.valveLiftInterval);
    if (var == "lift12") return String(12 * config.valveLiftInterval);
}



   // Wifi Settings
  if (var == "CONF_WIFI_SSID") return String(config.wifi_ssid);
  if (var == "CONF_WIFI_PSWD") return String(config.wifi_pswd);
  if (var == "CONF_WIFI_AP_SSID") return String(config.wifi_ap_ssid);
  if (var == "CONF_WIFI_AP_PSWD") return String(config.wifi_ap_pswd);
  if (var == "CONF_HOSTNAME") return String(config.hostname);
  if (var == "CONF_WIFI_TIMEOUT") return String(config.wifi_timeout);

  // API Settings
  if (var == "CONF_API_DELIM") return String(config.api_delim);
  if (var == "CONF_SERIAL_BAUD_RATE") return String(config.serial_baud_rate);

  // Data Filters
  if (var == "CONF_MIN_FLOW_RATE") return String(config.min_flow_rate);
  if (var == "CONF_MIN_BENCH_PRESSURE") return String(config.min_bench_pressure);
  if (var == "CONF_MAF_MIN_VOLTS") return String(config.maf_min_volts);
  if (var == "CONF_CYCLIC_AVERAGE_BUFFER") return String(config.cyc_av_buffer);

  // Bench Settings
  if (var == "CONF_REFRESH_RATE") return String(config.refresh_rate);
  if (var == "ADJ_FLOW_DEPRESSION") return String(config.adj_flow_depression);
  if (var == "TEMP_UNIT") return String(config.temp_unit);
  if (var == "TEMPERATURE_DROPDOWN"){
    if (strstr(String(config.temp_unit).c_str(), String("Celcius").c_str())){
      return String( "<select name='TEMP_UNIT' class='config-select' id='TEMP_UNIT'><option value='Celcius' selected>Celcius </option><option value='Farenheit'>Farenheit </option></select>");
    } else {
      return String("<select name='TEMP_UNIT' class='config-select' id='TEMP_UNIT'><option value='Celcius'>Celcius </option><option value='Farenheit' selected>Farenheit </option></select>");
    }
  }
  if (var == "VALVE_LIFT_INTERVAL") return String(config.valveLiftInterval);

  // Calibration Settings
  if (var == "CONF_CAL_FLOW_RATE") return String(config.cal_flow_rate);
  if (var == "CONF_CAL_REF_PRESS") return String(config.cal_ref_press);
  if (var == "CONF_LEAK_TEST_TOLERANCE") return String(config.leak_test_tolerance);
  if (var == "CONF_LEAK_TEST_THRESHOLD") return String(config.leak_test_threshold);

  // Calibration Data
  if (var == "FLOW_OFFSET") return String(calVal.flow_offset);
  if (var == "LEAK_CAL_PRESS_VAL") return String(calVal.leak_cal_vac_val);
  if (var == "LEAK_CAL_VAC_VAL") return String(calVal.leak_cal_vac_val);

  // Generate file list HTML code
  if (var == "FILE_LIST"){

    String fileList;
    String fileName;
    String fileSize;

    FILESYSTEM.begin();
    File root = FILESYSTEM.open("/");
    File file = root.openNextFile();
    while (file)  {
      fileName = file.name();
      fileSize = file.size();
      fileList += "<div class='fileListRow'><span class='column left'><a href='/api/file/download/" + fileName + "' download class='file-link'>" + fileName + "</a></span><span class='column middle'><span class='fileSizeTxt'>" + fileSize + " bytes</span></span><span class='column right'><form method='POST' action='/api/file/delete'><input type='hidden' name='filename' value='/" + fileName + "'><input id='delete-button'  class='button-sml' type='submit' value='Delete'></form></span></div>";
      file = root.openNextFile();
    }
    return fileList;
  }

  return "";
}



/***********************************************************
 * @brief reset wifi connection
 * @note helps to overcome ESP32/Arduino Wifi bug [#111]
 * @note https://github.com/espressif/arduino-esp32/issues/2501
 ***/
void Webserver::resetWifi ( void ) {
	WiFi.persistent(false);
  // WiFi.disconnect(true);
    WiFi.disconnect(true, true); // clears NVS
    WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_MODE_NULL);
  // delay(500);
}




/***********************************************************
 * @brief reset wifi connection
 * @note helps to overcome ESP32/Arduino Wifi bug [#111]
 * @note https://github.com/espressif/arduino-esp32/issues/2501
 * @note instantiated via API call
 * @todo instatiate via I/O (pushbutton)
 ***/
void Webserver::wifiReconnect ( void ) {

  WiFi.reconnect();
    
}





/***********************************************************
 * @brief get wifi connection
 * @return status value
 * @var wifi_retries : number of retry attempts
 * @var wifi_timeout : time in milliseconds
 ***/
int Webserver::getWifiConnection(){

  Messages _message;
  extern struct ConfigSettings config;
  
  uint8_t wifiConnectionAttempt = 1;
  uint8_t wifiConnectionStatus;

  for(;;) {
          
          WiFi.begin(config.wifi_ssid, config.wifi_pswd); 
          wifiConnectionStatus = WiFi.waitForConnectResult(config.wifi_timeout);
          if (wifiConnectionStatus == WL_CONNECTED || wifiConnectionAttempt > config.wifi_retries){
            break;
          } else if (wifiConnectionStatus != WL_DISCONNECTED) {
            resetWifi();
            delay (2000);
          } else {
            WiFi.disconnect();
          }
          _message.serialPrintf(".");
          wifiConnectionAttempt++;
          // WiFi.reconnect();
  }
 
  return wifiConnectionStatus;

}




  // TODO SD File handling / Data recording

  /*
  Need to work out how to process file list and display in GUI
  Also need to work out how to load file selected in GUI and display / edit contents
  Suspect that best method is to handle all data as JSON
  However this may need some consideration with reference to Key>Value data

  Consider... if we use valve lift as Key then we cannot hard code Key values in code as Valve interval may be changed
  Currently we take an interval value and scale it across 10 data points...

  1 / 1.5 / 3 / 4.5 / ... / 13.5 / 15

  But this may be changed to imperial or some other more appropriate interval by end user...

  100 / 200 / 300 / etc

  Which means that we cannot hard code the Key values. We can however utilise the current interval value.
  However there is also the possibility that users may use different intervals on different projects.
  Suspect easiest solution is to make valve interval editable via GUI config. This way users can change as needed.

  Also need to determine if 10 data points is enough. Need to canvas Facebook group - Performance Trends use 12 points!

  Best solution is to use dynamic number of data points but this adds complexity

  Also need to add notes field to GUI

  */


  // Code Examples...
  // https://wokwi.com/projects/323656763409695316
  // https://deguez07.medium.com/esp32-with-sd-card-modules-the-master-guide-5d391f6785d7
  // https://www.electronicwings.com/esp32/microsd-card-interfacing-with-esp32  
  // https://microcontrollerslab.com/microsd-card-esp32-arduino-ide/
  // https://www.mischianti.org/2021/03/28/how-to-use-sd-card-with-esp32-2/




/***********************************************************
 * @brief read JSON file from SD card 
 * @return JSON data
 * @NOTE: https://wokwi.com/projects/323656763409695316
 ***/
StaticJsonDocument<1024> Webserver::getFileSD(String filename){

  StaticJsonDocument<1024> dataJson;

  // Example of reading file from SD card:
  File textFile = SD.open("/wokwi.txt");
  if (textFile) {
    Serial.print("wokwi.txt: ");
    while (textFile.available()) {
      Serial.write(textFile.read());
    }
    textFile.close();
  } else {
    Serial.println("error opening wokwi.txt!");
  }

  return dataJson;

}




/***********************************************************
 * @brief read file list from SD card 
 * @return JSON formatted list
 * 
 ***/
StaticJsonDocument<1024> Webserver::getFileListSD(String filename){

  //TODO - not coded yet

  StaticJsonDocument<1024> dataJson;

  File root;

  Serial.println("Files in the card:");
  root = SD.open("/");
  // printDirectory(root, 0);
  Serial.println("");



  File dir;
  int numTabs = 10;


  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      // printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }

  return dataJson;

}