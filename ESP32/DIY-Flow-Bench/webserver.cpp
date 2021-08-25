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

// 
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
#include "pins.h"
#include "settings.h"
#include "hardware.h"
#include "messages.h"
#include "maths.h"
#include LANGUAGE_FILE


Webserver::Webserver() {

}





/***********************************************************
* Push Server Data to client via websocket
***/
void Webserver::SendWebSocketMessage(String jsonValues) {
  
  Messages _message;
  _message.DebugPrintLn("Webserver::SendWebSocketMessage");
  _message.DebugPrintLn(String(jsonValues));
  
  extern AsyncWebSocket ws;
  ws.textAll(String(jsonValues));
  
}







/***********************************************************
* Process Received Websocket Message
***/
void Webserver::ProcessWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  
  extern struct WebsocketData socketData;
  extern struct ConfigSettings settings;
  extern struct DeviceStatus status;
  
  Settings _settings;
  Hardware _hardware;
  Webserver _webserver;
  Calibration _calibration;
  Messages _message;
  
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  
  StaticJsonDocument<1024> messageData;
  DeserializationError err = deserializeJson(messageData, data);
  if (err) {
    _message.DebugPrint(F("ProcessWebSocketMessage->deserializeJson() failed: "));
    _message.DebugPrintLn(err.c_str());
  }
  
  int header = messageData["HEADER"].as<int>(); 

  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    
    data[len] = 0;
    StaticJsonDocument<1024> jsonMessage;
    String jsonMessageString;

    _message.DebugPrint("Websocket header received: '");
    _message.DebugPrint(String(header));
    _message.DebugPrintLn("'");

    switch (header) {

      case GET_FLOW_DATA: // HEADER: 1 
        _message.DebugPrintLn("Get Data");
        SendWebSocketMessage(_webserver.getDataJSON());
      break;      
      
      case REC_FLOW_DATA: // HEADER: 2 
      break;
      
      case CALIBRATE: // HEADER: 3 
        _message.DebugPrintLn("Calibrate");
        if (_hardware.benchIsRunning()){          
          _calibration.setFlowOffset();
          // send new calibration to the browser
          //_webserver.SendWebSocketMessage("flow offset");
        } else {
          _message.Handler(LANG_RUN_BENCH_TO_CALIBRATE);        
        }
      break;
      
      case FILE_LIST: // HEADER: 4 
        _message.DebugPrintLn("File List");
        SendWebSocketMessage(_webserver.getFileListJSON());      
      break;      
         
      case SYS_STATUS: // HEADER: 5 
        _message.DebugPrintLn("Send Status");
        SendWebSocketMessage(_webserver.getSystemStatusJSON());      
      break;  

      case SAVE_CONFIG: // HEADER: 6 
        _message.DebugPrintLn("Save Config");
        _settings.saveConfig(messageData);
        
      case LOAD_CONFIG: // HEADER: 7 
        _message.DebugPrintLn("Load Config");
        jsonMessage = _settings.LoadConfig();
        jsonMessage["HEADER"] = LOAD_CONFIG; 
        serializeJson(jsonMessage, jsonMessageString);
        SendWebSocketMessage(jsonMessageString);      
      break;      

      case FILE_DOWNLOAD: // HEADER: 8 
        // NOTE: https://github.com/DeeEmm/DIY-Flow-Bench/issues/71#issuecomment-893104615        
      break;  
 
      case FILE_DELETE: // HEADER: 9 
        _message.DebugPrintLn("File Delete: ");
        socketData.file_name = messageData["FILENAME"].as<String>();  
        _message.DebugPrintLn(socketData.file_name);
        if(SPIFFS.exists(socketData.file_name)){
          SPIFFS.remove(socketData.file_name);
        }  else {
          _message.DebugPrintLn("Delete Failed: ");      
          _message.DebugPrintLn(socketData.file_name);  
        } 
        SendWebSocketMessage(_webserver.getFileListJSON());      
      break;  

      case FILE_UPLOAD: // HEADER: 10 
                
      break;

      case START_BENCH: // HEADER: 11 
        _message.DebugPrintLn("Start Bench");
        status.liveStream = true;
        digitalWrite(VAC_BANK_1, HIGH);
      break;     
      
      case STOP_BENCH: // HEADER: 12 
        _message.DebugPrintLn("Stop Bench");
        status.liveStream = false;   
        digitalWrite(VAC_BANK_1, LOW);   
      break;  

      case LEAK_CAL: // HEADER: 13 
        _message.DebugPrintLn("Leak Test Calibration");
        if (_hardware.benchIsRunning()){          
          _calibration.setLeakTestPressure();
          // send new calibration to the browser
          //_webserver.SendWebSocketMessage("flow offset");
        } else {
          _message.Handler(LANG_RUN_BENCH_TO_CALIBRATE);        
        }
      break;         
      
      case GET_CAL: // HEADER: 14 
        _message.DebugPrintLn("Send Calibration");
        jsonMessage = _calibration.loadCalibration();
        jsonMessage["HEADER"] = GET_CAL; 
        serializeJson(jsonMessage, jsonMessageString);
        SendWebSocketMessage(jsonMessageString);            
      break;     

      default:
        _message.DebugPrintLn("Unrecognised Websocket Header");
        _message.DebugPrint("HEADER: '");
        _message.DebugPrint(String(header));
        _message.DebugPrintLn("'");
      break;

    }

    if (strcmp((char*)data, "getValues") == 0) {
      SendWebSocketMessage(_webserver.getDataJSON());
    }
    
  }
}




/***********************************************************
* Websocket Event Listener
***/
void Webserver::ReceiveWebSocketMessage(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {

  Messages _message;
  
  String clientIP;
  
  switch (type) {
    case WS_EVT_CONNECT:
      clientIP = client->remoteIP().toString().c_str();
      _message.DebugPrint("WebSocket client connected from  ");
      _message.DebugPrintLn(clientIP);
      break;
    case WS_EVT_DISCONNECT:
      _message.DebugPrintLn("WebSocket client disconnected");
      break;
    case WS_EVT_DATA:
      Webserver::ProcessWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}


/***********************************************************
* Byte Decode 
***/
String Webserver::byteDecode(size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}



/***********************************************************
* Process File Upload 
*
* Redirects browser back to Upload modal unless upload is index file
***/
void Webserver::ProcessUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {

  Messages _message;
  extern struct FileUploadData fileUploadData;
  String redirectURL;
  
  if (SPIFFS.exists (filename) ) {
    SPIFFS.remove (filename);
  }
  uint32_t freespace = SPIFFS.totalBytes() - SPIFFS.usedBytes();
  
  if(!filename.startsWith("/")) filename = "/"+filename;
  if(!index && !fileUploadData.upload_error){
    _message.DebugPrintLn((String)"UploadStart: " + filename);
    request->_tempFile = SPIFFS.open(filename, "w");
  }
  
  // Set redirect to file Upload modal unless uploading the index file
  if (filename == String("/index.html.gz")){
    redirectURL = "/";
  } else {
    redirectURL = "/?modal=upload";
  }
  
  if(len) {   
    fileUploadData.file_size += len;
    if (fileUploadData.file_size > freespace) {  
      // TODO: _message.DebugPrintLn('Upload rejected, not enough space');
      fileUploadData.upload_error = true;
    } else {
      // TODO: _message.DebugPrintLn('Writing file: \"' + String(filename) + '\" index=' + String(index) + ' len=' + String(len));
      request->_tempFile.write(data,len);
    }
  }
  
  if(final){
    _message.DebugPrintLn((String)"UploadEnd: " + filename + "," + fileUploadData.file_size);
    request->_tempFile.close();
    request->redirect(redirectURL);
  }
}



String processor(const String& var) { return String("Waiting Data"); }

/***********************************************************
* INITIALISE SERVER
***/
void Webserver::Initialise() {
    
    extern struct ConfigSettings config;
    extern DeviceStatus status;
    extern AsyncWebServer server;
    extern AsyncWebSocket ws;
    extern AsyncEventSource events;
    
    String index_html = "<!DOCTYPE HTML><html lang='en'><HEAD><title>DIY Flow Bench</title><meta name='viewport' content='width=device-width, initial-scale=1'> <script>function onFileUpload(event){this.setState({file:event.target.files[0]});const{file}=this.state;const data=new FormData;data.append('data',file);fetch('/upload',{method:'POST',body:data}).catch(e=>{console.log('Request failed',e);});}</script> <style>body,html{height:100%;margin:0;font-family:Arial;font-size:22px}a:link{color:#0A1128;text-decoration:none}a:visited,a:active{color:#0A1128;text-decoration:none}a:hover{color:#666;text-decoration:none}.headerbar{overflow:hidden;background-color:#0A1128;text-align:center}.headerbar h1 a:link, .headerbar h1 a:active, .headerbar h1 a:visited, .headerbar h1 a:hover{color:white;text-decoration:none}.align-center{text-align:center}.file-upload-button{padding:12px 0px;text-align:center}.button{display:inline-block;background-color:#008CBA;border:none;border-radius:4px;color:white;padding:12px 12px;text-decoration:none;font-size:22px;margin:2px;cursor:pointer;width:150px}#footer{clear:both;text-align:center}.file-upload-button{padding:12px 0px;text-align:center}input[type='file']{display:none}</style></HEAD><BODY><div class='headerbar'><h1><a href='/' >DIY Flow Bench</a></h1></div> <br><div class='align-center'><p>Welcome to the DIY Flow Bench.</p><p>Please upload the index.html.gz file to get started.</p> <br><form method='POST' action='/upload' enctype='multipart/form-data'> <label for='data' class='file-upload-button button'>Select File</label> <input id='data' type='file' name='wtf'/> <input class='button file-submit-button' type='submit' value='Upload'/></form></div> <br><div id='footer'><a href='https://diyflowbench.com' target='new'>DIYFlowBench.com</a></div> <br></BODY></HTML>";
    
    Messages _message;
    Settings _settings;
    Calibration _calibration;

    // Serial
    Serial.begin(config.serial_baud_rate);      
    _message.SerialPrintLn("\r\n");
    _message.SerialPrintLn("DIY Flow Bench ");
    _message.SerialPrint("Version: ");
    _message.SerialPrintLn(RELEASE);
    _message.SerialPrint("Version: ");
    _message.SerialPrintLn(BUILD_NUMBER);

    // Filesystem
    _message.SerialPrintLn("File System Initialisation... ");
    if (SPIFFS.begin()){
      _message.SerialPrintLn("done.");
    } else {
      _message.SerialPrintLn("failed.");
      #if defined FORMAT_FILESYSTEM_IF_FAILED
          SPIFFS.format();
          _message.SerialPrintLn("File System Formatted ");
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
 
    _message.SerialPrintLn("===== File system info =====");
    _message.SerialPrint("Total space:      ");
    _message.SerialPrintLn(byteDecode(status.spiffs_mem_size));
    _message.SerialPrint("Total space used: ");
    _message.SerialPrintLn(byteDecode(status.spiffs_mem_used)); 
    _message.SerialPrintLn("");

    // API
    #if defined API_ENABLED
      _message.SerialPrintLn("Serial API Enabled");
    #endif

    // WiFi    
    status.apMode = false;
    unsigned long timeOut;
    timeOut = millis() + config.wifi_timeout;
    _message.SerialPrintLn("Connecting to WiFi");
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.wifi_ssid.c_str(), config.wifi_pswd.c_str());
    while (WiFi.status() != WL_CONNECTED && millis() < timeOut) {
      delay(1000);
      _message.SerialPrint(".");
    } 
    if (WiFi.status() == WL_CONNECTED) {  
      // Connection success     
      _message.SerialPrint("Connected to ");
      _message.SerialPrintLn(config.wifi_ssid);
      status.local_ip_address = WiFi.localIP().toString();
      _message.SerialPrint("IP address: ");
      _message.SerialPrintLn(status.local_ip_address);
    } else {
      // Did not connect - Go into AP Mode
      status.apMode = true;
      _message.SerialPrintLn("failed!");
      _message.SerialPrint("Creating WiFi AP (");
      _message.SerialPrint(config.wifi_ap_ssid);
      _message.SerialPrintLn(")");
      WiFi.softAP(config.wifi_ap_ssid.c_str(), config.wifi_ap_pswd.c_str());
      status.local_ip_address = WiFi.softAPIP().toString();
      _message.SerialPrint("AP IP address: ");
      _message.SerialPrintLn(status.local_ip_address);     
    }

    // Set up Multicast DNS
    if(!MDNS.begin(config.hostname.c_str())) {
      _message.SerialPrintLn("Error starting mDNS");
    } else {
      _message.SerialPrint("Access via: http://");      
      _message.SerialPrint(config.hostname.c_str());      
      _message.SerialPrintLn(".local");      
    }

    // Debug request handler
    server.on("/debug", HTTP_POST, [](AsyncWebServerRequest *request){
      Messages _message;
      _message.SerialPrint("Debug Mode ");
      config.debug_mode = true;
      request->redirect("/");
    });
   
    // Upload request handler
    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
      Messages _message;
      _message.SerialPrint("Upload Request Called ");
      request->redirect("/?modal=upload");
    }, ProcessUpload);

    // Root URL request handler
    server.on("/", HTTP_ANY, [index_html](AsyncWebServerRequest *request){    
      //extern String index_html;
      if (SPIFFS.exists("/index.html.gz")){
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html.gz", "text/html", false);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
      } else if (SPIFFS.exists("/index.html")) {
        request->send(SPIFFS, "/index.html", "text/html");
      } else {
        request->send(200, "text/html", index_html); 
      }
    });

    // Index page request handler
    server.on("/index.html", HTTP_ANY, [index_html](AsyncWebServerRequest *request){
      if (SPIFFS.exists("/index.html.gz")){
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html.gz", "text/html", false);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
      } else if (SPIFFS.exists("/index.html")) {
        request->send(SPIFFS, "/index.html", "text/html");
      } else {
        request->send(200, "text/html", index_html); 
      }
    });

    // Style sheet request handler
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/style.css", "text/css");
    });
    
    // Javascript file request handler
    server.on("/javascript.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/javascript.js", "text/javascript");
    });
    
    // Download request handler
    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){              
          Messages _message;
          String downloadFilename = request->url();
          downloadFilename.remove(0,9);
          _message.SerialPrint("Request Delete File: ");
          _message.SerialPrint(downloadFilename);
          request->send(SPIFFS, downloadFilename, String(), true);
    });

    // NOTE" this acts as a catch-all and allows us to process any request not specifically declared above
    server.onNotFound( []( AsyncWebServerRequest * request ) {
      Messages _message;
      _message.SerialPrintLn("REQUEST: NOT_FOUND: ");
      if (request->method() == HTTP_GET)
        _message.SerialPrintLn("REQUEST: GET");
      else if (request->method() == HTTP_POST)
        _message.SerialPrintLn("REQUEST: POST");
      else if (request->method() == HTTP_DELETE)
        _message.SerialPrintLn("REQUEST: DELETE");
      else if (request->method() == HTTP_PUT)
        _message.SerialPrintLn("REQUEST: PUT");
      else if (request->method() == HTTP_PATCH)
        _message.SerialPrintLn("REQUEST: PATCH");
      else if (request->method() == HTTP_HEAD)
        _message.SerialPrintLn("REQUEST: HEAD");
      else if (request->method() == HTTP_OPTIONS)
        _message.SerialPrintLn("REQUEST: OPTIONS");
      else
        _message.SerialPrintLn("REQUEST UNKNOWN");
        //_message.SerialPrint(" http://%s%s\n", request->host().c_str(), request->url().c_str());
        request->send( 404, "text/plain", "Not found." );
    });

    ws.onEvent(ReceiveWebSocketMessage);
    
    server.onFileUpload(ProcessUpload);
    server.addHandler(&ws);
    server.begin();
    _message.SerialPrintLn("Server Running");

}


/***********************************************************
* Decode Message Header
* Pulls HEADER value from JSON string
***/
 int Webserver::decodeMessageHeader (char *data) {
   
  Messages _message;
  
  int header = 0;

  StaticJsonDocument<1024> messageData;
  DeserializationError error = deserializeJson(messageData, data);
  if (!error){
    header = messageData["HEADER"].as<int>();  
    _message.DebugPrintLn("Decoded Message Header: ");
    _message.DebugPrintLn(String(header));
  } else {
    _message.DebugPrintLn("Webserver::decodeMessageHeader ERROR");
  }
  
  return header;

}




/***********************************************************
* Get SPIFFS File List in JSON format
***/
 String Webserver::getFileListJSON () {

    StaticJsonDocument<1024> dataJson;    
    dataJson["HEADER"] = FILE_LIST;
    Messages _message;

    _message.DebugPrintLn("Filesystem contents:");
    FILESYSTEM.begin();
    File root = FILESYSTEM.open("/");
    File file = root.openNextFile();
    while(file){
        String fileName = file.name();
        size_t fileSize = file.size();
  
        dataJson[fileName] = String(fileSize);

        _message.DebugPrint( fileName.c_str());
        _message.DebugPrint(" : ");
        _message.DebugPrintLn( byteDecode(fileSize).c_str());
        file = root.openNextFile();
    }  

    char jsonString[1024];
    serializeJson(dataJson, jsonString);
    return jsonString;
    
 }


/***********************************************************
* Get System Status in JSON format
***/
String Webserver::getSystemStatusJSON() {
  
  extern DeviceStatus status;
  StaticJsonDocument<1024> dataJson;    

  dataJson["HEADER"] = SYS_STATUS;
  dataJson["SPIFFS_MEM_SIZE"] = status.spiffs_mem_size;
  dataJson["SPIFFS_MEM_USED"] = status.spiffs_mem_used;
  dataJson["LOCAL_IP_ADDRESS"] = status.local_ip_address;
  dataJson["BOARDTYPE"] = status.boardType;
  dataJson["MAF_SENSOR"] = status.mafSensor;
  dataJson["PREF_SENSOR"] = status.prefSensor;
  dataJson["TEMP_SENSOR"] = status.tempSensor;
  dataJson["RELH_SENSOR"] = status.relhSensor;
  dataJson["BARO_SENSOR"] = status.baroSensor;
  dataJson["PITOT_SENSOR"] = status.pitotSensor;
  dataJson["BOOT_TIME"] = status.boot_time;
 
  char jsonString[1024];
  serializeJson(dataJson, jsonString);
  return jsonString;

}




/***********************************************************
* Get JSON Data
*
* Package up current bench data into JSON string
***/
String Webserver::getDataJSON() { 

  extern struct DeviceStatus status;
  extern struct ConfigSettings config;
  
  Maths _maths;
  Hardware _hardware;
  
  float mafFlowCFM = _maths.calculateFlowCFM();
  float refPressure = _maths.calculateRefPressure(INWG);   
  StaticJsonDocument<1024> dataJson;    

  dataJson["HEADER"] = GET_FLOW_DATA;

  dataJson["STATUS_MESSAGE"] = String(status.statusMessage);

  // Flow Rate
  if (mafFlowCFM > config.min_flow_rate)
  {
    dataJson["FLOW"] = String(mafFlowCFM);        
  } else {
    dataJson["FLOW"] = String(0);        
  }

  // Temperature
  dataJson["TEMP"] = String(_maths.calculateTemperature(DEGC));        
  
  // Baro
  dataJson["BARO"] = String(_maths.calculateBaroPressure(KPA));        
  
  // Relative Humidity
  dataJson["RELH"] = String(_maths.calculateRelativeHumidity(PERCENT));

  // Pitot
  // NOT USED: double pitotPressure = _maths.calculatePitotPressure(INWG);
  // Pitot probe displays as a percentage of the reference pressure
  double pitotPercentage = (_maths.calculatePitotPressure(INWG) / refPressure);
  dataJson["PITOT"] = String(pitotPercentage);
  
  // Reference pressure
  dataJson["PREF"] = String(refPressure);
  
  // Adjusted Flow
  // get the desired bench test pressure
//    double desiredRefPressureInWg = menuARef.getCurrentValue(); //TODO:: Add ref pressure setting to UI & Config
  // convert from the existing bench test
//    double adjustedFlow = convertFlowDepression(refPressure, desiredRefPressureInWg, mafFlowCFM);
  // Send it to the display
//    dataJson["AFLOW"] = String(adjustedFlow);

  // Version and build
  String versionNumberString = String(MAJOR_VERSION) + '.' + String(MINOR_VERSION);
  String buildNumberString = String(BUILD_NUMBER);
  dataJson["RELEASE"] = String(RELEASE);
  dataJson["BUILD_NUMBER"] = String(BUILD_NUMBER);

  int supplyMillivolts = _hardware.getSupplyMillivolts();
  
  // Ref Pressure Voltage
  int refPressRaw = analogRead(REF_PRESSURE_PIN);
  double refMillivolts = (refPressRaw * (supplyMillivolts / 4095.0)) * 1000;
  dataJson["PREF_MV"] = String(refMillivolts);

  // Maf Voltage
  int mafFlowRaw = analogRead(MAF_PIN);
  double mafMillivolts = (mafFlowRaw * (supplyMillivolts / 4095.0)) * 1000;
  dataJson["MAF_MV"] = String(mafMillivolts);

  // Pitot Voltage
  int pitotRaw = analogRead(PITOT_PIN);
  double pitotMillivolts = (pitotRaw * (supplyMillivolts / 4095.0)) * 1000;
  dataJson["PITOT_MV"] = String(pitotMillivolts);

  char jsonString[1024];
  serializeJson(dataJson, jsonString);
  return jsonString;

}




/***********************************************************
* onBody - serve gzipped page if available 
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
* write JSON string to file
***/
void Webserver::writeJSONFile(String data, String filename) {

  Messages _message;

  StaticJsonDocument<1024> jsonData;
  DeserializationError error = deserializeJson(jsonData, data);
  if (!error){
    _message.DebugPrintLn("Writing JSON file...");  
    File outputFile = SPIFFS.open(filename, FILE_WRITE);
    serializeJsonPretty(jsonData, outputFile);
    outputFile.close();
  } else {
    _message.DebugPrintLn("Webserver::writeJSONFile ERROR");
  }

  



}






/***********************************************************
 * load JSON file 
 ***/
StaticJsonDocument<1024> Webserver::loadJSONFile(String filename) {
  
  Messages _message;
  
  // Allocate the memory pool on the stack.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<1024> jsonData;

  if (SPIFFS.exists(filename)){
    File jsonFile = SPIFFS.open(filename, FILE_READ);
  
    if (!jsonFile) {
        _message.Handler(LANG_ERROR_LOADING_FILE);
        _message.DebugPrintLn("Failed to open file for reading");
    } else {
        size_t size = jsonFile.size();
        if (size > 1024) {
          #ifdef DEBUG 
            _message.DebugPrintLn("Config file size is too large");
          #endif
        }

        DeserializationError error = deserializeJson(jsonData, jsonFile);
        if (error) {
          _message.DebugPrintLn("loadJSONFile->deserializeJson() failed: ");
          _message.DebugPrintLn(error.f_str());
        }
        
        jsonFile.close();
        return jsonData;
        
        // String jsonString;
        // serializeJson(jsonData, jsonString);
        // return jsonString;
    }    
    jsonFile.close();
  } else {
    _message.DebugPrintLn("File missing");
  }
  return jsonData;
}
