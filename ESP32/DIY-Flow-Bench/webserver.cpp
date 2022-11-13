/***********************************************************
 * The DIY Flow Bench project
 * https://diyflowbench.com
 * 
 * server.cpp - Webserver class file
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

#include "webserver.h"
#include "Arduino.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "constants.h"
#include "configuration.h"
#include "calibration.h"
#include "structs.h"
#include "sensors.h"
#include "pins.h"
#include "settings.h"
#include "hardware.h"
#include "messages.h"
#include "calculations.h"
#include LANGUAGE_FILE




/***********************************************************
* Class Constructor
***/
// Webserver::Webserver() {
// 
// }




/***********************************************************
* INITIALISE SERVER
*
* NOTE: https://github.com/me-no-dev/ESPAsyncWebServer/issues/225
* NOTE: https://github.com/me-no-dev/ESPAsyncWebServer/issues/978
* https://stackoverflow.com/questions/73063497/encapsulate-asyncwebserver-and-asyncwebsocket-in-c-class
* 
***/
void Webserver::begin() {
    
    extern struct ConfigSettings config;
    extern struct Translator translate;
    extern DeviceStatus status;
    
    server = new AsyncWebServer(80);
    webskt = new AsyncWebSocket("/ws");
    
    // TODO: Test Accesspoint is working
    String index_html = translate.LANG_VAL_INDEX_HTML;
    
    Messages _message;
    Settings _settings;
    Calibration _calibration;

    // Filesystem
    _message.serialPrintf("File System Initialisation...\n");
    if (SPIFFS.begin()){
      _message.serialPrintf("Complete.\n");
    } else {
      _message.serialPrintf("Failed.\n");
      #if defined FORMAT_FILESYSTEM_IF_FAILED
          SPIFFS.format();
          _message.serialPrintf("!! File System Formatted !!\n");
      #endif
    }
    
    // Check if config and calibration json files exist. If not create them.
    if (!SPIFFS.exists("/config.json")){
      _settings.createConfigFile();
    }
    if (!SPIFFS.exists("/cal.json")){
      _calibration.createCalibrationFile();
    }
    
    _settings.LoadConfig();
    _calibration.loadCalibration();
    
    // Filesystem info
    status.spiffs_mem_size = SPIFFS.totalBytes();
    status.spiffs_mem_used = SPIFFS.usedBytes();
 
    _message.serialPrintf("===== File system info ===== \n");
    _message.serialPrintf("Total space:      %s \n", byteDecode(status.spiffs_mem_size));
    _message.serialPrintf("Total space used: %s \n", byteDecode(status.spiffs_mem_used));

    // WiFi    
    status.apMode = false;
    unsigned long timeOut;
    timeOut = millis() + config.wifi_timeout;
    _message.serialPrintf("Connecting to WiFi...\n");
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.wifi_ssid, config.wifi_pswd);
    while (WiFi.status() != WL_CONNECTED && millis() < timeOut) {
      delay(500);
      _message.serialPrintf(">");
    } 
    
    if (WiFi.status() == WL_CONNECTED) {  
      // Connection success     
      _message.serialPrintf("\nConnected to %s \n", config.wifi_ssid);
      status.local_ip_address = WiFi.localIP().toString().c_str();
      _message.serialPrintf("IP address: %s \n", WiFi.localIP().toString().c_str());
    } else {
      // Did not connect - Go into AP Mode
      status.apMode = true;
      _message.serialPrintf("\nFailed to connect to Wifi \n");
      _message.serialPrintf("Creating WiFi Access Point:  %s  \n", config.wifi_ap_ssid); //NOTE: Default AP SSID / PW = DIYFB / 123456789
      WiFi.softAP(config.wifi_ap_ssid, config.wifi_ap_pswd);
      status.local_ip_address = WiFi.localIP().toString().c_str();
      _message.serialPrintf("Access Point IP address: %s \n", WiFi.localIP().toString().c_str()); 
    }

    // Set up Multicast DNS
    if(!MDNS.begin(config.hostname)) {
      _message.serialPrintf("Error starting mDNS \n");
    } else {
      _message.serialPrintf("Access via: http://%s.local \n", config.hostname);         
    }

    // Debug request handler
    server->on("/debug", HTTP_POST, [](AsyncWebServerRequest *request){
      Messages _message;
      _message.serialPrintf("Debug Mode \n");
      config.debug_mode = true;
      request->redirect("/");
    });
    
    // Dev mode request handler
    server->on("/dev", HTTP_POST, [](AsyncWebServerRequest *request){
      Messages _message;
      _message.serialPrintf("Developer Mode \n");
      config.dev_mode = true;
      request->redirect("/");
    });
    
    // Upload request handler
    server->on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
      Messages _message;
      _message.serialPrintf("Upload Request Called \n");
      request->redirect("/?modal=upload");
    }, processUpload);
    
    server->rewrite("/", "/index.html");

    // Index page request handler
    server->on("/index.html", HTTP_ANY, [index_html](AsyncWebServerRequest *request){
      // if (SPIFFS.exists("/index.html.gz")){
      //   AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html.gz", "text/html", false);
      //   response->addHeader("Content-Encoding", "gzip");
      //   request->send(response);
      // } else if (SPIFFS.exists("/index.html")) {
      if (SPIFFS.exists("/index.html")) {
        request->send(SPIFFS, "/index.html", "text/html");
      } else {
        request->send(200, "text/html", index_html); 
      }
    });

    // Style sheet request handler 
    server->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/style.css", "text/css");
    });
    
    // Javascript file request handler
    server->on("/javascript.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/javascript.js", "text/javascript");
    });
    
    // Download request handler
    server->on("/download", HTTP_GET, [](AsyncWebServerRequest *request){              
          Messages _message;
          String downloadFilename = request->url();
          downloadFilename.remove(0,9);
          _message.serialPrintf("Request Delete File: %s \n", downloadFilename);
          request->send(SPIFFS, downloadFilename, String(), true);
    });

    // NOTE" this acts as a catch-all and allows us to process any request not specifically declared above
    server->onNotFound( []( AsyncWebServerRequest * request ) {
      Messages _message;
      _message.debugPrintf("REQUEST: NOT_FOUND: \n");
      if (request->method() == HTTP_GET)
        _message.debugPrintf("REQUEST: GET \n");
      else if (request->method() == HTTP_POST)
        _message.debugPrintf("REQUEST: POST \n");
      else if (request->method() == HTTP_DELETE)
        _message.debugPrintf("REQUEST: DELETE \n");
      else if (request->method() == HTTP_PUT)
        _message.debugPrintf("REQUEST: PUT \n");
      else if (request->method() == HTTP_PATCH)
        _message.debugPrintf("REQUEST: PATCH \n");
      else if (request->method() == HTTP_HEAD)
        _message.debugPrintf("REQUEST: HEAD \n");
      else if (request->method() == HTTP_OPTIONS)
        _message.debugPrintf("REQUEST: OPTIONS \n");
      else
        _message.debugPrintf("REQUEST UNKNOWN \n");
        //_message.serialPrint(" http://%s%s\n", request->host().c_str(), request->url().c_str());
        request->send( 404, "text/plain", "Not found." );
    });

    // start websocket ISR listener
    // REF: https://github.com/me-no-dev/ESPAsyncWebServer/issues/225
    // webskt->onEvent(onWebSocketEvent); 
    
    webskt->onEvent(std::bind(&Webserver::onWebSocketEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    


    server->onFileUpload(processUpload);
    server->addHandler(webskt);
    server->begin();

    _message.Handler(translate.LANG_VAL_WEBSERVER_RUNNING);
    _message.serialPrintf("Webserver Running \n");

}




/***********************************************************
* sendWebSocketMessage
* Push Server Data to client via websocket
***/
void Webserver::sendWebSocketMessage(String jsonValues) {

  webskt->textAll(String(jsonValues));
  
}





/***********************************************************
* onWebSocketEvent
* Websocket Event Listener
***/
void Webserver::onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {

  extern struct WebsocketData socketData;
  extern struct DeviceStatus status;
  extern struct Translator translate;
  
  Settings _settings;
  Hardware _hardware;
  Webserver _webserver;
  Calibration _calibration;
  Messages _message;
    
  StaticJsonDocument<1024> messageData;
  StaticJsonDocument<1024> jsonMessage;
  String jsonMessageString;
  String clientIP;
  int header;
  DeserializationError err;
  
  switch (type) {
    
    case WS_EVT_CONNECT:
      clientIP = client->remoteIP().toString().c_str();
      _message.debugPrintf("WebSocket client connected from %s \n", clientIP);
    break;
    
    case WS_EVT_DISCONNECT:
      _message.debugPrintf("WebSocket client disconnected \n");
    break;

    case WS_EVT_PONG:
      _message.debugPrintf("PING \n");
    break;
    
    case WS_EVT_ERROR:
      _message.debugPrintf("WebSocket error \n");
    break;
    
    case WS_EVT_DATA:
      // this->processWebSocketMessage(arg, data, len);

      _message.debugPrintf("Websocket data received \n"); 

      err = deserializeJson(messageData, data);
      if (err) {
        _message.debugPrintf("processWebSocketMessage->deserializeJson() failed: %s \n", err.c_str());
      }

      header = messageData["HEADER"].as<int>(); 

      _message.debugPrintf("Websocket header: %i \n", header);

      switch (header) {

        case GET_FLOW_DATA: // HEADER: 1 
          _message.debugPrintf("Get Data \n");
          sendWebSocketMessage(_webserver.getDataJSON());
        break;      
        
        case REC_FLOW_DATA: // HEADER: 2 
        break;
        
        case CALIBRATE: // HEADER: 3 
          _message.debugPrintf("Calibrate \n");
          if (_hardware.benchIsRunning()){          
            _calibration.setFlowOffset();
            // send new calibration to the browsers
            //_webserver.sendWebSocketMessage("flow offset");
          } else {
            _message.Handler(translate.LANG_VAL_RUN_BENCH_TO_CALIBRATE);        
          }
        break;
        
        case FILE_LIST: // HEADER: 4 
          _message.debugPrintf("File List \n");
          sendWebSocketMessage(_webserver.getFileListJSON());      
        break;      
            
        case SYS_STATUS: // HEADER: 5 
          _message.debugPrintf("Send Status \n");
          sendWebSocketMessage(_webserver.getSystemStatusJSON());      
        break;  

        case SAVE_CONFIG: // HEADER: 6 
          _message.debugPrintf("Save Config \n");
          _settings.saveConfig(messageData);
          
        case LOAD_CONFIG: // HEADER: 7 
          _message.debugPrintf("Load Config \n");
          jsonMessage = _settings.LoadConfig();
          jsonMessage["HEADER"] = LOAD_CONFIG; 
          serializeJson(jsonMessage, jsonMessageString);
          sendWebSocketMessage(jsonMessageString);      
        break;      

        case FILE_DOWNLOAD: // HEADER: 8 
          // NOTE: https://github.com/DeeEmm/DIY-Flow-Bench/issues/71#issuecomment-893104615        
        break;  

        case FILE_DELETE: // HEADER: 9 
          socketData.file_name = messageData["FILENAME"].as<String>(); 
          _message.debugPrintf("File Delete: %s \n", socketData.file_name);
          if(SPIFFS.exists(socketData.file_name)){
            SPIFFS.remove(socketData.file_name);
          }  else {
            _message.statusPrintf("Delete Failed: %s \n", socketData.file_name);      
          } 
          sendWebSocketMessage(_webserver.getFileListJSON());      
        break;  

        case FILE_UPLOAD: // HEADER: 10 
                  
        break;

        case START_BENCH: // HEADER: 11 
          _message.debugPrintf("Start Bench \n");
          status.liveStream = true;
          digitalWrite(VAC_BANK_1_PIN, HIGH);
        break;     
        
        case STOP_BENCH: // HEADER: 12 
          _message.statusPrintf("Stop Bench \n");
          status.liveStream = false;   
          digitalWrite(VAC_BANK_1_PIN, LOW);   
        break;  

        case LEAK_CAL: // HEADER: 13 
          _message.debugPrintf("Leak Test Calibration \n");
          if (_hardware.benchIsRunning()){          
            _calibration.setLeakTestPressure();
            // send new calibration to the browser
            //_webserver.sendWebSocketMessage("flow offset");
          } else {
            _message.Handler(translate.LANG_VAL_RUN_BENCH_TO_CALIBRATE);        
          }
        break;         
        
        case GET_CAL: // HEADER: 14 
          _message.debugPrintf("Send Calibration \n");
          jsonMessage = _calibration.loadCalibration();
          jsonMessage["HEADER"] = GET_CAL; 
          serializeJson(jsonMessage, jsonMessageString);
          sendWebSocketMessage(jsonMessageString);            
        break;     
        
        case RESTART: // HEADER: 15 
          ESP.restart();
          _message.debugPrintf("ESP Restart \n");
        break;

        default:
          _message.debugPrintf("Unrecognised Websocket Header \n");
          _message.debugPrintf("HEADER: %s \n", header);
        break;

      }

      if (strcmp((char*)data, "getValues") == 0) {
        sendWebSocketMessage(_webserver.getDataJSON());
      }

    break;

  }
}


/***********************************************************
* byteDecode
* Byte Decode (returns string)
***/
String Webserver::byteDecode(size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}




/***********************************************************
* Process File Upload 
* Redirects browser back to Upload modal unless upload is index file
***/
void Webserver::processUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {

  Messages _message;
  extern struct FileUploadData fileUploadData;
  String redirectURL;
  
  if (SPIFFS.exists (filename) ) {
    SPIFFS.remove (filename);
  }
  uint32_t freespace = SPIFFS.totalBytes() - SPIFFS.usedBytes();
  
  if(!filename.startsWith("/")) filename = "/"+filename;
  if(!index && !fileUploadData.upload_error){
    _message.statusPrintf("UploadStart: %s \n", filename);
    request->_tempFile = SPIFFS.open(filename, "w");
  }
  
  // Set redirect to file Upload modal unless uploading the index file
  if (filename == String("/index.html.gz") || filename == String("/index.html") ){
    redirectURL = "/";
  } else {
    redirectURL = "/?modal=upload";
  }
  
  if(len) {   
    fileUploadData.file_size += len;
    if (fileUploadData.file_size > freespace) {  
      // TODO: _message.statusPrintf("Upload rejected, not enough space \n");
      fileUploadData.upload_error = true;
    } else {
      // TODO: _message.statusPrintf("Writing file: '%s' index=%s len=%s \n", filename, index, len);
      request->_tempFile.write(data,len);
    }
  }
  
  if(final){
    _message.statusPrintf("UploadEnd: %s,%s \n", filename, fileUploadData.file_size);
    request->_tempFile.close();
    request->redirect(redirectURL);
  }
}




/***********************************************************
* decodeMessageHeader
* Pulls HEADER value from JSON string
***/
 int Webserver::decodeMessageHeader (char *data) {
   
  Messages _message;
  
  int header = 0;

  StaticJsonDocument<1024> messageData;
  DeserializationError error = deserializeJson(messageData, data);
  if (!error){
    header = messageData["HEADER"].as<int>();  
    _message.debugPrintf("Decoded Message Header: %s \n", header);
  } else {
    _message.debugPrintf("Webserver::decodeMessageHeader ERROR \n");
  }
  
  return header;

}




/***********************************************************
* getFileListJSON
* Get SPIFFS File List in JSON format
***/
 String Webserver::getFileListJSON () {
    
    String jsonString;
    String fileName;
    size_t fileSize;

    StaticJsonDocument<1024> dataJson;    
    dataJson["HEADER"] = FILE_LIST;
    
    Messages _message;

    _message.statusPrintf("Filesystem contents: \n");
    FILESYSTEM.begin();
    File root = FILESYSTEM.open("/");
    File file = root.openNextFile();
    while(file){
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
* getSystemStatusJSON
* Get System Status in JSON format
***/
String Webserver::getSystemStatusJSON() {
  
  extern DeviceStatus status;
  String jsonString;
  
  Messages _message;
  
  _message.debugPrintf("Webserver::getDataJSON() \n");
  
  StaticJsonDocument<1024> dataJson;    

  dataJson["HEADER"] = SYS_STATUS;
  dataJson["SPIFFS_MEM_SIZE"] = status.spiffs_mem_size;
  dataJson["SPIFFS_MEM_USED"] = status.spiffs_mem_used;
  dataJson["LOCAL_IP_ADDRESS"] = status.local_ip_address;
  dataJson["BOARDTYPE"] = status.boardType;
  dataJson["BENCHTYPE"] = status.benchType;
  dataJson["MAF_SENSOR"] = status.mafSensor;
  dataJson["PREF_SENSOR"] = status.prefSensor;
  dataJson["TEMP_SENSOR"] = status.tempSensor;
  dataJson["RELH_SENSOR"] = status.relhSensor;
  dataJson["BARO_SENSOR"] = status.baroSensor;
  dataJson["PITOT_SENSOR"] = status.pitotSensor;
  dataJson["BOOT_TIME"] = status.boot_time;
 
  serializeJson(dataJson, jsonString);
  return jsonString;

}




/***********************************************************
* getDataJSON
* Package up current bench data into JSON string
***/
String Webserver::getDataJSON() { 

  extern struct DeviceStatus status;
  extern struct ConfigSettings config;
  extern struct SensorData sensorVal;
  String jsonString;
  
  Calculations _calculations;
  Hardware _hardware;
  Messages _message;
  Sensors _sensors;
  
  _message.debugPrintf("Webserver::getDataJSON() \n");
  
  // double refPressure = _calculations.convertPressure(sensorVal.PRefKPA, INWG);   

  double mafFlowCFM = _calculations.calculateFlowCFM();
  
  StaticJsonDocument<1024> dataJson;    
  
  dataJson["HEADER"] = GET_FLOW_DATA;
  dataJson["STATUS_MESSAGE"] = status.statusMessage;
  // Flow Rate
  if (mafFlowCFM > config.min_flow_rate) {
    dataJson["FLOW"] = mafFlowCFM;        
  } else {
    dataJson["FLOW"] = 0;        
  }
  dataJson["TEMP"] = sensorVal.TempDegC;        
  // dataJson["TEMP"] = String(_calculations.convertTemperature(sensorVal.tempDegC, DEGC);        
  dataJson["BARO"] = sensorVal.BaroKPA;        
  dataJson["RELH"] = sensorVal.RelH;

  // Pitot
  // NOT USED: double pitotPressure = _calculations.calculatePitotPressure(INWG);
  // Pitot probe displays as a percentage of the reference pressure
  // double pitotPercentage = (_calculations.convertPressure(sensorVal.PitotKPA, INWG) / refPressure);
  dataJson["PITOT"] = sensorVal.PitotKPA;
  
  // Reference pressure
  dataJson["PREF"] = sensorVal.PRefKPA;
  
  // Adjusted Flow
  // get the desired bench test pressure
   // double desiredRefPressureInWg = menuARef.getCurrentValue(); //TODO:: Add ref pressure setting to UI & Config
  // convert from the existing bench test
   // double adjustedFlow = convertFlowDepression(refPressure, desiredRefPressureInWg, mafFlowCFM);
  // Send it to the display
   // dataJson["AFLOW"] = String(adjustedFlow);

  
  dataJson["RELEASE"] = RELEASE;
  dataJson["BUILD_NUMBER"] = BUILD_NUMBER;
  dataJson["PREF_MV"] = sensorVal.PRefMv;
  dataJson["PITOT_MV"] = sensorVal.PitotMv;
  
  // TODO: need to add PDIFF_MV
  // PDiff Voltage
  // dataJson["PDIFF_MV"] = String(sensorVal.PDiffMv);
  
  
  serializeJson(dataJson, jsonString);
  
  _message.statusPrintf("JSON Data Created \n");
  
  return jsonString;

}




/***********************************************************
* onBody
* Serves gzipped page if available 
***/
void Webserver::onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  if (SPIFFS.exists("/index.html.gz")){
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html.gz", "text/html", false);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  } else {
    request->send(SPIFFS, "/index.html", "text/html");
  }
}




/***********************************************************
* writeJSONFile
* write JSON string to file
***/
void Webserver::writeJSONFile(String data, String filename) {

  Messages _message;

  StaticJsonDocument<1024> jsonData;
  DeserializationError error = deserializeJson(jsonData, data);
  if (!error){
    _message.debugPrintf("Writing JSON file... \n");  
    File outputFile = SPIFFS.open(filename, FILE_WRITE);
    serializeJsonPretty(jsonData, outputFile);
    outputFile.close();
  } else {
    _message.statusPrintf("Webserver::writeJSONFile ERROR \n");
  }

}




/***********************************************************
 * loadJSONFile
 * loads JSON data from file 
 ***/
StaticJsonDocument<1024> Webserver::loadJSONFile(String filename) {
  
  Messages _message;
  
  extern struct Translator translate;
  
  // Allocate the memory pool on the stack.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<1024> jsonData;

  if (SPIFFS.exists(filename)){
    File jsonFile = SPIFFS.open(filename, FILE_READ);
  
    if (!jsonFile) {
        _message.Handler(translate.LANG_VAL_ERROR_LOADING_FILE);
        _message.statusPrintf("Failed to open file for reading \n");
    } else {
        size_t size = jsonFile.size();
        if (size > 1024) {
          #ifdef DEBUG 
            _message.statusPrintf("Config file size is too large \n");
          #endif
        }

        DeserializationError error = deserializeJson(jsonData, jsonFile);
        if (error) {
          _message.statusPrintf("loadJSONFile->deserializeJson() failed: %s \n", error.f_str());
        }
        
        jsonFile.close();
        return jsonData;
        
        // String jsonString;
        // serializeJson(jsonData, jsonString);
        // return jsonString;
    }    
    jsonFile.close();
    
  } else {
    _message.statusPrintf("File missing \n");
  }
  
  return jsonData;
}





/***********************************************************
 * processTemplate
 * 
 * Processes template placeholders 
 * %PLACEHOLDER_FORMAT%
 ***/
String processTemplate(const String& var)
{

  return var == "TEMPLATE_TEST" ? F("Hello world!") : String();
  // return var == "TEMPLATE_TEST" ? F("Hello world!") : String();

}