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
#include <FS.h>
#include <SPI.h>
#include <SD.h>
#include <Update.h>

#include "datahandler.h"
#include "system.h"
#include "configuration.h"
#include "constants.h"
#include "structs.h"
#include "comms.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "webserver.h"

#include "calibration.h"
#include "sensors.h"

#include "hardware.h"
#include "messages.h"
#include "calculations.h"


#include <sstream>
using namespace std;

#define U_PART U_SPIFFS

// RTC_DATA_ATTR int bootCount; // flash mem

const char LANDING_PAGE[] PROGMEM = "<!DOCTYPE HTML> <html lang='en'> <HEAD> <title>DIY Flow Bench</title> <meta name='viewport' content='width=device-width, initial-scale=1'> <script> function onFileUpload(event) { this.setState({ file: event.target.files[0] }); const { file } = this.state; const data = new FormData; data.append('data', file); fetch('/api/file/upload', { method: 'POST', body: data }).catch(e => { console.log('Request failed', e); }); } </script> <style> body, html { height: 100%; margin: 0; font-family: Arial; font-size: 22px } a:link { color: #0A1128; text-decoration: none } a:visited, a:active { color: #0A1128; text-decoration: none } a:hover { color: #666; text-decoration: none } .headerbar { overflow: hidden; background-color: #0A1128; text-align: center } .headerbar h1 a:link, .headerbar h1 a:active, .headerbar h1 a:visited, .headerbar h1 a:hover { color: white; text-decoration: none } .align-center { text-align: center } .file-upload-button { padding: 12px 0px; text-align: center } .button { display: inline-block; background-color: #008CBA; border: none; border-radius: 4px; color: white; padding: 12px 12px; text-decoration: none; font-size: 22px; margin: 2px; cursor: pointer; width: 150px } #footer { clear: both; text-align: center } .file-upload-button { padding: 12px 0px; text-align: center } .file-submit-button { padding: 12px 0px; text-align: center; font-size: 15px; padding: 6px 6px; } .input_container { border: 1px solid #e5e5e5; } input[type=file]::file-selector-button { background-color: #fff; color: #000; border: 0px; border-right: 1px solid #e5e5e5; padding: 10px 15px; margin-right: 20px; transition: .5s; } input[type=file]::file-selector-button:hover { background-color: #eee; border: 0px; border-right: 1px solid #e5e5e5; } </style> </HEAD> <BODY> <div class='headerbar'> <h1><a href='/'>DIY Flow Bench</a></h1> </div> <br> <div class='align-center'> <p>Welcome to the DIY Flow Bench. Thank you for supporting our project.</p> <p>Please upload the following files to get started.</p> <p>~INDEX_STATUS~</p> <p>~PINS_STATUS~</p> <!--<p>~SETTINGS_STATUS~</p>--> <br> <form method='POST' action='/api/file/upload' enctype='multipart/form-data'> <div class=\"input_container\"> <input type=\"file\" name=\"upload\" id=\"fileUpload\"> <input type='submit' value='Upload' class=\"button file-submit-button\"> </div> </form> </div> <br> <div id='footer'><a href='https://diyflowbench.com' target='new'>DIYFlowBench.com</a></div> <br> </BODY> </HTML>";

void Webserver::begin()
{
  extern struct ConfigSettings config;
  extern struct Language language;
  extern struct DeviceStatus status;

  int wifiStatusCode;

  server = new AsyncWebServer(80);
  events = new AsyncEventSource("/events");

  Messages _message;
  Calibration _calibration;
  DataHandler _data;

  // API request handlers [JSON confirmation response]
  server->on("/api/bench/on", HTTP_GET, [](AsyncWebServerRequest *request){
      Messages _message;
      Hardware _hardware;
      _message.Handler(language.LANG_BENCH_RUNNING);
      _message.debugPrintf("Bench On \n");
      _hardware.benchOn(); 
      request->send(200, "text/html", "{\"bench\":\"on\"}"); 
      });

  server->on("/api/bench/off", HTTP_GET, [](AsyncWebServerRequest *request){
      Messages _message;
      Hardware _hardware;
      _message.Handler(language.LANG_BENCH_STOPPED);
      _message.debugPrintf("Bench Off \n");
      _hardware.benchOff(); 
      request->send(200, "text/html", "{\"bench\":\"off\"}"); 
      });

  server->on("/api/debug/on", HTTP_GET, [](AsyncWebServerRequest *request){
      Messages _message;
      _message.Handler(language.LANG_DEBUG_MODE);
      _message.debugPrintf("Debug Mode On\n");
      config.debug_mode = true;
      request->send(200, "text/html", "{\"debug\":\"on\"}"); });

  server->on("/api/debug/off", HTTP_GET, [](AsyncWebServerRequest *request){
      Messages _message;
      _message.Handler(language.LANG_BLANK);
      _message.debugPrintf("Debug Mode Off\n");
      config.debug_mode = false;
      request->send(200, "text/html", "{\"debug\":\"off\"}"); });

  server->on("/api/dev/on", HTTP_GET, [](AsyncWebServerRequest *request) {
      Messages _message;
      _message.Handler(language.LANG_DEV_MODE);
      _message.debugPrintf("Developer Mode On\n");
      config.dev_mode = true;
      request->send(200, "text/html", "{\"dev\":\"on\"}"); });

  server->on("/api/dev/off", HTTP_GET, [](AsyncWebServerRequest *request){
      Messages _message;
      _message.Handler(language.LANG_BLANK);
      _message.debugPrintf("Developer Mode Off\n");
      config.dev_mode = false;
      request->send(200, "text/html", "{\"dev\":\"off\"}"); });

  server->on("/api/clear-message", HTTP_GET, [](AsyncWebServerRequest *request) {
      Messages _message;
      status.statusMessage = "";
      _message.Handler(language.LANG_NO_ERROR);
      _message.debugPrintf("Clearing messages...\n");
       });

    server->on("/api/orifice-change", HTTP_GET, [](AsyncWebServerRequest *request){
      Messages _message;
      _message.Handler(language.LANG_ORIFICE_CHANGE);
      _message.debugPrintf("Active Orifice Changed\n");
      status.activeOrifice = request->arg("orifice");
      request->send(200, "text/html", "{\"orifice\":changed\"\"}"); });

  server->on("/api/bench/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
      Messages _message;
      _message.Handler(language.LANG_SYSTEM_REBOOTING);
      request->send(200, "text/html", "{\"reboot\":\"true\"}");
      ESP.restart(); 
      request->redirect("/"); });

  server->on("/api/bench/calibrate", HTTP_GET, [](AsyncWebServerRequest *request){
      Messages _message;
      Calibration _calibrate;
      Hardware _hardware;
      if (_hardware.benchIsRunning()) {
        _message.Handler(language.LANG_CALIBRATING);
        _message.debugPrintf("Calibrating Flow...\n");
        request->send(200, "text/html", "{\"calibrate\":\"true\"}");
        _calibrate.setFlowOffset();         
      } else {
        _message.Handler(language.LANG_RUN_BENCH_TO_CALIBRATE);
        request->send(200, "text/html", "{\"calibrate\":\"false\"}");
      }  
      request->redirect("/");
      });

  server->on("/api/bench/leakcal", HTTP_GET, [](AsyncWebServerRequest *request){
      Messages _message;
      Calibration _calibrate;
      Hardware _hardware;
      if (_hardware.benchIsRunning()) {
        _message.Handler(language.LANG_LEAK_CALIBRATING);
        _message.debugPrintf("Calibrating Leak Test...\n");
        request->send(200, "text/html", "{\"leakcal\":\"true\"}");
        _calibrate.setLeakOffset();
      } else {
        _message.Handler(language.LANG_RUN_BENCH_TO_CALIBRATE);
        request->send(200, "text/html", "{\"leakcal\":\"false\"}");
      } 
      request->redirect("/"); });

  // Upload request handler
  server->on("/api/file/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
      Messages _message;
      _message.debugPrintf("/api/file/upload \n");
      request->send(200);
      // request->redirect("/?view=upload"); 
      },
      processUpload);

  // Download request handler
  server->on("/api/file/download", HTTP_GET, [](AsyncWebServerRequest *request){              
      Messages _message;
      String downloadFilename = request->url();
      downloadFilename.remove(0,18); // Strip the file path (first 18 chars)
      _message.debugPrintf("Request Download File: %s \n", downloadFilename);
      request->send(SPIFFS, downloadFilename, String(), true); });

  // Simple Firmware Update Form
  server->on("/api/update", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", "<form method='POST' action='/api/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
  });

  // Firmware update handler
  server->on("/api/update", HTTP_POST, [](AsyncWebServerRequest *request){
    status.shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", status.shouldReboot?"<meta http-equiv=\"refresh\" content=\"3; url=/\" />Rebooting... If the page does not automatically refresh, pleae click <a href=\"/\">HERE</a>":"FIRMWARE UPDATE FAILED!");
    response->addHeader("Connection", "close");
    request->send(response);
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    Messages _message;
    if(!index){
      _message.debugPrintf("Update Start: %s\n", filename.c_str());
      // Update.runAsync(true);
      if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
        _message.debugPrintf("Not Rnough Room: \n");
        Update.printError(Serial);
      }
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len){
        _message.debugPrintf("Write error: \n");
        Update.printError(Serial);
      }
    }
    if(final){
      if(Update.end(true)){
        Serial.printf("Update Success: %uB\n", index+len);
      } else {
        Update.printError(Serial);
      }
    }
  });

  // Delete request handler
  server->on("/api/file/delete", HTTP_POST, [](AsyncWebServerRequest *request){              
      Messages _message;
      String fileToDelete;
      DataHandler _data;
      AsyncWebParameter *p = request->getParam("filename", true);
      fileToDelete = p->value();      
      // Don't delete index.html (you can overwrite it!!)
      // if (fileToDelete != "/index.html"){
        if(SPIFFS.exists(fileToDelete)){
          _message.debugPrintf("Deleting File: %s\n", fileToDelete.c_str());  
          SPIFFS.remove(fileToDelete);
        }  else {
          _message.debugPrintf("Delete Failed: %s\n", fileToDelete.c_str());  
          _message.Handler(language.LANG_DELETE_FAILED);    
        } 
        if (_data.checkSubstring(fileToDelete.c_str(), status.pinsFilename.c_str())) status.pinsLoaded = false;
        if (_data.checkSubstring(fileToDelete.c_str(), status.mafFilename.c_str())) status.mafLoaded = false;
        if (fileToDelete == "/index.html"){
          request->redirect("/");
        } else {
          request->redirect("/?view=upload");
        }
      
       });


  // Toggle Flow Dif Tile
  server->on("/api/fdiff", HTTP_GET, [](AsyncWebServerRequest *request){
    Messages _message;
    // _message.Handler(language.LANG_BENCH_RUNNING);
    _message.debugPrintf("/api/fdiff/toggle \n");
    toggleFlowDiffTile();
    request->send(200);
    // request->send(200, "text/html", "{\"fdiff\":\"changed\"}"); 
  });


  // Zero pDiff value
  server->on("/api/pdiff/zero", HTTP_GET, [](AsyncWebServerRequest *request){
    Messages _message;
    Calibration _cal;
    _message.debugPrintf("/api/fdiff/zero \n");
    _cal.setPdiffCalOffset();
    request->send(200);
    // request->send(200, "text/html", "{\"fdiff\":\"changed\"}"); 
  });

  
  // Zero Pitot value
  server->on("/api/pitot/zero", HTTP_GET, [](AsyncWebServerRequest *request){
    Messages _message;
    Calibration _cal;
    _message.debugPrintf("/api/pitot/zero \n");
    _cal.setPitotCalOffset();
    request->send(200);
    // request->send(200, "text/html", "{\"fdiff\":\"changed\"}"); 
  });

  
  // Send JSON Data
  server->on("/api/json", HTTP_GET, [](AsyncWebServerRequest *request){
    DataHandler _data;
    request->send(200, "text/html", String(_data.getDataJSON()).c_str());
  });


  // Save user Flow Diff target
  server->on("/api/saveflowtarget", HTTP_POST, parseUserFlowTargetForm);
  
  // Parse Configuration Form
  server->on("/api/saveconfig", HTTP_POST, parseConfigurationForm);

  // Parse Calibration Form
  server->on("/api/savecalibration", HTTP_POST, parseCalibrationForm);

  // Parse Lift Data Form
  server->on("/api/saveliftdata", HTTP_POST, parseLiftDataForm);

  // Parse Orifice Form
  server->on("/api/saveorifice", HTTP_POST, parseOrificeForm);

  // Clear Lift Data
  server->on("/api/clearLiftData", HTTP_POST, [](AsyncWebServerRequest *request) {
    DataHandler _data;
    _data.clearLiftDataFile;
  });  

  // index.html
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
      extern struct DeviceStatus status;
      if ((SPIFFS.exists("/index.html")) && ((status.pinsLoaded == true))) {
        request->send(SPIFFS, "/index.html", "text/html", false, processTemplate);
       } else {
        request->send_P(200, "text/html", LANDING_PAGE, processLandingPageTemplate); 
       }
      });

  server->onFileUpload(processUpload);
  server->addHandler(events);
  server->begin();

  _message.Handler(language.LANG_SERVER_RUNNING);
  _message.serialPrintf("Server Running \n");
}





/***********************************************************
 * @brief Process File Upload
 * @note Redirects browser back to Upload modal unless upload is index file
 ***/
void Webserver::processUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{

  Messages _message;
  DataHandler _data;
  String redirectURL;
  extern struct DeviceStatus status;
  extern struct Language language;
  extern struct DeviceStatus status;

  bool upload_error = false;
  int file_size = 0;

  if (!filename.startsWith("/")){
    filename = "/" + filename;
  }

  uint32_t freespace = SPIFFS.totalBytes() - SPIFFS.usedBytes();

//  if (!index && !upload_error)  {
  if (!index)  {
    _message.debugPrintf("UploadStart: %s \n", filename.c_str());
    // open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open(filename, "w");
  }

  if (len)  {
    file_size += len;
    if (file_size > freespace)    {
      _message.Handler(language.LANG_UPLOAD_FAILED_NO_SPACE);
      _message.debugPrintf("Upload failed, no Space: %s \n", freespace);
      upload_error = true;
    }    else    {
      _message.Handler(language.LANG_FILE_UPLOADED);
      _message.debugPrintf("Writing file: '%s' index=%u len=%u \n", filename.c_str(), index, len);
      // stream the incoming chunk to the opened file
      request->_tempFile.write(data, len);
    }
  } 

  // Set redirect to file Upload modal unless uploading the index file
  if (filename == String("/index.html.gz") || (filename == String("/index.html")) || (filename == String(status.pinsFilename)))  {
    redirectURL = "/";
  }  else  {
    redirectURL = "/?view=upload";
  }

  if (final)  {
    _message.debugPrintf("Upload Complete: %s, %u bytes\n", filename.c_str(), file_size);
    request->_tempFile.close();

    if (_data.checkUserFile(PINSFILE)) status.pinsLoaded = true;
    if (_data.checkUserFile(MAFFILE)) status.mafLoaded = true;  

    request->redirect(redirectURL);
  }
}







/***********************************************************
 * @brief parseConfigurationForm
 * @details Parses calibration form post vars and stores into global struct
 * @details Saves configuration data to config.json file
 * @note Creates file if it does not exist
 * @note Redirects browser to file list
 * 
 ***/
void Webserver::parseConfigurationForm(AsyncWebServerRequest *request)
{

  Messages _message;
  Webserver _webserver;
  DataHandler _data;

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
  config.maf_housing_diameter = configData["CONF_MAF_HOUSING_DIAMETER"].as<int>();
  config.refresh_rate = configData["CONF_REFRESH_RATE"].as<int>();
  config.min_bench_pressure  = configData["CONF_MIN_BENCH_PRESSURE"].as<int>();
  config.min_flow_rate = configData["CONF_MIN_FLOW_RATE"].as<int>();
  strcpy(config.data_filter_type, configData["DATA_FILTER_TYPE"]);
  strcpy(config.rounding_type, configData["ROUNDING_TYPE"]);
  config.flow_decimal_length = configData["FLOW_DECIMAL_LENGTH"].as<int>();
  config.gen_decimal_length = configData["GEN_DECIMAL_LENGTH"].as<int>();
  config.cyc_av_buffer  = configData["CONF_CYCLIC_AVERAGE_BUFFER"].as<int>();
  config.maf_min_volts  = configData["CONF_MAF_MIN_VOLTS"].as<int>();
  strcpy(config.api_delim, configData["CONF_API_DELIM"]);
  config.serial_baud_rate = configData["CONF_SERIAL_BAUD_RATE"].as<long>();
  config.show_alarms = configData["CONF_SHOW_ALARMS"].as<bool>();
  config.adj_flow_depression = configData["ADJ_FLOW_DEPRESSION"].as<int>();
  config.standardReference = configData["STANDARD_REFERENCE"].as<int>();
  config.std_adj_flow = configData["STD_ADJ_FLOW"].as<int>();
  config.dataGraphMax = configData["DATAGRAPH_MAX"].as<int>();
  strcpy(config.temp_unit, configData["TEMP_UNIT"]);
  config.valveLiftInterval = configData["VALVE_LIFT_INTERVAL"].as<double>();
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

  // save settings to config file
  serializeJsonPretty(configData, jsonString);
  if (SPIFFS.exists("/config.json"))  {
    SPIFFS.remove("/config.json");
  }
  _data.writeJSONFile(jsonString, "/config.json", CONFIG_JSON_SIZE);

  _message.debugPrintf("Configuration Saved \n");

  // request->redirect("/?view=config");
  request->redirect("/");

}





/***********************************************************
 * @brief saveCalibration
 * @details Saves calibration data to cal.json file
 * @note Creates file if it does not exist
 * @note Redirects browser to configuration tab
 * @note duplicates _calibration.saveCalibrationData whjich is unable to be called from server->on directive
 * 
 ***/
void Webserver::parseCalibrationForm(AsyncWebServerRequest *request)
{

  Calibration _calibrate;
  Messages _message;
  // Webserver _webserver;

  StaticJsonDocument<CAL_DATA_JSON_SIZE> calData;
  extern struct CalibrationData calVal;
  // String jsonString;

  int params = request->params();

  _message.debugPrintf("Parsing Calibration Form Data... \n");

  // Convert POST vars to JSON 
  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
      calData[p->name().c_str()] = p->value().c_str();
  }

  // Update global Config Vars
  calVal.flow_offset = calData["FLOW_OFFSET"].as<double>();
  calVal.user_offset = calData["USER_OFFSET"].as<double>();
  calVal.leak_cal_offset = calData["LEAK_CAL_OFFSET"].as<double>();
  calVal.leak_cal_offset_rev = calData["LEAK_CAL_OFFSET_REV"].as<double>();
  calVal.leak_cal_baseline= calData["LEAK_CAL_BASELINE"].as<double>();
  calVal.leak_cal_baseline_rev = calData["LEAK_CAL_BASELINE_REV"].as<double>();

  _message.debugPrintf("Calibration form post vars parsed \n");

  _calibrate.saveCalibrationData();

  // request->redirect("/?view=config");
  request->redirect("/");

}






/***********************************************************
 * @brief saveCalibration
 * @details Saves calibration data to cal.json file
 * @note Creates file if it does not exist
 * @note Redirects browser to configuration tab
 * @note duplicates _calibration.saveCalibrationData whjich is unable to be called from server->on directive
 * 
 ***/
void Webserver::parseUserFlowTargetForm(AsyncWebServerRequest *request)
{

  Calibration _calibrate;
  Messages _message;
  // Webserver _webserver;

  StaticJsonDocument<CAL_DATA_JSON_SIZE> calData;
  extern struct CalibrationData calVal;
  // String jsonString;

  int params = request->params();

  _message.debugPrintf("Parsing User Flow Target Form Data... \n");

  // Convert POST vars to JSON 
  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
      calData[p->name().c_str()] = p->value().c_str();
  }

  // Update global Config Vars
  calVal.user_offset = calData["USER_OFFSET"].as<double>();
  // calVal.flow_offset = calData["FLOW_OFFSET"].as<double>();
  // calVal.leak_cal_offset = calData["LEAK_CAL_OFFSET"].as<double>();
  // calVal.leak_cal_offset_rev = calData["LEAK_CAL_OFFSET_REV"].as<double>();
  // calVal.leak_cal_baseline= calData["LEAK_CAL_BASELINE"].as<double>();
  // calVal.leak_cal_baseline_rev = calData["LEAK_CAL_BASELINE_REV"].as<double>();

  _message.debugPrintf("sUer Flow Target Form Data parsed \n");

  _calibrate.saveCalibrationData();

  request->redirect("/");

}





/***********************************************************
* @brief toggleFlowDiffTile
* @details advances data assinged to Flow Differential tile
***/
void Webserver::toggleFlowDiffTile () {

  Messages _message;

  extern struct SensorData sensorVal;
  extern struct CalibrationData calVal;

  _message.debugPrintf("Flow Differential: %u \n", sensorVal.FDiffType);

    // Flow differential type
  switch (sensorVal.FDiffType) {

  case USERTARGET:
    sensorVal.FDiffType = 2;
    break;

  case BASELINE:
    sensorVal.FDiffType = 3;
    break;

  case BASELINE_LEAK :
    sensorVal.FDiffType = 1;
    break;


  default:
    break;
  }

}








/***********************************************************
 * @brief captureLiftData
 * @details captures lift data to working memory
 * 
 ***/
void Webserver::parseLiftDataForm(AsyncWebServerRequest *request)
{

  Messages _message;
  DataHandler _data;
  Webserver _webserver;

  StaticJsonDocument<LIFT_DATA_JSON_SIZE> liftData;
  extern struct SensorData sensorVal;
  extern struct ValveLiftData valveData;
  extern struct ConfigSettings config;
  
  String jsonString;
  String liftPoint;
  int switchval;
  char *end;
  int params = request->params();
  const char* PARAM_INPUT = "lift-data";
  double flowValue;

  _message.debugPrintf("Saving Lift Data...\n");

  // Convert POST vars to JSON 
  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);

        // get selected radio button and store it (radio button example from https://www.electrorules.com/esp32-web-server-control-stepper-motor-html-form/)
        if (p->name() == PARAM_INPUT) {
          liftPoint = p->value();
        }

  }


  // Get flow value based on capture Datatype
  switch (config.data_capture_datatype) {

    case ACFM:
      flowValue = sensorVal.FlowCFM;
    break;
    
    case STD_ACFM:
      flowValue = sensorVal.FlowSCFM;
    break;
    
    case ADJ_ACFM:
      flowValue = sensorVal.FlowADJ;
    break;
    
    case ADJ_STD_ACFM:
      flowValue = sensorVal.FlowADJSCFM;
    break;
    
    case MAF:
      flowValue = sensorVal.FlowKGH;
    break;
        
  }


  // Update lift point data
  switchval = strtol(liftPoint.c_str(), &end, 10); // convert std::str to int

  switch (switchval) {

    case 1:
      valveData.LiftData1 = flowValue;
      break;

    case 2:
      valveData.LiftData2 = flowValue;
      break;

    case 3:
      valveData.LiftData3 = flowValue;
      break;

    case 4:
      valveData.LiftData4 = flowValue;
      break;

    case 5:
      valveData.LiftData5 = flowValue;
      break;

    case 6:
      valveData.LiftData6 = flowValue;
      break;

    case 7:
      valveData.LiftData7 = flowValue;
      break;

    case 8:
      valveData.LiftData8 = flowValue;
      break;

    case 9:
      valveData.LiftData9 = flowValue;
      break;

    case 10:
      valveData.LiftData10 = flowValue;
      break;

    case 11:
      valveData.LiftData11 = flowValue;
      break;

    case 12:
      valveData.LiftData12 = flowValue;
      break;
  }

  liftData["LIFTDATA1"] = valveData.LiftData1;
  liftData["LIFTDATA2"] = valveData.LiftData2;
  liftData["LIFTDATA3"] = valveData.LiftData3;
  liftData["LIFTDATA4"] = valveData.LiftData4;
  liftData["LIFTDATA5"] = valveData.LiftData5;
  liftData["LIFTDATA6"] = valveData.LiftData6;
  liftData["LIFTDATA7"] = valveData.LiftData7;
  liftData["LIFTDATA8"] = valveData.LiftData8;
  liftData["LIFTDATA9"] = valveData.LiftData9;
  liftData["LIFTDATA10"] = valveData.LiftData10;
  liftData["LIFTDATA11"] = valveData.LiftData11;
  liftData["LIFTDATA12"] = valveData.LiftData12;
    
  // save settings to liftdata file
  serializeJsonPretty(liftData, jsonString);
  if (SPIFFS.exists("/liftdata.json"))  {
    SPIFFS.remove("/liftdata.json");
  }
  _data.writeJSONFile(jsonString, "/liftdata.json", LIFT_DATA_JSON_SIZE);

  request->redirect("/");


}



/***********************************************************
 * @brief getValveDataJSON
 * @details Package up current bench data into JSON string
 ***/
String Webserver::getValveDataJSON()
{
  extern struct ValveLiftData valveData;

  String jsonString;

  StaticJsonDocument<DATA_JSON_SIZE> liftData;

  liftData["LIFTDATA1"] = valveData.LiftData1;
  liftData["LIFTDATA2"] = valveData.LiftData2;
  liftData["LIFTDATA3"] = valveData.LiftData3;
  liftData["LIFTDATA4"] = valveData.LiftData4;
  liftData["LIFTDATA5"] = valveData.LiftData5;
  liftData["LIFTDATA6"] = valveData.LiftData6;
  liftData["LIFTDATA7"] = valveData.LiftData7;
  liftData["LIFTDATA8"] = valveData.LiftData8;
  liftData["LIFTDATA9"] = valveData.LiftData9;
  liftData["LIFTDATA10"] = valveData.LiftData10;
  liftData["LIFTDATA11"] = valveData.LiftData11;
  liftData["LIFTDATA12"] = valveData.LiftData12;

  serializeJson(liftData, jsonString);

  return jsonString;

}




/***********************************************************
 * @brief parseOrificeForm
 * @details Sets selected orifice and loads orifice data
 * @todo //TODO auto orifice decode
 ***/
void Webserver::parseOrificeForm(AsyncWebServerRequest *request)
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
  extern struct Language language;

  Messages _message;

  // Translate GUI
  // Note language struct is overwritten when language.json file is present
  if (var == "LANG_GUI_SELECT_LIFT_VAL_BEFORE_CAPTURE") return language.LANG_GUI_SELECT_LIFT_VAL_BEFORE_CAPTURE;
  if (var == "LANG_GUI_LIFT_VALUE") return language.LANG_GUI_LIFT_VALUE;
  if (var == "LANG_GUI_LIFT_CAPTURE") return language.LANG_GUI_LIFT_CAPTURE;
  if (var == "LANG_GUI_UPLOAD_FIRMWARE_BINARY") return language.LANG_GUI_UPLOAD_FIRMWARE_BINARY;
  if (var == "LANG_GUI_FIRMWARE_UPDATE") return language.LANG_GUI_FIRMWARE_UPDATE;
  if (var == "LANG_GUI_USER_FLOW_TARGET_VALUE") return language.LANG_GUI_USER_FLOW_TARGET_VALUE;
  if (var == "LANG_GUI_USER_FLOW_TARGET_SAVE") return language.LANG_GUI_USER_FLOW_TARGET_SAVE;
  if (var == "LANG_GUI_CAL_FLOW_OFFSET") return language.LANG_GUI_CAL_FLOW_OFFSET;
  if (var == "LANG_GUI_CAL_LEAK_TEST") return language.LANG_GUI_CAL_LEAK_TEST;
  if (var == "LANG_GUI_LOAD_LIFT_PROFILE") return language.LANG_GUI_LOAD_LIFT_PROFILE;
  if (var == "LANG_GUI_LOAD_LIFT_PROFILE_LOAD") return language.LANG_GUI_LOAD_LIFT_PROFILE_LOAD;
  if (var == "LANG_GUI_SAVE_LIFT_DATA") return language.LANG_GUI_SAVE_LIFT_DATA;
  if (var == "LANG_GUI_SAVE_GRAPH_FILENAME") return language.LANG_GUI_SAVE_GRAPH_FILENAME;
  if (var == "LANG_GUI_FILE_MANAGER") return language.LANG_GUI_FILE_MANAGER;
  if (var == "LANG_GUI_FILEMANAGER_UPLOAD") return language.LANG_GUI_FILEMANAGER_UPLOAD;
  if (var == "LANG_GUI_FIRMWARE") return language.LANG_GUI_FIRMWARE;
  if (var == "LANG_GUI_VERSION") return language.LANG_GUI_VERSION;
  if (var == "LANG_GUI_GUI") return language.LANG_GUI_GUI;
  if (var == "LANG_GUI_BUILD") return language.LANG_GUI_BUILD;
  if (var == "LANG_GUI_MEM_SIZE") return language.LANG_GUI_MEM_SIZE;
  if (var == "LANG_GUI_MEM_USED") return language.LANG_GUI_MEM_USED;
  if (var == "LANG_GUI_STORAGE") return language.LANG_GUI_STORAGE;
  if (var == "LANG_GUI_NETWORK") return language.LANG_GUI_NETWORK;
  if (var == "LANG_GUI_IP_ADDRESS") return language.LANG_GUI_IP_ADDRESS;
  if (var == "LANG_GUI_HOSTNAME") return language.LANG_GUI_HOSTNAME;
  if (var == "LANG_GUI_HARDWARE_CONFIG") return language.LANG_GUI_HARDWARE_CONFIG;
  if (var == "LANG_GUI_BENCH_TYPE") return language.LANG_GUI_BENCH_TYPE;
  if (var == "LANG_GUI_BOARD_TYPE") return language.LANG_GUI_BOARD_TYPE;
  if (var == "LANG_GUI_SENSOR_CONFIG") return language.LANG_GUI_SENSOR_CONFIG;
  if (var == "LANG_GUI_MAF_DATA_FILE") return language.LANG_GUI_MAF_DATA_FILE;
  if (var == "LANG_GUI_REF_PRESSURE_SENSOR") return language.LANG_GUI_REF_PRESSURE_SENSOR;
  if (var == "LANG_GUI_TEMP_SENSOR") return language.LANG_GUI_TEMP_SENSOR;
  if (var == "LANG_GUI_HUMIDITY_SENSOR") return language.LANG_GUI_HUMIDITY_SENSOR;
  if (var == "LANG_GUI_BARO_SENSOR") return language.LANG_GUI_BARO_SENSOR;
  if (var == "LANG_GUI_PITOT_SENSOR") return language.LANG_GUI_PITOT_SENSOR;
  if (var == "LANG_GUI_DIFFERENTIAL_SENSOR") return language.LANG_GUI_DIFFERENTIAL_SENSOR;
  if (var == "LANG_GUI_PITOT") return language.LANG_GUI_PITOT;
  if (var == "LANG_GUI_PREF") return language.LANG_GUI_PREF;
  if (var == "LANG_GUI_PDIFF") return language.LANG_GUI_PDIFF;
  if (var == "LANG_GUI_FLOW") return language.LANG_GUI_FLOW;
  if (var == "LANG_GUI_AFLOW") return language.LANG_GUI_AFLOW;
  if (var == "LANG_GUI_SFLOW") return language.LANG_GUI_SFLOW;
  if (var == "LANG_GUI_MFLOW") return language.LANG_GUI_MFLOW;
  if (var == "LANG_GUI_SWIRL") return language.LANG_GUI_SWIRL;
  if (var == "LANG_GUI_FLOW_DIFF") return language.LANG_GUI_FLOW_DIFF;
  if (var == "LANG_GUI_TEMP") return language.LANG_GUI_TEMP;
  if (var == "LANG_GUI_BARO") return language.LANG_GUI_BARO;
  if (var == "LANG_GUI_HUMIDITY") return language.LANG_GUI_HUMIDITY;
  if (var == "LANG_GUI_START") return language.LANG_GUI_START;
  if (var == "LANG_GUI_STOP") return language.LANG_GUI_STOP;
  if (var == "LANG_GUI_CLEAR_ALARM") return language.LANG_GUI_CLEAR_ALARM;
  if (var == "LANG_GUI_CAPTURE") return language.LANG_GUI_CAPTURE;
  if (var == "LANG_GUI_DASHBOARD") return language.LANG_GUI_DASHBOARD;
  if (var == "LANG_GUI_DATA") return language.LANG_GUI_DATA;
  if (var == "LANG_GUI_CONFIG") return language.LANG_GUI_CONFIG;
  if (var == "LANG_GUI_CLEAR") return language.LANG_GUI_CLEAR;
  if (var == "LANG_GUI_EXPORT") return language.LANG_GUI_EXPORT;
  if (var == "LANG_GUI_IMAGE") return language.LANG_GUI_IMAGE;
  if (var == "LANG_GUI_SYS_SETTINGS") return language.LANG_GUI_SYS_SETTINGS;
  if (var == "LANG_GUI_WIFI_INFO") return language.LANG_GUI_WIFI_INFO;
  if (var == "LANG_GUI_WIFI_SSID") return language.LANG_GUI_WIFI_SSID;
  if (var == "LANG_GUI_WIFI_PASS") return language.LANG_GUI_WIFI_PASS;
  if (var == "LANG_GUI_WIFI_AP_SSID") return language.LANG_GUI_WIFI_AP_SSID;
  if (var == "LANG_GUI_WIFI_AP_PASS") return language.LANG_GUI_WIFI_AP_PASS;
  if (var == "LANG_GUI_HOSTNAME") return language.LANG_GUI_HOSTNAME;
  if (var == "LANG_GUI_WIFI_TIMEOUT") return language.LANG_GUI_WIFI_TIMEOUT;
  if (var == "LANG_GUI_GENERAL_SETTINGS") return language.LANG_GUI_GENERAL_SETTINGS;
  if (var == "LANG_GUI_BENCH_TYPE") return language.LANG_GUI_BENCH_TYPE;
  if (var == "LANG_GUI_MAF_HOUSING_DIAMETER") return language.LANG_GUI_MAF_HOUSING_DIAMETER;
  if (var == "LANG_GUI_REFRESH_RATE") return language.LANG_GUI_REFRESH_RATE;
  if (var == "LANG_GUI_TEMPERATURE_UNIT") return language.LANG_GUI_TEMPERATURE_UNIT;
  if (var == "LANG_GUI_VALVE_LIFT_INTERVAL") return language.LANG_GUI_VALVE_LIFT_INTERVAL;
  if (var == "LANG_GUI_DATA_GRAPH_MAX_VALUE") return language.LANG_GUI_DATA_GRAPH_MAX_VALUE;
  if (var == "LANG_GUI_AUTO") return language.LANG_GUI_AUTO;
  if (var == "LANG_GUI_RESOLUTION_AND_ACCURACY") return language.LANG_GUI_RESOLUTION_AND_ACCURACY;
  if (var == "LANG_GUI_FLOW_VALUE_ROUNDING") return language.LANG_GUI_FLOW_VALUE_ROUNDING;
  if (var == "LANG_GUI_FLOW_DECIMAL_ROUNDING") return language.LANG_GUI_FLOW_DECIMAL_ROUNDING;
  if (var == "LANG_GUI_GEN_DECIMAL_ACCURACY") return language.LANG_GUI_GEN_DECIMAL_ACCURACY;
  if (var == "LANG_GUI_DATA_FILTERS") return language.LANG_GUI_DATA_FILTERS;
  if (var == "LANG_GUI_DATA_FILTER_TYPE") return language.LANG_GUI_DATA_FILTER_TYPE;
  if (var == "LANG_GUI_MIN_FLOW_RATE") return language.LANG_GUI_MIN_FLOW_RATE;
  if (var == "LANG_GUI_MIN_BENCH_PRESSURE") return language.LANG_GUI_MIN_BENCH_PRESSURE;
  if (var == "LANG_GUI_MAF_MIN_VOLTS") return language.LANG_GUI_MAF_MIN_VOLTS;
  if (var == "LANG_GUI_CYCLIC_AVERAGE_BUFFER") return language.LANG_GUI_CYCLIC_AVERAGE_BUFFER;
  if (var == "LANG_GUI_CONVERSION_SETTINGS") return language.LANG_GUI_CONVERSION_SETTINGS;
  if (var == "LANG_GUI_ADJ_FLOW_DEPRESSION") return language.LANG_GUI_ADJ_FLOW_DEPRESSION;
  if (var == "LANG_GUI_STANDARD_REF_CONDITIONS") return language.LANG_GUI_STANDARD_REF_CONDITIONS;
  if (var == "LANG_GUI_STANDARDISED_ADJ_FLOW") return language.LANG_GUI_STANDARDISED_ADJ_FLOW;
  if (var == "LANG_GUI_CAL_ORIFICE_SETTINGS") return language.LANG_GUI_CAL_ORIFICE_SETTINGS;
  if (var == "LANG_GUI_CAL_ORIFICE_FLOW_RATE") return language.LANG_GUI_CAL_ORIFICE_FLOW_RATE;
  if (var == "LANG_GUI_CAL_ORIFICE_TEST_PRESS") return language.LANG_GUI_CAL_ORIFICE_TEST_PRESS;
  if (var == "LANG_GUI_ORIFICE_DATA") return language.LANG_GUI_ORIFICE_DATA;
  if (var == "LANG_GUI_ORIFICE1_FLOW") return language.LANG_GUI_ORIFICE1_FLOW;
  if (var == "LANG_GUI_ORIFICE1_PRESSURE") return language.LANG_GUI_ORIFICE1_PRESSURE;
  if (var == "LANG_GUI_ORIFICE2_FLOW") return language.LANG_GUI_ORIFICE2_FLOW;
  if (var == "LANG_GUI_ORIFICE2_PRESSURE") return language.LANG_GUI_ORIFICE2_PRESSURE;
  if (var == "LANG_GUI_ORIFICE3_FLOW") return language.LANG_GUI_ORIFICE3_FLOW;
  if (var == "LANG_GUI_ORIFICE3_PRESSURE") return language.LANG_GUI_ORIFICE3_PRESSURE;
  if (var == "LANG_GUI_ORIFICE4_FLOW") return language.LANG_GUI_ORIFICE4_FLOW;
  if (var == "LANG_GUI_ORIFICE4_PRESSURE") return language.LANG_GUI_ORIFICE4_PRESSURE;
  if (var == "LANG_GUI_ORIFICE5_FLOW") return language.LANG_GUI_ORIFICE5_FLOW;
  if (var == "LANG_GUI_ORIFICE5_PRESSURE") return language.LANG_GUI_ORIFICE5_PRESSURE;
  if (var == "LANG_GUI_ORIFICE6_FLOW") return language.LANG_GUI_ORIFICE6_FLOW;
  if (var == "LANG_GUI_ORIFICE6_PRESSURE") return language.LANG_GUI_ORIFICE6_PRESSURE;
  if (var == "LANG_GUI_API_SETTINGS") return language.LANG_GUI_API_SETTINGS;
  if (var == "LANG_GUI_API_DELIMITER") return language.LANG_GUI_API_DELIMITER;
  if (var == "LANG_GUI_SERIAL_BAUD_RATE") return language.LANG_GUI_SERIAL_BAUD_RATE;
  if (var == "LANG_GUI_CALIBRATION_DATA") return language.LANG_GUI_CALIBRATION_DATA;
  if (var == "LANG_GUI_CAL_OFFSET") return language.LANG_GUI_CAL_OFFSET;
  if (var == "LANG_GUI_LEAK_TEST_BASELINE") return language.LANG_GUI_LEAK_TEST_BASELINE;
  if (var == "LANG_GUI_LEAK_TEST_OFFSET") return language.LANG_GUI_LEAK_TEST_OFFSET;
  if (var == "LANG_GUI_LEAK_TEST_BASELINE_REV") return language.LANG_GUI_LEAK_TEST_BASELINE_REV;
  if (var == "LANG_GUI_LEAK_TEST_OFFSET_REV") return language.LANG_GUI_LEAK_TEST_OFFSET_REV;
  if (var == "LANG_GUI_OVERWRITE") return language.LANG_GUI_OVERWRITE;
  if (var == "LANG_GUI_DATA_CAPTURE_SETTINGS") return language.LANG_GUI_DATA_CAPTURE_SETTINGS;
  if (var == "LANG_GUI_CAPTURE_DATATYPE") return language.LANG_GUI_CAPTURE_DATATYPE;
  if (var == "MAF_FLOW_UNIT") {

      if (status.mafUnits == KG_H) {
        return "kg/h";
      } else {
        return "mg/s";
      }
  }


  // Bench definitions for system status pane 
  if (strstr(config.bench_type, "MAF")!=NULL) {
    status.benchType = "MAF Style";
  } else if (strstr(config.bench_type, "ORIFICE")!=NULL) {
    status.benchType = "Orifice Style";
  } else if (strstr(config.bench_type, "VENTURI")!=NULL) {
    status.benchType = "Venturi Style";
  } else if (strstr(config.bench_type, "PITOT")!=NULL) {
    status.benchType = "Pitot Style";
  }

  // // Board definitions for system status pane
  // #if defined WEMOS_D1_R32                    
  //   status.boardType = "WEMOS_D1_R32";
  // #elif defined ARDUCAM_ESP32S
  //   status.boardType = "ARDUCAM_ESP32S";
  // #elif defined ESP32DUINO
  //   status.boardType = "ESP32DUINO";
  // #elif defined ESP32_WROVER_KIT 
  //   status.boardType = "ESP32_WROVER_KIT";
  // #else
  //   status.boardType = "CUSTOM_PIN_MAPPING";
  // #endif


  // Config Info
  if (var == "RELEASE") return RELEASE;
  if (var == "BUILD_NUMBER") return BUILD_NUMBER;
  if (var == "SPIFFS_MEM_SIZE") return String(status.spiffs_mem_size);
  if (var == "SPIFFS_MEM_USED") return String(status.spiffs_mem_used);
  if (var == "LOCAL_IP_ADDRESS") return String(status.local_ip_address);
  if (var == "HOSTNAME") return String(status.hostname);
  if (var == "UPTIME") return String(esp_timer_get_time()/1000);
  if (var == "BENCH_TYPE") return String(status.benchType);
  if (var == "BOARD_TYPE") return String(status.boardType);
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

  if (var == "PITOT_COLOUR"){
    if (calVal.pitot_cal_offset == 0) {
      return GUI_COLOUR_UNSET;
      _message.debugPrintf("pitot unset");
    } else {
      return GUI_COLOUR_SET;
      _message.debugPrintf("pitot set");
    }
  }

  if (var == "PDIFF_COLOUR"){
    if (calVal.pdiff_cal_offset == 0) {
      return GUI_COLOUR_UNSET;
      _message.debugPrintf("diff unset");
    } else {
      return GUI_COLOUR_SET;
      _message.debugPrintf("diff set");
    }
  }

  //Datagraph Max Val selected item
  if (var == "DATAGRAPH_MAX_0" && config.dataGraphMax == 0) return String("selected");
  if (var == "DATAGRAPH_MAX_1" && config.dataGraphMax == 1) return String("selected");
  if (var == "DATAGRAPH_MAX_2" && config.dataGraphMax == 2) return String("selected");
  if (var == "DATAGRAPH_MAX_3" && config.dataGraphMax == 3) return String("selected");


  // Datagraph Stuff
  extern struct ValveLiftData valveData;
  int maxval = 0;
  int maxcfm = 0;
  double scaleFactor = 0.0;

  // very rough cfm calculation from max mafdata (approx half of KG/h rate)
  if (status.mafUnits == MG_S) {
    maxcfm = (0.0036 * status.mafDataValMax) / 2; 
  } else {
    maxcfm = status.mafDataValMax / 2;
  }

  // Determine data graph flow axis scale
  // NOTE: currently 1000cfm is the largest flow that the graph will display. 
  // We could change scaling to be realtive to SVG height (surface currently fixed at 500)
  if ((maxcfm < 500) || (config.dataGraphMax == 1)) {
    maxval = 250;
    scaleFactor = 2;
  } else if ((maxcfm > 250 && maxcfm < 500) || (config.dataGraphMax == 2)) {
    maxval = 500;
    scaleFactor = 1;
  } else if ((maxcfm > 500)  || (config.dataGraphMax == 3)  || (config.dataGraphMax == 0)){
    maxval = 1000; 
    scaleFactor = 0.5;
  }



  // scale the data graph flow axis
  if (var == "flow1") return String(maxval / 10);
  if (var == "flow2") return String(maxval / 10 * 2);
  if (var == "flow3") return String(maxval / 10 * 3);
  if (var == "flow4") return String(maxval / 10 * 4);
  if (var == "flow5") return String(maxval / 10 * 5);
  if (var == "flow6") return String(maxval / 10 * 6);
  if (var == "flow7") return String(maxval / 10 * 7);
  if (var == "flow8") return String(maxval / 10 * 8);
  if (var == "flow9") return String(maxval / 10 * 9);
  if (var == "flow10") return String(maxval );

  // scale the datapoint values to fit the flow axis scale
  // NOTE: surface is 500 units high with zero at the bottom
  if (var == "LINE_DATA1") return String(500 - (valveData.LiftData1 * scaleFactor));
  if (var == "LINE_DATA2") return String(500 - (valveData.LiftData2 * scaleFactor));
  if (var == "LINE_DATA3") return String(500 - (valveData.LiftData3 * scaleFactor));
  if (var == "LINE_DATA4") return String(500 - (valveData.LiftData4 * scaleFactor));
  if (var == "LINE_DATA5") return String(500 - (valveData.LiftData5 * scaleFactor));
  if (var == "LINE_DATA6") return String(500 - (valveData.LiftData6 * scaleFactor));
  if (var == "LINE_DATA7") return String(500 - (valveData.LiftData7 * scaleFactor));
  if (var == "LINE_DATA8") return String(500 - (valveData.LiftData8 * scaleFactor));
  if (var == "LINE_DATA9") return String(500 - (valveData.LiftData9 * scaleFactor));
  if (var == "LINE_DATA10") return String(500 - (valveData.LiftData10 * scaleFactor));
  if (var == "LINE_DATA11") return String(500 - (valveData.LiftData11 * scaleFactor));
  if (var == "LINE_DATA12") return String(500 - (valveData.LiftData12 * scaleFactor));


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





  // if (var == "LINE_DATA") {

  //   extern struct ValveLiftData valveData;
  //   String attribs;
  //   char* lineData;

  //   attribs =  R"END(
  //     fill="none" stroke="#000" stroke-width="1" stroke-dasharray="4,1" opacity="0.5"
  //   )END" ;

  //   int point1 = 350-valveData.LiftData1;
  //   int point2 = 350-valveData.LiftData2;
  //   int point3 = 350-valveData.LiftData3;
  //   int point4 = 350-valveData.LiftData4;
  //   int point5 = 350-valveData.LiftData5;
  //   int point6 = 350-valveData.LiftData6;
  //   int point7 = 350-valveData.LiftData7;
  //   int point8 = 350-valveData.LiftData8;
  //   int point9 = 350-valveData.LiftData9;
  //   int point10 = 350-valveData.LiftData10;
  //   int point11 = 350-valveData.LiftData11;
  //   int point12 = 350-valveData.LiftData12;


  //   // sprintf (lineData, "<polyline %s points=\"100,0 150, %d 200,%d 250,%d 300,%d 350,%d 400,%d 450,%d 500,%d 550,%d 600,%d 650,%d 700,%d\"  />", attribs, point1,  point2,  point3,  point4,  point5,  point6,  point7,  point8,  point9,  point10,  point11,  point12 );
  //   sprintf (lineData, "<polyline %s points=\"100,0 150,%d 200,%d \"  />", attribs, point1, point2);


  //   return '<polyline fill="none" stroke="#000" stroke-width="1" stroke-dasharray="4, 1" points="100,350 150,340 200,315 250,301 300,280 350,249 400,245 450,242 500,238 550,230 600,222 650,220 700,218" />';


  //   // return String(lineData).c_str();
  // }




  // Orifice plates
  if (var == "ORIFICE1_FLOW_RATE") return String(config.orificeOneFlow);
  if (var == "ORIFICE1_TEST_PRESSURE") return String(config.orificeOneDepression);
  if (var == "ORIFICE2_FLOW_RATE") return String(config.orificeTwoFlow);
  if (var == "ORIFICE2_TEST_PRESSURE") return String(config.orificeTwoDepression);
  if (var == "ORIFICE3_FLOW_RATE") return String(config.orificeThreeFlow);
  if (var == "ORIFICE3_TEST_PRESSURE") return String(config.orificeThreeDepression);
  if (var == "ORIFICE4_FLOW_RATE") return String(config.orificeFourFlow);
  if (var == "ORIFICE4_TEST_PRESSURE") return String(config.orificeFourDepression);
  if (var == "ORIFICE5_FLOW_RATE") return String(config.orificeFiveFlow);
  if (var == "ORIFICE5_TEST_PRESSURE") return String(config.orificeFiveDepression);
  if (var == "ORIFICE6_FLOW_RATE") return String(config.orificeSixFlow);
  if (var == "ORIFICE6_TEST_PRESSURE") return String(config.orificeSixDepression);

  // Current orifice data
  if (var == "ACTIVE_ORIFICE") return String(status.activeOrifice);
  if (var == "ORIFICE_MAX_FLOW") return String(status.activeOrificeFlowRate);
  if (var == "ORIFICE_CALIBRATED_DEPRESSION") return String(status.activeOrificeTestPressure);


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

  // Update javascript template vars
  if (var == "FLOW_DECIMAL_LENGTH") return String(config.flow_decimal_length);
  if (var == "GEN_DECIMAL_LENGTH") return String(config.gen_decimal_length);

  // Rounding type
  if (var == "ROUNDING_TYPE_DROPDOWN"){
    if (strstr(String(config.rounding_type).c_str(), String("NONE").c_str())){
      return String( "<select name='ROUNDING_TYPE' class='config-select'><option value='NONE' selected>No Rounding</option><option value='INTEGER'>Integer</option><option value='HALF'>Half</option></select>");
    } else if (strstr(String(config.rounding_type).c_str(), String("INTEGER").c_str())) {
      return String( "<select name='ROUNDING_TYPE' class='config-select'><option value='NONE'>No Rounding</option><option value='INTEGER' selected>Integer</option><option value='HALF'>Half</option></select>");
    } else if (strstr(String(config.rounding_type).c_str(), String("HALF").c_str())){
      return String( "<select name='ROUNDING_TYPE' class='config-select'><option value='NONE'>No Rounding</option><option value='INTEGER'>Integer</option><option value='HALF' selected>Half</option></select>");
    }
  }


  if (var == "STD_ADJ_FLOW" ) {
    if (config.std_adj_flow == 1) {
      return String("Checked");
    }
  }

  if (var == "AFLOW_UNITS" && config.std_adj_flow == 0) return String("ACFM");
  if (var == "AFLOW_UNITS" && config.std_adj_flow == 1) return String("SCFM");
  

  // Reference standard type dropdown selected item
  if (var == "STD_REF_1" && config.standardReference == 1) return String("selected");
  if (var == "STD_REF_2" && config.standardReference == 2) return String("selected");
  if (var == "STD_REF_3" && config.standardReference == 3) return String("selected");
  if (var == "STD_REF_4" && config.standardReference == 4) return String("selected");
  if (var == "STD_REF_5" && config.standardReference == 5) return String("selected");


  if (var == "STD_REF"  || var == "STANDARD_FLOW") {
    // Standard reference
    switch (config.standardReference) {

      case ISO_1585:
        return String("ISO-1585");
      break;

      case ISA :
        return String("ISA");
      break;

      case ISO_13443:
        return String("ISO-13443");
      break;

      case ISO_5011:
        return String("ISO-5011");
      break;

      case ISO_2533:
        return String("ISO-2533");
      break;
    }
  }

  // Flow Decimal type
  if (var == "FLOW_DECIMAL_LENGTH_DROPDOWN"){
    if (strstr(String(config.flow_decimal_length).c_str(), String("0").c_str())){
      return String( "<select name='FLOW_DECIMAL_LENGTH' class='config-select'><option value='0' selected>1 Whole</option><option value='1'>0.1 Tenths</option><option value='2'>0.01 Hundredths </option></select>");
    } else if (strstr(String(config.flow_decimal_length).c_str(), String("1").c_str())) {
      return String( "<select name='FLOW_DECIMAL_LENGTH' class='config-select'><option value='0'>1 Whole</option><option value='1' selected>0.1 Tenths</option><option value='2'>0.01 Hundredths </option></select>");
    } else if (strstr(String(config.flow_decimal_length).c_str(), String("2").c_str())){
      return String( "<select name='FLOW_DECIMAL_LENGTH' class='config-select'><option value='0'>1 Whole</option><option value='1'>0.1 Tenths</option><option value='2' selected>0.01 Hundredths </option></select>");
    }
  }

  // General Decimal type
  if (var == "GEN_DECIMAL_LENGTH_DROPDOWN"){
    if (strstr(String(config.gen_decimal_length).c_str(), String("0").c_str())){
      return String( "<select name='GEN_DECIMAL_LENGTH' class='config-select'><option value='0' selected>1 Whole</option><option value='1'>0.1 Tenths</option><option value='2'>0.01 Hundredths </option></select>");
    } else if (strstr(String(config.gen_decimal_length).c_str(), String("1").c_str())) {
      return String( "<select name='GEN_DECIMAL_LENGTH' class='config-select'><option value='0>1 Whole</option><option value='1' selected>0.1 Tenths</option><option value='2'>0.01 Hundredths </option></select>");
    } else if (strstr(String(config.gen_decimal_length).c_str(), String("2").c_str())){
      return String( "<select name='GEN_DECIMAL_LENGTH' class='config-select'><option value='0'>1 Whole</option><option value='1'>0.1 Tenths</option><option value='2' selected>0.01 Hundredths </option></select>");
    }
  }

  // Data Filter type
  if (var == "DATA_FILTER_TYPE_DROPDOWN"){
    if (strstr(String(config.data_filter_type).c_str(), String("NONE").c_str())){
      return String( "<select name='DATA_FILTER_TYPE' class='config-select'><option value='NONE' selected>None</option><option value='MEDIAN'>Rolling Median</option><option value='AVERAGE'>Cyclic Average </option><option value='MODE'>Mode</option></select>");
    } else if (strstr(String(config.data_filter_type).c_str(), String("MEDIAN").c_str())) {
      return String( "<select name='DATA_FILTER_TYPE' class='config-select'><option value='NONE' >None</option><option value='MEDIAN' selected>Rolling Median</option><option value='AVERAGE'>Cyclic Average </option><option value='MODE'>Mode</option></select>");
    } else if (strstr(String(config.data_filter_type).c_str(), String("AVERAGE").c_str())){
      return String( "<select name='DATA_FILTER_TYPE' class='config-select'><option value='NONE'>None</option><option value='MEDIAN'>Rolling Median</option><option value='AVERAGE' selected>Cyclic Average </option><option value='MODE'>Mode</option></select>");
    } else if (strstr(String(config.data_filter_type).c_str(), String("MODE").c_str())) {
      return String( "<select name='DATA_FILTER_TYPE' class='config-select'><option value='NONE'>None</option><option value='MEDIAN'>Rolling Median</option><option value='AVERAGE'>Cyclic Average </option><option value='MODE' selected>Mode</option></select>");
    }
  }

  // Data Filter Settings
  if (var == "CONF_MIN_FLOW_RATE") return String(config.min_flow_rate);
  if (var == "CONF_MIN_BENCH_PRESSURE") return String(config.min_bench_pressure);
  if (var == "CONF_MAF_MIN_VOLTS") return String(config.maf_min_volts);
  if (var == "CONF_CYCLIC_AVERAGE_BUFFER") return String(config.cyc_av_buffer);

  // Bench Settings
  if (var == "CONF_MAF_HOUSING_DIAMETER") return String(config.maf_housing_diameter);
  if (var == "CONF_REFRESH_RATE") return String(config.refresh_rate);
  if (var == "ADJ_FLOW_DEPRESSION") return String(config.adj_flow_depression);
  if (var == "TEMP_UNIT") return String(config.temp_unit);
  
  // Temperature
  if (var == "TEMPERATURE_DROPDOWN"){
    if (strstr(String(config.temp_unit).c_str(), String("Celcius").c_str())){
      return String( "<select name='TEMP_UNIT' class='config-select' id='TEMP_UNIT'><option value='Celcius' selected>Celcius </option><option value='Farenheit'>Farenheit </option></select>");
    } else {
      return String("<select name='TEMP_UNIT' class='config-select' id='TEMP_UNIT'><option value='Celcius'>Celcius </option><option value='Farenheit' selected>Farenheit </option></select>");
    }
  }

  // Lift
  if (var == "VALVE_LIFT_INTERVAL") return String(config.valveLiftInterval);

  // Bench type
  if (var == "BENCH_TYPE_DROPDOWN"){
    if (strstr(String(config.bench_type).c_str(), String("MAF").c_str())){
      return String( "<select name='BENCH_TYPE' class='config-select'><option value='MAF' selected>MAF Style</option><option value='ORIFICE'>Orifice Style</option><option value='VENTURI'>Venturi Style </option><option value='PITOT'>Pitot Style</option></select>");
    } else if (strstr(String(config.bench_type).c_str(), String("ORIFICE").c_str())) {
      return String( "<select name='BENCH_TYPE' class='config-select'><option value='MAF'>MAF Style</option><option value='ORIFICE' selected>Orifice Style</option><option value='VENTURI'>Venturi Style </option><option value='PITOT'>Pitot Style</option></select>");
    } else if (strstr(String(config.bench_type).c_str(), String("VENTURI").c_str())){
      return String( "<select name='BENCH_TYPE' class='config-select'><option value='MAF'>MAF Style</option><option value='ORIFICE'>Orifice Style</option><option value='VENTURI' selected>Venturi Style </option><option value='PITOT'>Pitot Style</option></select>");
    } else if (strstr(String(config.bench_type).c_str(), String("PITOT").c_str())) {
      return String( "<select name='BENCH_TYPE' class='config-select'><option value='MAF'>MAF Style</option><option value='ORIFICE'>Orifice Style</option><option value='VENTURI'>Venturi Style </option><option value='PITOT' selected>Pitot Style</option></select>");
    }
  }
 
  // Calibration Settings
  if (var == "CONF_CAL_FLOW_RATE") return String(config.cal_flow_rate);
  if (var == "CONF_CAL_REF_PRESS") return String(config.cal_ref_press);

  // Calibration Data
  if (var == "FLOW_OFFSET") return String(calVal.flow_offset);
  if (var == "USER_OFFSET") return String(calVal.user_offset);
  if (var == "LEAK_CAL_BASELINE") return String(calVal.leak_cal_baseline);
  if (var == "LEAK_CAL_OFFSET") return String(calVal.leak_cal_offset);
  if (var == "LEAK_CAL_BASELINE_REV") return String(calVal.leak_cal_baseline_rev);
  if (var == "LEAK_CAL_OFFSET_REV") return String(calVal.leak_cal_offset_rev);

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
    // FILESYSTEM.end();
    return fileList;
  }

  return "";
}





/***********************************************************
 * @brief processLandingPage
 * @details Replaces template placeholders with variable values
 * @param &var HTML payload 
 * @note ~PLACEHOLDER_FORMAT~
 * @note using IF statements for this sucks but C++ switch statement cannot handle text operators
 ***/
String Webserver::processLandingPageTemplate(const String &var) {

  extern struct DeviceStatus status;

  if (var == "INDEX_STATUS") {
    if (!SPIFFS.exists("/index.html")) return String("index.html");
  }  
  
  if (var == "SETTINGS_STATUS") {
    if (!SPIFFS.exists("/settings.json")) return String("settings.json");
  }

  if (var == "PINS_STATUS" ) {
    if (status.pinsLoaded == false) return String("PINS_***.json");    
    // if (!SPIFFS.exists(status.pinsFilename)) return String("PINS_***.json");    
  }

  if (var == "MAF_STATUS" ) {
    if (status.mafLoaded == false) return String("MAF_***.json");    
  }

  if (var == "CONFIGURATION_STATUS") {
    if (!SPIFFS.exists("/configuration.json")) return String("configuration.json");
  }

  return "";

}





//***********************************************************
//***********************************************************
//***********************************************************
//***********************************************************
//***********************************************************
//***********************************************************
//***********************************************************
//***********************************************************
  // TODO SD File handling / Data recording
//***********************************************************
//***********************************************************
//***********************************************************
//***********************************************************
//***********************************************************
//***********************************************************
//***********************************************************

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

/*


From https://deguez07.medium.com/esp32-with-sd-card-modules-the-master-guide-5d391f6785d7

// File will be null/false if /parent directory doesn't exist
File file = SD.open("/parent/hello-sd.txt", FILE_WRITE);
if (!file) { // the /parent directory doesn't exist, hence the file can't be created}
// Perform operations on the file
file.close(); // Never forget to close!


bool didCreate = SD.mkdir("/test-dir");
bool didMove = SD.rename("/test-dir", "/test");
bool didRemoveDir = SD.rmdir("/test");
bool didRemoveFile = SD.remove("hello.txt");



SD.mkdir("/test-dir"); //true
SD.mkdir("/parent/test-dir"); //false


SD.mkdir("/parent"); // true
SD.mkdir("/parent/test-dir"); // true, now that /parent exists


SD.rename("/hello.txt", "/top/hello.txt"); // false, target path already exists
SD.rename("/hello-world.txt", "/hw.txt"); // false, /hello-world.txt doesn't exist
SD.rename("/hello.txt", "/top2/hello.txt"); // false, /top2 doesn't exist
SD.rename("/hello.txt", "/top/hello-world.txt"); // true, we're moving and renaming /hello.txt
SD.rename("/top", "/top2/top-mod"); // false, /top2 doesn't exist
SD.rename("/top", "/top-mod"); // true, we're renaming the /top directory to /top-mod



SD.rmdir("/parent"); false, /parent directory doesn't exist
SD.rmdir("/top-dir"); false, /top-dir contains a directory /nested-dir
SD.rmdir("/top-dir2"); false, /top-dir2 contains a file hello.txt
SD.rmdir("/top-dir/nested-dir"); true, /nested-dir can be deleted, it doesn't contain files or other directories
SD.rmdir("/top-dir"); true, /top-dir can NOW be deleted, as it doesn't contain sub-directories







*/







// /***********************************************************
//  * @brief read JSON file from SD card 
//  * @return JSON data
//  * @NOTE: https://wokwi.com/projects/323656763409695316
//  ***/
// StaticJsonDocument<1024> Webserver::getSDFile(String filename){

//   StaticJsonDocument<1024> dataJson;

//   // Example of reading file from SD card:
//   File textFile = SD.open("/wokwi.txt");
//   if (textFile) {
//     Serial.print("wokwi.txt: ");
//     while (textFile.available()) {
//       Serial.write(textFile.read());
//     }
//     textFile.close();
//   } else {
//     Serial.println("error opening wokwi.txt!");
//   }

//   return dataJson;

// }




// /***********************************************************
//  * @brief read file list from SD card 
//  * @return JSON formatted list
//  * 
//  ***/
// StaticJsonDocument<1024> Webserver::getSDFileList(String filename){

//   //TODO - not coded yet

//   StaticJsonDocument<1024> dataJson;

//   File root;

//   Serial.println("Files in the card:");
//   root = SD.open("/");
//   // printDirectory(root, 0);
//   Serial.println("");



//   File dir;
//   int numTabs = 10;


//   while (true) {

//     File entry =  dir.openNextFile();
//     if (! entry) {
//       // no more files
//       break;
//     }
//     for (uint8_t i = 0; i < numTabs; i++) {
//       Serial.print('\t');
//     }
//     Serial.print(entry.name());
//     if (entry.isDirectory()) {
//       Serial.println("/");
//       // printDirectory(entry, numTabs + 1);
//     } else {
//       // files have sizes, directories do not
//       Serial.print("\t\t");
//       Serial.println(entry.size(), DEC);
//     }
//     entry.close();
//   }

//   return dataJson;

// }


// /** 
//  * Writes the given [data] to file at the given [filePath] 
//  * @return True if the data operation completes without errors
//  */
// bool Webserver::writeToSDFile(const char* filePath, const char* data) {
//     File file = SD.open(filePath, FILE_WRITE);

//     // Check the file could be opened
//     if (!file) {
//         return false;
//     }

//     file.println(data);
//     file.close();

//     return true;
// }

// /**
//  * Appends the given [data] to the file at the given [filePath]
//  * @return True if data is appended without errors
//  */
// bool Webserver::appendToSDFile(const char* filePath, const char* data) {
//     File file = SD.open(filePath, FILE_APPEND);
//     if (!file) { return false; }

//     file.println(data);
//     file.close();
//     return true;
// }

// /** Returns the data of the file at the given [filePath] */
// const char* Webserver::readSDFile(const char* filePath) {
//     File file = SD.open(filePath, FILE_READ);
//     if (!file) { return ""; }

//     String data = file.readString();
//     file.close();
//     return data.c_str();
// }



// void Webserver::deleteFile(fs::FS &fs, const char * path){
//     Serial.printf("Deleting file: %s\n", path);
//     if(fs.remove(path)){
//         Serial.println("File deleted");
//     } else {
//         Serial.println("Delete failed");
//     }
// }






/* 
https://github.com/espressif/arduino-esp32/blob/master/libraries/SD/examples/SD_Test/SD_Test.ino
*/


/*
 * pin 1 - not used          |  Micro SD card     |
 * pin 2 - CS (SS)           |                   /
 * pin 3 - DI (MOSI)         |                  |__
 * pin 4 - VDD (3.3V)        |                    |
 * pin 5 - SCK (SCLK)        | 8 7 6 5 4 3 2 1   /
 * pin 6 - VSS (GND)         | ▄ ▄ ▄ ▄ ▄ ▄ ▄ ▄  /
 * pin 7 - DO (MISO)         | ▀ ▀ █ ▀ █ ▀ ▀ ▀ |
 * pin 8 - not used          |_________________|
 *                             ║ ║ ║ ║ ║ ║ ║ ║
 *                     ╔═══════╝ ║ ║ ║ ║ ║ ║ ╚═════════╗
 *                     ║         ║ ║ ║ ║ ║ ╚══════╗    ║
 *                     ║   ╔═════╝ ║ ║ ║ ╚═════╗  ║    ║
 * Connections for     ║   ║   ╔═══╩═║═║═══╗   ║  ║    ║
 * full-sized          ║   ║   ║   ╔═╝ ║   ║   ║  ║    ║
 * SD card             ║   ║   ║   ║   ║   ║   ║  ║    ║
 * Pin name         |  -  DO  VSS SCK VDD VSS DI CS    -  |
 * SD pin number    |  8   7   6   5   4   3   2   1   9 /
 *                  |                                  █/
 *                  |__▍___▊___█___█___█___█___█___█___/
 *
 * Note:  The SPI pins can be manually configured by using `SPI.begin(sck, miso, mosi, cs).`
 *        Alternatively, you can change the CS pin and use the other default settings by using `SD.begin(cs)`.
 *
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | SPI Pin Name | ESP8266 | ESP32 | ESP32‑S2 | ESP32‑S3 | ESP32‑C3 | ESP32‑C6 | ESP32‑H2 |
 * +==============+=========+=======+==========+==========+==========+==========+==========+
 * | CS (SS)      | GPIO15  | GPIO5 | GPIO34   | GPIO10   | GPIO7    | GPIO18   | GPIO0    |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | DI (MOSI)    | GPIO13  | GPIO23| GPIO35   | GPIO11   | GPIO6    | GPIO19   | GPIO25   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | DO (MISO)    | GPIO12  | GPIO19| GPIO37   | GPIO13   | GPIO5    | GPIO20   | GPIO11   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | SCK (SCLK)   | GPIO14  | GPIO18| GPIO36   | GPIO12   | GPIO4    | GPIO21   | GPIO10   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 *
 * For more info see file README.md in this library or on URL:
 * https://github.com/espressif/arduino-esp32/tree/master/libraries/SD
 */


void Webserver::listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void Webserver::createDir(fs::FS &fs, const char *path) {
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void Webserver::removeDir(fs::FS &fs, const char *path) {
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path)) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void Webserver::readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void Webserver::writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void Webserver::appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void Webserver::renameFile(fs::FS &fs, const char *path1, const char *path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void Webserver::deleteFile(fs::FS &fs, const char *path) {

  Messages _message;
  extern struct DeviceStatus status;
  extern struct DataHandler _data;

  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    _message.debugPrintf("deleting file: %s", path);   
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void Webserver::testFileIO(fs::FS &fs, const char *path) {
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if (file) {
    len = file.size();
    size_t flen = len;
    start = millis();
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes read for %lu ms\n", flen, end);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }

  file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for (i = 0; i < 2048; i++) {
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %lu ms\n", 2048 * 512, end);
  file.close();
}