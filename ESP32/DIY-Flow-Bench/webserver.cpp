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
#include <Preferences.h>

#include "datahandler.h"
#include "system.h"

#include "constants.h"
#include "structs.h"
#include "comms.h"

// #include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "webserver.h"

#include "calibration.h"
#include "sensors.h"

#include "hardware.h"
#include "messages.h"
#include "calculations.h"
#include "mafdata.h"

#include <sstream>

#include "publichtml.h"

using namespace std;

#define U_PART U_SPIFFS

// RTC_DATA_ATTR int bootCount; // flash mem


void Webserver::begin()
{
  extern struct BenchSettings settings;
  extern struct Language language;
  extern struct DeviceStatus status;

  int wifiStatusCode;

  server = new AsyncWebServer(80);
  events = new AsyncEventSource("/events");

  Messages _message;
  Calibration _calibration;
  DataHandler _data;
  PublicHTML _public_html;

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
      settings.debug_mode = true;
      request->send(200, "text/html", "{\"debug\":\"on\"}"); });

  server->on("/api/debug/off", HTTP_GET, [](AsyncWebServerRequest *request){
      Messages _message;
      _message.Handler(language.LANG_BLANK);
      _message.debugPrintf("Debug Mode Off\n");
      settings.debug_mode = false;
      request->send(200, "text/html", "{\"debug\":\"off\"}"); });

  server->on("/api/dev/on", HTTP_GET, [](AsyncWebServerRequest *request) {
      Messages _message;
      _message.Handler(language.LANG_DEV_MODE);
      _message.debugPrintf("Developer Mode On\n");
      settings.dev_mode = true;
      request->send(200, "text/html", "{\"dev\":\"on\"}"); });

  server->on("/api/dev/off", HTTP_GET, [](AsyncWebServerRequest *request){
      Messages _message;
      _message.Handler(language.LANG_BLANK);
      _message.debugPrintf("Developer Mode Off\n");
      settings.dev_mode = false;
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
      // request->send(200);
      // request->redirect("/?view=upload"); 
      },
      fileUpload);

  // Download request handler
  server->on("/api/file/download", HTTP_GET, [](AsyncWebServerRequest *request){              
      Messages _message;
      String downloadFilename = request->url();
      downloadFilename.remove(0,18); // Strip the file path (first 18 chars)
      _message.debugPrintf("Request Download File: %s \n", downloadFilename);
      request->send(SPIFFS, downloadFilename, String(), true); });

  // Firmware update handler
  server->on("/api/update", HTTP_POST, [](AsyncWebServerRequest *request){
    status.shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", status.shouldReboot?"<meta http-equiv=\"refresh\" content=\"3; url=/\" />Rebooting... If the page does not automatically refresh, please click <a href=\"/\">HERE</a>":"FIRMWARE UPDATE FAILED!");
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
        // if (_data.checkSubstring(fileToDelete.c_str(), status.indexFilename.c_str())) status.GUIexists = false;
        // if (fileToDelete == "/index.html"){
        // If we delete a system file, send user back to boot loop to upload missing file
        if (fileToDelete == status.indexFilename || fileToDelete == status.pinsFilename || fileToDelete == status.mafFilename){
          request->send_P(200, "text/html", language.LANG_INDEX_HTML, processLandingPageTemplate); 
          _data.bootLoop();
          // request->redirect("/");
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


  // Set flow to MAF flow
  server->on("/api/flow/mafflow", HTTP_GET, [](AsyncWebServerRequest *request){
    extern struct SensorData sensorVal;
    Messages _message;
    _message.debugPrintf("/api/flow/mafflow \n");
    sensorVal.flowtile = MAFFLOW_TILE;
    request->send(200);
  });

  // Set flow to flow
  server->on("/api/flow/flow", HTTP_GET, [](AsyncWebServerRequest *request){
    extern struct SensorData sensorVal;
    Messages _message;
    _message.debugPrintf("/api/flow/flow \n");
    sensorVal.flowtile = ACFM_TILE;
    request->send(200);
  });

  // Set flow to Aflow
  server->on("/api/flow/aflow", HTTP_GET, [](AsyncWebServerRequest *request){
    extern struct SensorData sensorVal;
    Messages _message;
    _message.debugPrintf("/api/flow/aflow \n");
    sensorVal.flowtile = ADJCFM_TILE;
    request->send(200);
  });

  // Set flow to Standard flow
  server->on("/api/flow/sflow", HTTP_GET, [](AsyncWebServerRequest *request){
    extern struct SensorData sensorVal;
    Messages _message;
    _message.debugPrintf("/api/flow/sflow \n");
    sensorVal.flowtile = SCFM_TILE;
    request->send(200);
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
    request->send(200, "text/html", String(_data.buildIndexSSEJsonData()).c_str());
  });


  // Save user Flow Diff target
  server->on("/api/saveflowtarget", HTTP_POST, parseUserFlowTargetForm);
  
  // Save Settings Form
  server->on("/api/savesettings", HTTP_POST, saveSettingsForm); 

  // Save Configuration Form
  server->on("/api/saveconfig", HTTP_POST, saveConfigurationForm);

  // Save Pins Form
  server->on("/api/savepins", HTTP_POST, savePinsForm);

  // Save Calibration Form
  server->on("/api/savecalibration", HTTP_POST, saveCalibrationForm);

  // Save Lift Data Form
  server->on("/api/saveliftdata", HTTP_POST, parseLiftDataForm);

  // Parse Orifice Form
  server->on("/api/saveorifice", HTTP_POST, parseOrificeForm);

  // Clear Lift Data
  server->on("/api/clearLiftData", HTTP_POST, [](AsyncWebServerRequest *request) {
    DataHandler _data;
    _data.clearLiftDataFile;
  });  


  // HTML server responses

  // index.html
  server->rewrite("/index.html", "/");

  // Basic Upload Page (in case of file error - does not require working GUI)
  server->on("/upload", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/html", "<form method='POST' action='/api/file/upload' enctype='multipart/form-data'><input type='file' name='upload'><input type='submit' value='Upload'></form>");
      },
      fileUpload);

  // Simple Firmware Update Form - does not require working GUI)
  server->on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", language.LANG_INDEX_HTML, processLandingPageTemplate); 
    request->send(200, "text/html", "<form method='POST' action='/api/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
  });

  // Favicon request handler (icon hex dump is in constants.h)
  server->on("/favicon.ico", HTTP_ANY, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "image/x-icon", favicon_ico_gz, favicon_ico_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  // CSS request handler
  server->on("/style.css", HTTP_ANY, [](AsyncWebServerRequest *request){
        PublicHTML _public_html;
        AsyncResponseStream *response = request->beginResponseStream("text/css");
        response->print(_public_html.css().c_str());
        request->send(response);
      });

  // Javascript.js request handler
  server->on("/index.js", HTTP_ANY, [](AsyncWebServerRequest *request){
        PublicHTML _public_html;
        AsyncResponseStream *response = request->beginResponseStream("text/javascript");
        response->print(_public_html.indexJs().c_str());
        request->send(response);
      });
  
  server->on("/settings.js", HTTP_ANY, [](AsyncWebServerRequest *request){
        PublicHTML _public_html;
        AsyncResponseStream *response = request->beginResponseStream("text/javascript");
        response->print(_public_html.settingsJs().c_str());
        request->send(response);
      });
  
  server->on("/config.js", HTTP_ANY, [](AsyncWebServerRequest *request){
        PublicHTML _public_html;
        AsyncResponseStream *response = request->beginResponseStream("text/javascript");
        response->print(_public_html.configJs().c_str());
        request->send(response);
      });
  
  server->on("/mimic.js", HTTP_ANY, [](AsyncWebServerRequest *request){
        PublicHTML _public_html;
        AsyncResponseStream *response = request->beginResponseStream("text/javascript");
        response->print(_public_html.mimicJs().c_str());
        request->send(response);
      });
  
  server->on("/data.js", HTTP_ANY, [](AsyncWebServerRequest *request){
        PublicHTML _public_html;
        AsyncResponseStream *response = request->beginResponseStream("text/javascript");
        response->print(_public_html.dataJs().c_str());
        request->send(response);
      });
  
  // Settings page request handler
  server->on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
        PublicHTML _public_html;
        status.GUIpage = SETTINGS_PAGE;
        request->send_P(200, "text/html", _public_html.settingsPage().c_str(), processSettingsPageTemplate); 
      });

  // Data page request handler
  server->on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
        PublicHTML _public_html;
        status.GUIpage = DATA_PAGE;
        request->send_P(200, "text/html", _public_html.dataPage().c_str(), processDatagraphPageTemplate); 
      });

  // Configuration page request handler
  server->on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
        PublicHTML _public_html;
        status.GUIpage = CONFIG_PAGE;
        request->send_P(200, "text/html", _public_html.configPage().c_str(), processConfigPageTemplate); 
      });

  // Pins page request handler
  server->on("/pins", HTTP_GET, [](AsyncWebServerRequest *request){
        PublicHTML _public_html;
        status.GUIpage = PINS_PAGE;
        request->send_P(200, "text/html", _public_html.pinsPage().c_str(), processPinsPageTemplate); 
      });

  // Mimic page request handler
  server->on("/mimic", HTTP_GET, [](AsyncWebServerRequest *request){
        PublicHTML _public_html;
        status.GUIpage = MIMIC_PAGE;
        request->send_P(200, "text/html", _public_html.mimicPage().c_str(), processMimicPageTemplate); 
      });

  // // Index page request handler
  // server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  //       PublicHTML _public_html;
  //       request->send_P(200, "text/html", _public_html.indexPage().c_str(), processIndexPageTemplate); 
  //     });


  // Index page request handler
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        PublicHTML _public_html;
        status.GUIpage = INDEX_PAGE;
        request->send_P(200, "text/html", _public_html.indexPage().c_str(),  processIndexPageTemplate);
      });


  server->onFileUpload(fileUpload);
  server->addHandler(events);
  server->begin();

  _message.Handler(language.LANG_SERVER_RUNNING);
  _message.serialPrintf("Server Running \n");

  status.webserverIsRunning = true;
}





/***********************************************************
 * @brief Process File Upload
 * @note Redirects browser back to Upload modal unless upload is index file
 ***/
void Webserver::fileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{

  Messages _message;
  DataHandler _data;
  String redirectURL;
  extern struct DeviceStatus status;
  extern struct Language language;
  extern struct DeviceStatus status;

  bool upload_error = false;
  int file_size = 0;

  if (!filename.startsWith("/")) filename = "/" + filename;

  uint32_t freespace = SPIFFS.totalBytes() - SPIFFS.usedBytes();

//  if (!index && !upload_error)  {
  if (!index)  {
    // _message.debugPrintf("UploadStart: %s \n", filename.c_str());
    // open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open(filename, "w");
  }

  if (len)  {
    file_size += len;
    if (file_size > freespace)    {
      // _message.Handler(language.LANG_UPLOAD_FAILED_NO_SPACE);
      _message.debugPrintf("Upload failed, no Space: %s \n", freespace);
      upload_error = true;
    }    else    {
      // _message.debugPrintf("Writing file: '%s' index=%u len=%u \n", filename.c_str(), index, len);
      // stream the incoming chunk to the opened file
      request->_tempFile.write(data, len);
    }
  } 

  if (final)  {
    // _message.debugPrintf("Upload Complete: %s, %u bytes\n", filename.c_str(), file_size);
    request->_tempFile.close();
  }


  // if (!filename.startsWith("/MAF")) status.mafLoaded = true;
  // if (!filename.startsWith("/configuration")) status.configLoaded = true;
  // if (!filename.startsWith("/PINS")) status.pinsLoaded = true;
  // if (!filename.startsWith("/V{RELEASE}")) status.GUIexists = true;
  
  // _message.Handler(language.LANG_FILE_UPLOADED);

  request->redirect("/");
  
}




/***********************************************************
 * @brief saveConfigurationForm
 * @details Parses calibration form post vars and stores into NVM memory
 * @note Loads data into structs
 ***/
void Webserver::saveConfigurationForm(AsyncWebServerRequest *request)
{

  Messages _message;
  DataHandler _data;
  Preferences _prefs;

  AsyncWebParameter* p;

  _prefs.begin("config");

  int params = request->params();

  _message.debugPrintf("Saving Configuration... \n");

  // Update Config Vars
  for(int i=0;i<params;i++){
    p = request->getParam(i);
      if (p->name().startsWith("i")) {
        _prefs.putInt(p->name().c_str(), p->value().toInt());
      } else if (p->name().startsWith("b")) {
        _prefs.putBool(p->name().c_str(), p->value().toInt());
      } else if (p->name().startsWith("d")) {
        _prefs.putDouble(p->name().c_str(), p->value().toDouble());
      } else if (p->name().startsWith("f")) {
        _prefs.putFloat(p->name().c_str(), p->value().toFloat());
      } else if (p->name().startsWith("s")) {
        _prefs.putString(p->name().c_str(), p->value().c_str());
      }
      _message.verbosePrintf("Writing Configuration: Key: %s  Value: %s \n", p->name().c_str(), p->value().c_str());
  }

  _prefs.end();
  _data.loadConfig();
  request->redirect("/");
}




/***********************************************************
 * @brief saveSettingsForm
 * @details Parses settings form post vars and stores into NVM memory
 * @note Loads data into structs
 ***/
void Webserver::saveSettingsForm(AsyncWebServerRequest *request)
{

  Messages _message;
  DataHandler _data;
  Preferences _prefs;

  AsyncWebParameter* p;

  _prefs.begin("settings");

  int params = request->params();

  _message.debugPrintf("Saving Settings to NVM... \n");

  // Update Settings Vars
  for(int i=0;i<params;i++){
    p = request->getParam(i);
      if (p->name().startsWith("i")) {
        _prefs.putInt(p->name().c_str(), p->value().toInt());
      } else if (p->name().startsWith("b")) {
        _prefs.putBool(p->name().c_str(), p->value().toInt());
      } else if (p->name().startsWith("d")) {
        _prefs.putDouble(p->name().c_str(), p->value().toDouble());
      } else if (p->name().startsWith("f")) {
        _prefs.putFloat(p->name().c_str(), p->value().toFloat());
      } else if (p->name().startsWith("s")) {
        _prefs.putString(p->name().c_str(), p->value().c_str());
      }

      _message.verbosePrintf("Writing Setting: Key: %s  Value: %s \n", p->name().c_str(), p->value().c_str());
  }

  _prefs.end();
  _data.loadSettings();
  request->redirect("/");
}








/***********************************************************
 * @brief savePinsForm
 * @details Parses pins form post vars and stores into NVM memory
 * @note Loads data into structs
 ***/
void Webserver::savePinsForm(AsyncWebServerRequest *request)
{

  Messages _message;
  DataHandler _data;
  Hardware _hardware;
  Preferences _prefs;

  AsyncWebParameter* p;

  _prefs.begin("pins");

  int params = request->params();

  _message.debugPrintf("Saving Pins data to NVM... \n");

  // Update pins
  for(int i=0;i<params;i++){
    p = request->getParam(i);
    _prefs.putInt(p->name().c_str(), p->value().toInt());
    _message.verbosePrintf("Writing Pins data: Pin: %s  Value: %u \n", p->name().c_str(), p->value().toInt());
  }

  _prefs.end();
  _hardware.loadPinsData();
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
void Webserver::saveCalibrationForm(AsyncWebServerRequest *request)
{

  Calibration _calibrate;
  Messages _message;
  Preferences _prefs;

  AsyncWebParameter* p;

  extern struct CalibrationData calVal;

  _prefs.begin("calibration");

  int params = request->params();

  _message.debugPrintf("Parsing Calibration Form Data... \n");

  for(int i=0;i<params;i++){
    p = request->getParam(i);
    _prefs.putDouble(p->name().c_str(), p->value().toDouble());
  }

  _prefs.end();
  _calibrate.loadCalibrationData();
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
  // calVal.leak_cal_offset = calData["LEAK_OFFSET"].as<double>();
  // calVal.leak_cal_offset_rev = calData["LEAK_OFFSET_REV"].as<double>();
  // calVal.leak_cal_baseline= calData["LEAK_BASE"].as<double>();
  // calVal.leak_cal_baseline_rev = calData["LEAK_BASE_REV"].as<double>();

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
void Webserver::parseLiftDataForm(AsyncWebServerRequest *request){

  Messages _message;
  DataHandler _data;
  Webserver _webserver;

  StaticJsonDocument<LIFT_DATA_JSON_SIZE> liftData;
  extern struct SensorData sensorVal;
  extern struct ValveLiftData valveData;
  extern struct BenchSettings settings;
  
  String jsonString;
  String liftPoint;
  int switchval;
  char *end;
  int params = request->params();
  const char* PARAM_INPUT = "lift-data";
  double flowValue;

  _message.debugPrintf("Saving Lift Data....\n");

  // TEST - Print POST vars to serial
  // for(int i=0;i<params;i++){
  //   AsyncWebParameter* p = request->getParam(i);
  //   if(p->isFile()){ //p->isPost() is also true
  //     Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
  //   } else if(p->isPost()){
  //     Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
  //   } else {
  //     Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
  //   }
  // }

  // Convert POST vars to JSON 
  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
        // get selected radio button and store it (radio button example from https://www.electrorules.com/esp32-web-server-control-stepper-motor-html-form/)
        if (p->name() == PARAM_INPUT) {
          liftPoint = p->value();
        }
  }

  // Get flow value based on capture Datatype
  switch (settings.data_capture_datatype) {

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
    
    case RAW_MASS:
      flowValue = sensorVal.FlowKGH;
    break;
        
  }

  // Update lift point data
  switchval = stoi(liftPoint.c_str()); // convert std::str to int

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

  Preferences _prefs;
  _prefs.begin("liftData");

  _prefs.putDouble("LIFTDATA1", valveData.LiftData1);
  _prefs.putDouble("LIFTDATA2", valveData.LiftData2);
  _prefs.putDouble("LIFTDATA3", valveData.LiftData3);
  _prefs.putDouble("LIFTDATA4", valveData.LiftData4);
  _prefs.putDouble("LIFTDATA5", valveData.LiftData5);
  _prefs.putDouble("LIFTDATA6", valveData.LiftData6);
  _prefs.putDouble("LIFTDATA7", valveData.LiftData7);
  _prefs.putDouble("LIFTDATA8", valveData.LiftData8);
  _prefs.putDouble("LIFTDATA9", valveData.LiftData9);
  _prefs.putDouble("LIFTDATA10", valveData.LiftData10);
  _prefs.putDouble("LIFTDATA11", valveData.LiftData11);
  _prefs.putDouble("LIFTDATA12", valveData.LiftData12);

  _prefs.end();
    
  request->send(200);

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
 * @brief parseLanguage Template Vars
 * @details Replaces template placeholders with variable values
 * @param &var HTML payload 
 * @note %PLACEHOLDER_FORMAT%
 * @note using IF statements for this sucks but C++ switch statement cannot handle text operators
 ***/
String Webserver::processLanguageTemplateVars(const String &var) {

  extern struct Language language;

  // Translate GUI
  // Note language struct is overwritten when language.json file is present
  if (var == "LANG_GUI_SELECT_LIFT_VAL_BEFORE_CAPTURE") return language.LANG_GUI_SELECT_LIFT_VAL_BEFORE_CAPTURE;
  if (var == "LANG_GUI_LIFT_VAL") return language.LANG_GUI_LIFT_VAL;
  if (var == "LANG_GUI_LIFT_CAPTURE") return language.LANG_GUI_LIFT_CAPTURE;
  if (var == "LANG_GUI_UPLOAD_FIRMWARE_BINARY") return language.LANG_GUI_UPLOAD_FIRMWARE_BINARY;
  if (var == "LANG_GUI_FIRMWARE_UPDATE") return language.LANG_GUI_FIRMWARE_UPDATE;
  if (var == "LANG_GUI_USER_FLOW_TARGET_VAL") return language.LANG_GUI_USER_FLOW_TARGET_VAL;
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
  if (var == "LANG_GUI_SENS_CONFIG") return language.LANG_GUI_SENS_CONFIG;
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
  if (var == "LANG_GUI_SETTINGS") return language.LANG_GUI_SETTINGS;
  if (var == "LANG_GUI_PINS") return language.LANG_GUI_PINS;
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
  if (var == "LANG_GUI_MAF_DIAMETER") return language.LANG_GUI_MAF_DIAMETER;
  if (var == "LANG_GUI_REFRESH_RATE") return language.LANG_GUI_REFRESH_RATE;
  if (var == "LANG_GUI_TEMPERATURE_UNIT") return language.LANG_GUI_TEMPERATURE_UNIT;
  if (var == "LANG_GUI_LIFT_INTERVAL") return language.LANG_GUI_LIFT_INTERVAL;
  if (var == "LANG_GUI_DATA_GRAPH_MAX_VAL") return language.LANG_GUI_DATA_GRAPH_MAX_VAL;
  if (var == "LANG_GUI_AUTO") return language.LANG_GUI_AUTO;
  if (var == "LANG_GUI_RESOLUTION_AND_ACCURACY") return language.LANG_GUI_RESOLUTION_AND_ACCURACY;
  if (var == "LANG_GUI_FLOW_VAL_ROUNDING") return language.LANG_GUI_FLOW_VAL_ROUNDING;
  if (var == "LANG_GUI_FLOW_DECIMAL_ACCURACY") return language.LANG_GUI_FLOW_DECIMAL_ACCURACY;
  if (var == "LANG_GUI_GEN_DECIMAL_ACCURACY") return language.LANG_GUI_GEN_DECIMAL_ACCURACY;
  if (var == "LANG_GUI_DATA_FILTERS") return language.LANG_GUI_DATA_FILTERS;
  if (var == "LANG_GUI_DATA_FLTR_TYP") return language.LANG_GUI_DATA_FLTR_TYP;
  if (var == "LANG_GUI_MIN_FLOW_RATE") return language.LANG_GUI_MIN_FLOW_RATE;
  if (var == "LANG_GUI_MIN_PRESSURE") return language.LANG_GUI_MIN_PRESSURE;
  if (var == "LANG_GUI_MAF_MIN_VOLTS") return language.LANG_GUI_MAF_MIN_VOLTS;
  if (var == "LANG_GUI_CYCLIC_AVERAGE_BUFFER") return language.LANG_GUI_CYCLIC_AVERAGE_BUFFER;
  if (var == "LANG_GUI_CONVERSION_SETTINGS") return language.LANG_GUI_CONVERSION_SETTINGS;
  if (var == "LANG_GUI_ADJ_FLOW_DEP") return language.LANG_GUI_ADJ_FLOW_DEP;
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
  if (var == "LANG_GUI_SERIAL_BAUD") return language.LANG_GUI_SERIAL_BAUD;
  if (var == "LANG_GUI_CALIBRATION_DATA") return language.LANG_GUI_CALIBRATION_DATA;
  if (var == "LANG_GUI_CAL_OFFSET") return language.LANG_GUI_CAL_OFFSET;
  if (var == "LANG_GUI_LEAK_TEST_BASELINE") return language.LANG_GUI_LEAK_TEST_BASELINE;
  if (var == "LANG_GUI_LEAK_TEST_OFFSET") return language.LANG_GUI_LEAK_TEST_OFFSET;
  if (var == "LANG_GUI_LEAK_TEST_BASELINE_REV") return language.LANG_GUI_LEAK_TEST_BASELINE_REV;
  if (var == "LANG_GUI_LEAK_TEST_OFFSET_REV") return language.LANG_GUI_LEAK_TEST_OFFSET_REV;
  if (var == "LANG_GUI_OVERWRITE") return language.LANG_GUI_OVERWRITE;
  if (var == "LANG_GUI_DATA_CAPTURE_SETTINGS") return language.LANG_GUI_DATA_CAPTURE_SETTINGS;
  if (var == "LANG_GUI_CAPTURE_DATATYPE") return language.LANG_GUI_CAPTURE_DATATYPE;
  if (var == "LANG_GUI_MAF_VOLTS") return language.LANG_GUI_MAF_VOLTS;
  if (var == "LANG_GUI_PREF_VOLTS") return language.LANG_GUI_PREF_VOLTS;
  if (var == "LANG_GUI_PDIFF_VOLTS") return language.LANG_GUI_PDIFF_VOLTS;
  if (var == "LANG_GUI_PITOT_VOLTS") return language.LANG_GUI_PITOT_VOLTS;
  if (var == "LANG_GUI_MAF_TYPE") return language.LANG_GUI_MAF_TYPE;
  if (var == "LANG_GUI_MIMIC") return language.LANG_GUI_MIMIC;

  return var;
  
}






/***********************************************************
 * @brief processindexTemplate
 * @details Replaces template placeholders with variable values
 * @param &var HTML payload 
 * @note %PLACEHOLDER_FORMAT%
 * @note using IF statements for this sucks but C++ switch statement cannot handle text operators
 ***/
String Webserver::processIndexPageTemplate(const String &var) {

  extern struct DeviceStatus status;
  extern struct BenchSettings settings;
  extern struct CalibrationData calVal;
  extern struct Language language;
  extern struct Configuration config;
  extern struct ValveLiftData valveData;

  Calculations _calculations;
  Messages _message;

  MafData _maf(config.iMAF_SENS_TYP);

  // Process language vars
  String langVar = processLanguageTemplateVars(var);
  if ( langVar != var) return langVar;


  
  if (var == "MAF_FLOW_UNIT") {
  	const auto unitKG_H = std::string("KG_H");
	  const auto mafUnit = std::string(status.mafUnits);
	  bool mafUnitIsKG_H = mafUnit.find(unitKG_H) != string::npos;

    if (mafUnitIsKG_H) {
      return "kg/h";
    } else {
      return "mg/s";
    }
  }

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


  // Current orifice data
  if (var == "ACTIVE_ORIFICE") return String(status.activeOrifice);
  if (var == "ORIFICE_MAX_FLOW") return String(status.activeOrificeFlowRate);
  if (var == "ORIFICE_CALIBRATED_DEPRESSION") return String(status.activeOrificeTestPressure);


  // Temp Unit
  if (var == "iTEMP_UNIT") {
    if (settings.temp_unit == CELCIUS) {
      return String("Celcius");
    } else {
      return String("Farenheit");
    }
  } 

  // Adj Flow Unit
  if (var == "AFLOW_UNITS" && settings.std_adj_flow == 1) return String("ACFM");
  if (var == "AFLOW_UNITS" && settings.std_adj_flow == 2) return String("SCFM");



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




  return "";
}





/***********************************************************
 * @brief processLandingPage
 * @details Replaces template placeholders with variable values
 * @param &var HTML payload 
 * @note ~PLACEHOLDER_FORMAT~
 ***/
String Webserver::processLandingPageTemplate(const String &var) {

  extern struct DeviceStatus status;

  // if (var == "INDEX_STATUS") {
  //   if (status.GUIexists == false) return String("<a href='https://github.com/DeeEmm/DIY-Flow-Bench/tree/master/ESP32/DIY-Flow-Bench/release/' target='_BLANK'>index.html</a>");    
  //   // if (!SPIFFS.exists(status.indexFilename)) return String("index.html");
  // }  
  
  // if (var == "SETTINGS_STATUS") {
  //   if (!SPIFFS.exists("/settings.json")) return String("<a href='https://github.com/DeeEmm/DIY-Flow-Bench/tree/master/ESP32/DIY-Flow-Bench/release/' target='_BLANK'>settings.json</a>");
  // }

  if (var == "PINS_STATUS" ) {
    if (status.pinsLoaded == false) return String("<a href='https://github.com/DeeEmm/DIY-Flow-Bench/tree/master/ESP32/DIY-Flow-Bench/pins/' target='_BLANK'>pins.json</a>");    
    // if (!SPIFFS.exists(status.pinsFilename)) return String("PINS_***.json");    
  }

  if (var == "MAF_STATUS" ) {
    if (status.mafLoaded == false) return String("<a href='https://github.com/DeeEmm/DIY-Flow-Bench/tree/master/ESP32/DIY-Flow-Bench/mafData/' target='_BLANK'>maf.json</a>");    
  }

  if (var == "CONFIGURATION_STATUS") {
    if (!SPIFFS.exists("/configuration.json")) return String("<a href='https://github.com/DeeEmm/DIY-Flow-Bench/tree/master/ESP32/DIY-Flow-Bench/' target='_BLANK'>configuration.json</a>");
  }

  return "";

}




/***********************************************************
 * @brief processSettingsPageTemplate
 * @details Replaces template placeholders with variable values
 * @param &var HTML payload 
 * @note ~PLACEHOLDER_FORMAT~
 ***/
String Webserver::processSettingsPageTemplate(const String &var) {

  extern struct DeviceStatus status;
  extern struct BenchSettings settings;
  extern struct Configuration config;
  extern struct CalibrationData calVal;
  
  Calculations _calculations;

  MafData _maf(config.iMAF_SENS_TYP);

  // Process language vars
  String langVar = processLanguageTemplateVars(var);
  if ( langVar != var) return langVar;



  // Bench definitions for system status pane 
  switch (settings.bench_type){

    case MAF_BENCH:
      status.benchType = "MAF Style";
    break;

    case ORIFICE_BENCH:
      status.benchType = "Orifice Style";
    break;

    case VENTURI_BENCH:
      status.benchType = "Venturi Style";
    break;

    case PITOT_BENCH:
      status.benchType = "Pitot Style";
    break;

  }


  // NOTE Build Vars are added to environment by user_actions.py at compile time
  if (var == "RELEASE") return RELEASE;
  if (var == "BUILD_NUMBER") return BUILD_NUMBER;
  if (var == "GUI_BUILD_NUMBER") return GUI_BUILD_NUMBER; 

  // Config Info
  if (var == "SPIFFS_MEM_SIZE") return String(_calculations.byteDecode(status.spiffs_mem_size));
  if (var == "SPIFFS_MEM_USED") return String(_calculations.byteDecode(status.spiffs_mem_used));
  if (var == "LOCAL_IP_ADDRESS") return String(status.local_ip_address);
  if (var == "sHOSTNAME") return String(status.hostname);
  if (var == "UPTIME") return String(esp_timer_get_time()/1000);
  if (var == "iBENCH_TYPE") return String(status.benchType);
  if (var == "BOARD_TYPE") return String(status.boardType);
  if (var == "BOOT_TIME") return String(status.boot_time);

  // Sensor Values
  if (var == "MAF_SENSOR") return String(status.mafSensor);
  if (var == "MAF_LINK") return _maf.getMafLink();
  if (var == "MAF_TYPE") return _maf.getType();
  if (var == "PREF_SENSOR") return String(status.prefSensor);
  if (var == "TEMP_SENSOR") return String(status.tempSensor);
  if (var == "RELH_SENSOR") return String(status.relhSensor);
  if (var == "BARO_SENSOR") return String(status.baroSensor);
  if (var == "PITOT_SENSOR") return String(status.pitotSensor);
  if (var == "PDIFF_SENSOR") return String(status.pdiffSensor);
  if (var == "STATUS_MESSAGE") return String(status.statusMessage);


  //Datagraph Max Val selected item
  if (var == "iDATAGRAPH_MAX_0" && settings.dataGraphMax == 0) return String("selected");
  if (var == "iDATAGRAPH_MAX_1" && settings.dataGraphMax == 1) return String("selected");
  if (var == "iDATAGRAPH_MAX_2" && settings.dataGraphMax == 2) return String("selected");
  if (var == "iDATAGRAPH_MAX_3" && settings.dataGraphMax == 3) return String("selected");


  // Orifice plates
  if (var == "dORIFICE1_FLOW") return String(settings.orificeOneFlow);
  if (var == "dORIFICE1_PRESS") return String(settings.orificeOneDepression);
  if (var == "dORIFICE2_FLOW") return String(settings.orificeTwoFlow);
  if (var == "dORIFICE2_PRESS") return String(settings.orificeTwoDepression);
  if (var == "dORIFICE3_FLOW") return String(settings.orificeThreeFlow);
  if (var == "dORIFICE3_PRESS") return String(settings.orificeThreeDepression);
  if (var == "dORIFICE4_FLOW") return String(settings.orificeFourFlow);
  if (var == "dORIFICE4_PRESS") return String(settings.orificeFourDepression);
  if (var == "dORIFICE5_FLOW") return String(settings.orificeFiveFlow);
  if (var == "dORIFICE5_PRESS") return String(settings.orificeFiveDepression);
  if (var == "dORIFICE6_FLOW") return String(settings.orificeSixFlow);
  if (var == "dORIFICE6_PRESS") return String(settings.orificeSixDepression);


   // Wifi Settings
  if (var == "sWIFI_SSID") return settings.wifi_ssid;
  if (var == "sWIFI_PSWD") return settings.wifi_pswd;
  if (var == "sWIFI_AP_SSID") return settings.wifi_ap_ssid;
  if (var == "sWIFI_AP_PSWD") return settings.wifi_ap_pswd;
  if (var == "sHOSTNAME") return settings.hostname;
  if (var == "iWIFI_TIMEOUT") return String(settings.wifi_timeout);

  // API Settings
  if (var == "sAPI_DELIM") return settings.api_delim;
  if (var == "iSERIAL_BAUD") return String(settings.serial_baud_rate);

  // Decinal accuracy
  if (var == "iFLOW_DECI_ACC") return String(settings.flow_decimal_length);
  if (var == "iGEN_DECI_ACC") return String(settings.gen_decimal_length);

  // Reference standard type dropdown selected item
  if (var == "iSTD_REF_1" && settings.standardReference == 1) return String("selected");
  if (var == "iSTD_REF_2" && settings.standardReference == 2) return String("selected");
  if (var == "iSTD_REF_3" && settings.standardReference == 3) return String("selected");
  if (var == "iSTD_REF_4" && settings.standardReference == 4) return String("selected");
  if (var == "iSTD_REF_5" && settings.standardReference == 5) return String("selected");

  if (var == "iSTD_ADJ_FLOW_1" && settings.std_adj_flow == 1) return String("selected");
  if (var == "iSTD_ADJ_FLOW_2" && settings.std_adj_flow == 2) return String("selected");



  if (var == "iSTD_REF"  || var == "STANDARD_FLOW") {
    // Standard reference
    switch (settings.standardReference) {

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

  // General Decimal type
  if (var == "iGEN_DECI_ACC_0" && settings.gen_decimal_length == 0) return String("selected");
  if (var == "iGEN_DECI_ACC_1" && settings.gen_decimal_length == 1) return String("selected");
  if (var == "iGEN_DECI_ACC_2" && settings.gen_decimal_length == 2) return String("selected");

  // Flow Decimal type
  if (var == "iFLOW_DECI_ACC_0" && settings.flow_decimal_length == 0) return String("selected");
  if (var == "iFLOW_DECI_ACC_1" && settings.flow_decimal_length == 1) return String("selected");
  if (var == "iFLOW_DECI_ACC_2" && settings.flow_decimal_length == 2) return String("selected");


  // Data Filter type
  if (var == "iDATA_FLTR_TYP_1" && settings.data_filter_type == 1) return String("selected");
  if (var == "iDATA_FLTR_TYP_2" && settings.data_filter_type == 2) return String("selected");
  if (var == "iDATA_FLTR_TYP_3" && settings.data_filter_type == 3) return String("selected");
  if (var == "iDATA_FLTR_TYP_4" && settings.data_filter_type == 4) return String("selected");

  // Rounding type
  if (var == "iROUNDING_TYP_1" && settings.rounding_type == NONE) return String("selected");
  if (var == "iROUNDING_TYP_2" && settings.rounding_type == INTEGER) return String("selected");
  if (var == "iROUNDING_TYP_3" && settings.rounding_type == HALF) return String("selected");

  // if (var == "iROUNDING_TYP_DROPDOWN"){

  //   switch (settings.rounding_type) {
  //     case NONE:
  //       return String( "<select name='iROUNDING_TYP' class='config-select'><option value='1' selected>No Rounding</option><option value='2'>Integer</option><option value='3'>Half</option></select>");
  //     break;

  //     case INTEGER:
  //       return String( "<select name='iROUNDING_TYP' class='config-select'><option value='1'>No Rounding</option><option value='2' selected>Integer</option><option value='3'>Half</option></select>");
  //     break;

  //     case HALF:
  //       return String( "<select name='iROUNDING_TYP' class='config-select'><option value='1'>No Rounding</option><option value='2'>Integer</option><option value='3' selected>Half</option></select>");
  //     break;

  //     default:
  //       return String( "<select name='iROUNDING_TYP' class='config-select'><option value='1' selected>No Rounding</option><option value='2'>Integer</option><option value='3'>Half</option></select>");
  //     break;
  //   }
  // }

  
  // if (var == "iDATA_FLTR_TYP_DROPDOWN"){
  //   switch (settings.data_filter_type){
      
  //     case NONE:
  //       return String( "<select name='iDATA_FLTR_TYP' class='config-select'><option value='1' selected>None</option><option value='2'>Rolling Median</option><option value='3'>Cyclic Average </option><option value='4'>Mode</option></select>");
  //     break;

  //     case MEDIAN:
  //       return String( "<select name='iDATA_FLTR_TYP' class='config-select'><option value='1'>None</option><option value='2' selected>Rolling Median</option><option value='3'>Cyclic Average </option><option value='4'>Mode</option></select>");
  //     break;

  //     case AVERAGE:
  //       return String( "<select name='iDATA_FLTR_TYP' class='config-select'><option value='1'>None</option><option value='2'>Rolling Median</option><option value='3' selected>Cyclic Average </option><option value='4'>Mode</option></select>");
  //     break;

  //     case MODE:
  //       return String( "<select name='iDATA_FLTR_TYP' class='config-select'><option value='1'>None</option><option value='2'>Rolling Median</option><option value='3'>Cyclic Average </option><option value='4' selected>Mode</option></select>");
  //     break;
  //   }
  // }

  // Data Filter Settings
  if (var == "iMIN_FLOW_RATE") return String(settings.min_flow_rate);
  if (var == "iMIN_PRESSURE") return String(settings.min_bench_pressure);
  if (var == "iMAF_MIN_VOLTS") return String(settings.maf_min_volts);
  if (var == "iCYC_AV_BUFF") return String(settings.cyc_av_buffer);

  // Bench Settings
  if (var == "iMAF_DIAMETER") return String(settings.maf_housing_diameter);
  if (var == "iREFRESH_RATE") return String(settings.refresh_rate);
  if (var == "iADJ_FLOW_DEP") return String(settings.adj_flow_depression);

  // Temperature
  if (var == "TEMPERATURE_DROPDOWN"){
    if (settings.temp_unit == CELCIUS) {
      return String( "<select name='iTEMP_UNIT' class='config-select' id='iTEMP_UNIT'><option value='1' selected>Celcius </option><option value='2'>Farenheit </option></select>");
    } else {
      return String("<select name='iTEMP_UNIT' class='config-select' id='iTEMP_UNIT'><option value='1'>Celcius </option><option value='2' selected>Farenheit </option></select>");
    }
  }

  // Lift
  if (var == "dLIFT_INTERVAL") return String(settings.valveLiftInterval);

  // Bench type
  if (var == "iBENCH_TYPE_DROPDOWN") {
    switch (settings.bench_type) {
      case MAF_BENCH:
        return String( "<select name='iBENCH_TYPE' class='config-select'><option value='1' selected>MAF Style</option><option value='2'>Orifice Style</option><option value='3'>Venturi Style </option><option value='4'>Pitot Style</option></select>");
      break;

      case ORIFICE_BENCH:
        return String( "<select name='iBENCH_TYPE' class='config-select'><option value='1'>MAF Style</option><option value='2' selected>Orifice Style</option><option value='3'>Venturi Style </option><option value='4'>Pitot Style</option></select>");
      break;

      case VENTURI_BENCH:
        return String( "<select name='iBENCH_TYPE' class='config-select'><option value='1'>MAF Style</option><option value='2'>Orifice Style</option><option value='3' selected>Venturi Style </option><option value='4'>Pitot Style</option></select>");
      break;
      
      case PITOT_BENCH:
        return String( "<select name='iBENCH_TYPE' class='config-select'><option value='1'>MAF Style</option><option value='2'>Orifice Style</option><option value='3'>Venturi Style </option><option value='4' selected>Pitot Style</option></select>");
      break;
    }
  }
 

  // Bench definitions for system status pane 
  switch (settings.bench_type){

    case MAF_BENCH:
      status.benchType = "MAF Style";
    break;

    case ORIFICE_BENCH:
      status.benchType = "Orifice Style";
    break;

    case VENTURI_BENCH:
      status.benchType = "Venturi Style";
    break;

    case PITOT_BENCH:
      status.benchType = "Pitot Style";
    break;

  }






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

    if (fileList.isEmpty()) {
      return "<div class='fileListRow'><span class='column left'>No Files Found</span></div>";
    } else {
      return fileList;
    } 
  }



  // Calibration Settings
  if (var == "dCAL_FLW_RATE") return String(settings.cal_flow_rate);
  if (var == "dCAL_REF_PRESS") return String(settings.cal_ref_press);

  // Calibration Data
  if (var == "FLOW_OFFSET") return String(calVal.flow_offset);
  if (var == "USER_OFFSET") return String(calVal.user_offset);
  if (var == "LEAK_BASE") return String(calVal.leak_cal_baseline);
  if (var == "LEAK_OFFSET") return String(calVal.leak_cal_offset);
  if (var == "LEAK_BASE_REV") return String(calVal.leak_cal_baseline_rev);
  if (var == "LEAK_OFFSET_REV") return String(calVal.leak_cal_offset_rev);

  return "";
}








/***********************************************************
 * @brief processConfigPageTemplate
 * @details Replaces template placeholders with variable values
 * @param &var HTML payload 
 * @note ~PLACEHOLDER_FORMAT~
 ***/
String Webserver::processConfigPageTemplate(const String &var) {

  extern struct Configuration config;

  // Process language vars
  String langVar = processLanguageTemplateVars(var);
  if ( langVar != var) return langVar;


  //SD Dropdown
  if (var == "bSD_ENABLED_0" && config.bSD_ENABLED == 0) return String("selected");
  if (var == "bSD_ENABLED_1" && config.bSD_ENABLED == 1) return String("selected");

  if (var == "iMIN_PRESS_PCT" ) return String(config.iMIN_PRESS_PCT);
  if (var == "dPIPE_RAD_FT" ) return String(config.dPIPE_RAD_FT);

  // 3.3v dropdown
  if (var == "bFIXED_3_3V_0" && config.bFIXED_3_3V == 0) return String("selected");
  if (var == "bFIXED_3_3V_1" && config.bFIXED_3_3V == 1) return String("selected");

  if (var == "dVCC_3V3_TRIM" ) return String(config.dVCC_3V3_TRIM);

  // 5v dropdown
  if (var == "bFIXED_5V_0" && config.bFIXED_5V == 0) return String("selected");
  if (var == "bFIXED_5V_1" && config.bFIXED_5V == 1) return String("selected");

  if (var == "dVCC_5V_TRIM" ) return String(config.dVCC_5V_TRIM);

  // BME dropdown
  if (var == "iBME_TYP_1" && config.iBME_TYP == SENSOR_DISABLED) return String("selected");
  if (var == "iBME_TYP_7" && config.iBME_TYP == BOSCH_BME280) return String("selected");
  if (var == "iBME_TYP_17" && config.iBME_TYP == BOSCH_BME680) return String("selected");

  if (var == "iBME_ADDR" ) return String(config.iBME_ADDR);
  if (var == "iBME_SCAN_MS" ) return String(config.iBME_SCAN_MS);

  // ADC dropdown
  if (var == "iADC_TYPE_10" && config.iADC_TYPE == 10) return String("selected");
  if (var == "iADC_TYPE_11" && config.iADC_TYPE == 11) return String("selected");

  if (var == "iADC_I2C_ADDR" ) return String(config.iADC_I2C_ADDR);
  if (var == "iADC_SCAN_DLY" ) return String(config.iADC_SCAN_DLY);
  // if (var == "iADC_MAX_RETRY" ) return String(config.iADC_MAX_RETRY);
  if (var == "iADC_RANGE" ) return String(config.iADC_RANGE);
  if (var == "dADC_GAIN" ) return String(config.dADC_GAIN);

  // MAF Sensor Type dropdown
  if (var == "iMAF_SENS_TYP_0" && config.iMAF_SENS_TYP == 0) return String("selected");
  if (var == "iMAF_SENS_TYP_1" && config.iMAF_SENS_TYP == 1) return String("selected");
  if (var == "iMAF_SENS_TYP_2" && config.iMAF_SENS_TYP == 2) return String("selected");
  if (var == "iMAF_SENS_TYP_3" && config.iMAF_SENS_TYP == 3) return String("selected");
  if (var == "iMAF_SENS_TYP_4" && config.iMAF_SENS_TYP == 4) return String("selected");
  if (var == "iMAF_SENS_TYP_5" && config.iMAF_SENS_TYP == 5) return String("selected");
  if (var == "iMAF_SENS_TYP_6" && config.iMAF_SENS_TYP == 6) return String("selected");
  if (var == "iMAF_SENS_TYP_7" && config.iMAF_SENS_TYP == 7) return String("selected");
  if (var == "iMAF_SENS_TYP_8" && config.iMAF_SENS_TYP == 8) return String("selected");
  if (var == "iMAF_SENS_TYP_9" && config.iMAF_SENS_TYP == 9) return String("selected");
  if (var == "iMAF_SENS_TYP_10" && config.iMAF_SENS_TYP == 10) return String("selected");

  if (var == "dMAF_MV_TRIM" ) return String(config.dMAF_MV_TRIM);

  // if (var == "iMAF_ADC_CHAN" ) return String(config.iMAF_ADC_CHAN);

  // MAF Source dropdown
  if (var == "iMAF_SRC_TYP_1" && config.iMAF_SRC_TYP == 1) return String("selected");
  if (var == "iMAF_SRC_TYP_12" && config.iMAF_SRC_TYP == 12) return String("selected");
  if (var == "iMAF_SRC_TYP_18" && config.iMAF_SRC_TYP == 18) return String("selected");

  // MAF ADC Channel dropdown
  // if (var == "iMAF_ADC_CHAN_0" && config.iMAF_ADC_CHAN == 0) return String("selected");
  // if (var == "iMAF_ADC_CHAN_1" && config.iMAF_ADC_CHAN == 1) return String("selected");
  // if (var == "iMAF_ADC_CHAN_2" && config.iMAF_ADC_CHAN == 2) return String("selected");
  // if (var == "iMAF_ADC_CHAN_3" && config.iMAF_ADC_CHAN == 3) return String("selected");

  // pRef Sensor type dropdown
  if (var == "iPREF_SENS_TYP_1" && config.iPREF_SENS_TYP == 1) return String("selected");
  if (var == "iPREF_SENS_TYP_2" && config.iPREF_SENS_TYP == 2) return String("selected");
  if (var == "iPREF_SENS_TYP_3" && config.iPREF_SENS_TYP == 3) return String("selected");
  if (var == "iPREF_SENS_TYP_4" && config.iPREF_SENS_TYP == 4) return String("selected");
  if (var == "iPREF_SENS_TYP_5" && config.iPREF_SENS_TYP == 5) return String("selected");
  if (var == "iPREF_SENS_TYP_8" && config.iPREF_SENS_TYP == 8) return String("selected");
  if (var == "iPREF_SENS_TYP_12" && config.iPREF_SENS_TYP == 12) return String("selected");
  if (var == "iPREF_SENS_TYP_13" && config.iPREF_SENS_TYP == 13) return String("selected");
  if (var == "iPREF_SENS_TYP_14" && config.iPREF_SENS_TYP == 14) return String("selected");
  if (var == "iPREF_SENS_TYP_15" && config.iPREF_SENS_TYP == 15) return String("selected");
  if (var == "iPREF_SENS_TYP_16" && config.iPREF_SENS_TYP == 16) return String("selected");

  // pRef ADC Source dropdown
  if (var == "iPREF_SRC_TYP_12" && config.iPREF_SRC_TYP == 12) return String("selected");
  if (var == "iPREF_SRC_TYP_18" && config.iPREF_SRC_TYP == 18) return String("selected");

  // pRef ADC channel Dropdown
  // if (var == "iPREF_ADC_CHAN_1" && config.iPREF_ADC_CHAN == 1) return String("selected");
  // if (var == "iPREF_ADC_CHAN_2" && config.iPREF_ADC_CHAN == 2) return String("selected");
  // if (var == "iPREF_ADC_CHAN_3" && config.iPREF_ADC_CHAN == 3) return String("selected");
  // if (var == "iPREF_ADC_CHAN_4" && config.iPREF_ADC_CHAN == 4) return String("selected");

  // if (var == "iFIXED_PREF_VAL" ) return String(config.iFIXED_PREF_VAL);
  // if (var == "dPREF_ALG_SCALE" ) return String(config.dPREF_ALG_SCALE);
  if (var == "dPREF_MV_TRIM" ) return String(config.dPREF_MV_TRIM);


  //pDiff Sensor type dropdown
  if (var == "iPDIFF_SENS_TYP_1" && config.iPDIFF_SENS_TYP == 1) return String("selected");
  if (var == "iPDIFF_SENS_TYP_3" && config.iPDIFF_SENS_TYP == 3) return String("selected");
  if (var == "iPDIFF_SENS_TYP_4" && config.iPDIFF_SENS_TYP == 4) return String("selected");
  if (var == "iPDIFF_SENS_TYP_5" && config.iPDIFF_SENS_TYP == 5) return String("selected");
  if (var == "iPDIFF_SENS_TYP_8" && config.iPDIFF_SENS_TYP == 8) return String("selected");
  if (var == "iPDIFF_SENS_TYP_12" && config.iPDIFF_SENS_TYP == 12) return String("selected");
  if (var == "iPDIFF_SENS_TYP_13" && config.iPDIFF_SENS_TYP == 13) return String("selected");
  if (var == "iPDIFF_SENS_TYP_14" && config.iPDIFF_SENS_TYP == 14) return String("selected");
  if (var == "iPDIFF_SENS_TYP_15" && config.iPDIFF_SENS_TYP == 15) return String("selected");
  if (var == "iPDIFF_SENS_TYP_16" && config.iPDIFF_SENS_TYP == 16) return String("selected");

  if (var == "dPDIFF_MV_TRIM" ) return String(config.dPDIFF_MV_TRIM);

  // pDiff ADC Source dropdown
  if (var == "iPDIFF_SRC_TYP_12" && config.iPDIFF_SRC_TYP == 12) return String("selected");
  if (var == "iPDIFF_SRC_TYP_18" && config.iPDIFF_SRC_TYP == 18) return String("selected");

  //pDiff ADC channel Dropdown
  // if (var == "iPDIFF_ADC_CHAN_1" && config.iPDIFF_ADC_CHAN == 1) return String("selected");
  // if (var == "iPDIFF_ADC_CHAN_2" && config.iPDIFF_ADC_CHAN == 2) return String("selected");
  // if (var == "iPDIFF_ADC_CHAN_3" && config.iPDIFF_ADC_CHAN == 3) return String("selected");
  // if (var == "iPDIFF_ADC_CHAN_4" && config.iPDIFF_ADC_CHAN == 4) return String("selected");

  // if (var == "iFIXD_PDIFF_VAL" ) return String(config.iFIXD_PDIFF_VAL);
  // if (var == "dPDIFF_SCALE" ) return String(config.dPDIFF_SCALE);

  // Pitot Semnsor type dropdown
  if (var == "iPITOT_SENS_TYP_1" && config.iPITOT_SENS_TYP == 1) return String("selected");
  if (var == "iPITOT_SENS_TYP_3" && config.iPITOT_SENS_TYP == 3) return String("selected");
  if (var == "iPITOT_SENS_TYP_4" && config.iPITOT_SENS_TYP == 4) return String("selected");
  if (var == "iPITOT_SENS_TYP_5" && config.iPITOT_SENS_TYP == 5) return String("selected");
  if (var == "iPITOT_SENS_TYP_8" && config.iPITOT_SENS_TYP == 8) return String("selected");
  if (var == "iPITOT_SENS_TYP_12" && config.iPITOT_SENS_TYP == 12) return String("selected");
  if (var == "iPITOT_SENS_TYP_13" && config.iPITOT_SENS_TYP == 13) return String("selected");
  if (var == "iPITOT_SENS_TYP_14" && config.iPITOT_SENS_TYP == 14) return String("selected");
  if (var == "iPITOT_SENS_TYP_15" && config.iPITOT_SENS_TYP == 15) return String("selected");
  if (var == "iPITOT_SENS_TYP_16" && config.iPITOT_SENS_TYP == 16) return String("selected");

  if (var == "dPITOT_MV_TRIM" ) return String(config.dPITOT_MV_TRIM);

  // Pitot ADC Source dropdown
  if (var == "iPITOT_SRC_TYP_12" && config.iPITOT_SRC_TYP == 12) return String("selected");
  if (var == "iPITOT_SRC_TYP_18" && config.iPITOT_SRC_TYP == 18) return String("selected");

  //Pitot ADC channel Dropdown
  // if (var == "iPITOT_ADC_CHAN_1" && config.iPITOT_ADC_CHAN == 1) return String("selected");
  // if (var == "iPITOT_ADC_CHAN_2" && config.iPITOT_ADC_CHAN == 2) return String("selected");
  // if (var == "iPITOT_ADC_CHAN_3" && config.iPITOT_ADC_CHAN == 3) return String("selected");
  // if (var == "iPITOT_ADC_CHAN_4" && config.iPITOT_ADC_CHAN == 4) return String("selected");

  // if (var == "dPITOT_SCALE" ) return String(config.dPITOT_SCALE);

  // Baro Sensor type dropdown
  if (var == "iBARO_SENS_TYP_1" && config.iBARO_SENS_TYP == 1) return String("selected");
  if (var == "iBARO_SENS_TYP_3" && config.iBARO_SENS_TYP == 3) return String("selected");
  if (var == "iBARO_SENS_TYP_7" && config.iBARO_SENS_TYP == 7) return String("selected");
  if (var == "iBARO_SENS_TYP_12" && config.iBARO_SENS_TYP == 12) return String("selected");
  if (var == "iBARO_SENS_TYP_17" && config.iBARO_SENS_TYP == 17) return String("selected");

  if (var == "dBARO_MV_TRIM" ) return String(config.dBARO_MV_TRIM);
  if (var == "dBARO_FINE_TUNE" ) return String(config.dBARO_FINE_TUNE);

  // if (var == "dFIXD_BARO_VAL" ) return String(config.dFIXD_BARO_VAL);
  // if (var == "dBARO_ALG_SCALE" ) return String(config.dBARO_ALG_SCALE);
  // if (var == "dBARO_SCALE" ) return String(config.dBARO_SCALE);
  // if (var == "dBARO_OFFSET" ) return String(config.dBARO_OFFSET);

  // Temp Sensor type dropdown
  if (var == "iTEMP_SENS_TYP_1" && config.iTEMP_SENS_TYP == 1) return String("selected");
  if (var == "iTEMP_SENS_TYP_3" && config.iTEMP_SENS_TYP == 3) return String("selected");
  if (var == "iTEMP_SENS_TYP_7" && config.iTEMP_SENS_TYP == 7) return String("selected");
  if (var == "iTEMP_SENS_TYP_12" && config.iTEMP_SENS_TYP == 12) return String("selected");
  if (var == "iTEMP_SENS_TYP_17" && config.iTEMP_SENS_TYP == 17) return String("selected");

  if (var == "dTEMP_MV_TRIM" ) return String(config.dTEMP_MV_TRIM);
  if (var == "dTEMP_FINE_TUNE" ) return String(config.dTEMP_FINE_TUNE);
  // if (var == "dFIXED_TEMP_VAL" ) return String(config.dFIXED_TEMP_VAL);
  // if (var == "dTEMP_ALG_SCALE" ) return String(config.dTEMP_ALG_SCALE);

  // Humidity Sensor type dropdown
  if (var == "iRELH_SENS_TYP_1" && config.iRELH_SENS_TYP == 1) return String("selected");
  if (var == "iRELH_SENS_TYP_3" && config.iRELH_SENS_TYP == 3) return String("selected");
  if (var == "iRELH_SENS_TYP_7" && config.iRELH_SENS_TYP == 7) return String("selected");
  if (var == "iRELH_SENS_TYP_12" && config.iRELH_SENS_TYP == 12) return String("selected");
  if (var == "iRELH_SENS_TYP_17" && config.iRELH_SENS_TYP == 17) return String("selected");

  if (var == "dRELH_MV_TRIM" ) return String(config.dRELH_MV_TRIM);
  if (var == "dRELH_FINE_TUNE" ) return String(config.dRELH_FINE_TUNE);
  // if (var == "dFIXED_RELH_VAL" ) return String(config.dFIXED_RELH_VAL);
  // if (var == "dRELH_ALG_SCALE" ) return String(config.dRELH_ALG_SCALE);
  

  return "";
}



/***********************************************************
 * @brief processDatagraphPage
 * @details Replaces template placeholders with variable values
 * @param &var HTML payload 
 * @note ~PLACEHOLDER_FORMAT~
 ***/
String Webserver::processDatagraphPageTemplate(const String &var) {

  extern struct BenchSettings settings;
  extern struct ValveLiftData valveData;  
  extern struct Configuration config;

  MafData _maf(config.iMAF_SENS_TYP);

  // Process language vars
  String langVar = processLanguageTemplateVars(var);
  if ( langVar != var) return langVar;



  // Datagraph Stuff
  int maxval = 0;
  int maxcfm = 0;
  double scaleFactor = 0.0;

  // very rough cfm calculation from max mafdata (approx half of KG/h rate)
  maxcfm = _maf.getMaxKGH() / 2;

  // Determine data graph flow axis scale
  // NOTE: currently 1000cfm is the largest flow that the graph will display. 
  // We could change scaling to be relative to SVG height (surface currently fixed at 500)
  if ((maxcfm < 500) || (settings.dataGraphMax == 1)) {
    maxval = 250;
    scaleFactor = 2;
  } else if ((maxcfm > 250 && maxcfm < 500) || (settings.dataGraphMax == 2)) {
    maxval = 500;
    scaleFactor = 1;
  } else if ((maxcfm > 500)  || (settings.dataGraphMax == 3)  || (settings.dataGraphMax == 0)){
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
  if (floor(settings.valveLiftInterval) == settings.valveLiftInterval) {

    // it's an integer so lets truncate fractional part
    int liftInterval = settings.valveLiftInterval;
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
    if (var == "lift1") return String(1 * settings.valveLiftInterval);
    if (var == "lift2") return String(2 * settings.valveLiftInterval);
    if (var == "lift3") return String(3 * settings.valveLiftInterval);
    if (var == "lift4") return String(4 * settings.valveLiftInterval);
    if (var == "lift5") return String(5 * settings.valveLiftInterval);
    if (var == "lift6") return String(6 * settings.valveLiftInterval);
    if (var == "lift7") return String(7 * settings.valveLiftInterval);
    if (var == "lift8") return String(8 * settings.valveLiftInterval);
    if (var == "lift9") return String(9 * settings.valveLiftInterval);
    if (var == "lift10") return String(10 * settings.valveLiftInterval);
    if (var == "lift11") return String(11 * settings.valveLiftInterval);
    if (var == "lift12") return String(12 * settings.valveLiftInterval);
  }

  return "";

}






/***********************************************************
 * @brief processPinsPage
 * @details Replaces template placeholders with variable values
 * @param &var HTML payload 
 * @note ~PLACEHOLDER_FORMAT~
 ***/
String Webserver::processPinsPageTemplate(const String &var) {

  extern struct Pins pins;

  // Process language vars
  String langVar = processLanguageTemplateVars(var);
  if (langVar != var) return langVar;

  if (var == "VAC_SPEED") return String(pins.VAC_SPEED);
  if (var == "VAC_BLEED_VALVE") return String(pins.VAC_BLEED_VALVE);
  if (var == "VAC_BANK_1") return String(pins.VAC_BANK_1);
  if (var == "VAC_BANK_2") return String(pins.VAC_BANK_2);
  if (var == "VAC_BANK_3") return String(pins.VAC_BANK_3);
  if (var == "AVO_STEP") return String(pins.AVO_STEP);
  if (var == "AVO_DIR") return String(pins.AVO_DIR);
  if (var == "FLOW_VALVE_STEP") return String(pins.FLOW_VALVE_STEP);
  if (var == "FLOW_VALVE_DIR") return String(pins.FLOW_VALVE_DIR);
  if (var == "VCC_3V3") return String(pins.VCC_3V3);
  if (var == "VCC_5V") return String(pins.VCC_5V);
  if (var == "SPEED_SENS") return String(pins.SPEED_SENS);
  if (var == "SWIRL_ENCODER_A") return String(pins.SWIRL_ENCODER_A);
  if (var == "SWIRL_ENCODER_B") return String(pins.SWIRL_ENCODER_B);
  if (var == "ORIFICE_BCD_1") return String(pins.ORIFICE_BCD_1);
  if (var == "ORIFICE_BCD_2") return String(pins.ORIFICE_BCD_2);
  if (var == "ORIFICE_BCD_3") return String(pins.ORIFICE_BCD_3);
  if (var == "MAF") return String(pins.MAF);
  if (var == "PREF") return String(pins.PREF);
  if (var == "PDIFF") return String(pins.PDIFF);
  if (var == "PITOT") return String(pins.PITOT);
  if (var == "TEMPERATURE") return String(pins.TEMPERATURE);
  if (var == "REF_BARO") return String(pins.REF_BARO);
  if (var == "HUMIDITY") return String(pins.HUMIDITY);
  if (var == "SERIAL0_TX") return String(pins.SERIAL0_TX);
  if (var == "SERIAL0_RX") return String(pins.SERIAL0_RX);
  if (var == "SERIAL2_TX") return String(pins.SERIAL2_TX);
  if (var == "SERIAL2_RX") return String(pins.SERIAL2_RX);
  if (var == "SDA") return String(pins.SDA);
  if (var == "SCL") return String(pins.SCL);
  if (var == "SD_CS") return String(pins.SD_CS);
  if (var == "SD_MOSI") return String(pins.SD_MOSI);
  if (var == "SD_MISO") return String(pins.SD_MISO);
  if (var == "SD_SCK") return String(pins.SD_SCK);
  if (var == "SPARE_PIN_1") return String(pins.SPARE_PIN_1);
  if (var == "SPARE_PIN_2") return String(pins.SPARE_PIN_2);

  return "";

}





/***********************************************************
 * @brief processMimicPage
 * @details Replaces template placeholders with variable values
 * @param &var HTML payload 
 * @note ~PLACEHOLDER_FORMAT~
 ***/
String Webserver::processMimicPageTemplate(const String &var) {

  extern struct Pins pins;

  // Process language vars
  String langVar = processLanguageTemplateVars(var);
  if (langVar != var) return langVar;

  if (var == "VAC_SPEED") return String(pins.VAC_SPEED);
  if (var == "VAC_BLEED_VALVE") return String(pins.VAC_BLEED_VALVE);


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
  Suspect easiest solution is to make valve interval editable via GUI settings. This way users can change as needed.

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