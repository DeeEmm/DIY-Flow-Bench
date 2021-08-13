/****************************************
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
#include "structs.h"
#include "pins.h"
#include "settings.h"
#include "hardware.h"
#include "messages.h"
#include "maths.h"
// #include LANGUAGE_FILE


Webserver::Webserver() {

}





/****************************************
 * Push Server Data to client via websocket
 ***/
void Webserver::SendWebSocketMessage(String jsonValues) {
  
  extern AsyncWebSocket ws;
  
  ws.textAll(String(jsonValues));

}




void merge(JsonObject dest, JsonObjectConst src) {
   for (auto kvp : src) {
     dest[kvp.key()] = kvp.value();
   }
}

// void deepmerge(JsonVariant dst, JsonVariantConst src) {
//   if (src.is<JsonObject>()) {
//     for (auto kvp : src.as<JsonObject>()) {
//       merge(dst.getOrAddMember(kvp.key()), kvp.value());
//     }
//   } else {
//     dst.set(src);
//   }
// }



/****************************************
 * Process Websocket Message
 ***/
void Webserver::ProcessWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  
  extern struct WebsocketData socketData;
  extern struct ConfigSettings settings;
  extern struct DeviceStatus status;
  
  Settings _settings;
  Hardware _hardware;
  Webserver _webserver;
  
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  
  StaticJsonDocument<1024> messageData;
  DeserializationError err = deserializeJson(messageData, data);
  if (err) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(err.c_str());
  }
  
  int header = messageData["HEADER"].as<int>(); 

  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    
    data[len] = 0;
    StaticJsonDocument<1024> configMessage;
    StaticJsonDocument<1024> configHeader;  
    StaticJsonDocument<1024> concatMessage;  
    
    String configMessageString;

    Serial.print("HEADER: ");
    Serial.println(header);

    switch (header) {

      case GET_FLOW_DATA:
        Serial.println("Get Data");
        SendWebSocketMessage(_webserver.getDataJSON());
      break;      

      case SAVE_CONFIG:
        Serial.println("Save Config");
        _settings.saveConfig((char*)data);
        
      case LOAD_CONFIG:
        Serial.println("Load Config");
        configMessage = _settings.LoadConfig();
        configMessage["HEADER"] = LOAD_CONFIG; 
        serializeJson(configMessage, configMessageString);
        SendWebSocketMessage(configMessageString);      
      break;      

      case CALIBRATE:
        Serial.println("Calibrate");
        if (_hardware.benchIsRunning()){
// TODO:          setCalibrationOffset();         
// TODO:          //calibration.flow_offset                
        }

      break;      

      case FILE_LIST:
        Serial.println("File List");
        SendWebSocketMessage(_webserver.getFileListJSON());      
      break;      

      case SYS_STATUS:
        Serial.println("Send Status");
        SendWebSocketMessage(_webserver.getSystemStatusJSON());      
      break;      


      case FILE_DELETE:
        Serial.print("File Delete: ");
        socketData.file_name = messageData["FILENAME"].as<String>();  
        Serial.println(socketData.file_name);
        if(SPIFFS.exists(socketData.file_name)){
          SPIFFS.remove(socketData.file_name);
        }  else {
          Serial.print("Delete Failed: ");      
          Serial.println(socketData.file_name);  
        } 
        SendWebSocketMessage(_webserver.getFileListJSON());      
      break;      

      case FILE_DOWNLOAD:
        // NOTE: https://github.com/DeeEmm/DIY-Flow-Bench/issues/71#issuecomment-893104615        
      break;      
      
      case START_BENCH:
        status.liveStream = true;
        // TODO: Relay output active
      break;
      
      case STOP_BENCH:
        status.liveStream = false;   
        // TODO: Relay output disabled   
      break;

      default:
        Serial.println("Unrecognised Message");
      break;

    }


    if (strcmp((char*)data, "getValues") == 0) {
      SendWebSocketMessage(_webserver.getDataJSON());
    }
    
  }
}




/****************************************
 * Websocket Event Listener
 ***/
void Webserver::ReceiveWebSocketMessage(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      Webserver::ProcessWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}


/****************************************
 * BYTE DECODE 
 ***/
String Webserver::byteDecode(size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}



/****************************************
 * UPLOAD FILE
 ***/
void Webserver::ProcessUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {

  extern struct FileUploadData fileUploadData;
  
  if (SPIFFS.exists (filename) ) {
    SPIFFS.remove (filename);
  }
  uint32_t freespace = SPIFFS.totalBytes() - SPIFFS.usedBytes();
  
  if(!filename.startsWith("/")) filename = "/"+filename;
  if(!index && !fileUploadData.upload_error){
    Serial.println((String)"UploadStart: " + filename);
    request->_tempFile = SPIFFS.open(filename, "w");
  }
  if(len) {   
    fileUploadData.file_size += len;
    if (fileUploadData.file_size > freespace) {  
      Serial.println( 'Upload rejected, not enough space');
      fileUploadData.upload_error = true;
    } else {
      Serial.println('Writing file: \"' + String(filename) + '\" index=' + String(index) + ' len=' + String(len));
      request->_tempFile.write(data,len);
    }
  }
  if(final){
    Serial.println((String)"UploadEnd: " + filename + "," + fileUploadData.file_size);
    request->_tempFile.close();
    request->redirect("/?modal=upload");
  }
}





/****************************************
 * INITIALISE SERVER
 ***/
void Webserver::Initialise() {
    
    extern struct ConfigSettings config;
    extern DeviceStatus status;
    extern AsyncWebServer server;
    extern AsyncWebSocket ws;
    extern AsyncEventSource events;
    
    Messages _message;
    

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
    Serial.println("Connecting to WiFi");
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.wifi_ssid.c_str(), config.wifi_pswd.c_str());
    while (WiFi.status() != WL_CONNECTED && millis() < timeOut) {
      delay(1000);
      Serial.print(".");
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
    server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request){    
      if (SPIFFS.exists("/index.html.gz")){
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html.gz", "text/html", false);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
      } else {
        request->send(SPIFFS, "/index.html", "text/html");
      }
      
    });

    // Index page request handler
    server.on("/index.html", HTTP_ANY, [](AsyncWebServerRequest *request){
      if (SPIFFS.exists("/index.html.gz")){
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html.gz", "text/html", false);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
      } else {
        request->send(SPIFFS, "/index.html", "text/html");
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


/****************************************
 * Decode Message Header
 * Pulls HEADER value from JSON string
 ***/
 int Webserver::decodeMessageHeader (char *data) {

  StaticJsonDocument<1024> messageData;
  DeserializationError error = deserializeJson(messageData, data);
  int header = messageData["HEADER"].as<int>();  

  Serial.print("Decoded Message Header: ");
  Serial.println(header, DEC);

  return header;

}




/****************************************
 * Get SPIFFS File List in JSON format
 ***/
 String Webserver::getFileListJSON () {

    StaticJsonDocument<1024> dataJson;    
    dataJson["HEADER"] = FILE_LIST;

    Serial.println("Filesystem contents:");
    FILESYSTEM.begin();
    File root = FILESYSTEM.open("/");
    File file = root.openNextFile();
    while(file){
        String fileName = file.name();
        size_t fileSize = file.size();
  
        dataJson[fileName] = String(fileSize);

        Serial.print( fileName.c_str());
        Serial.print(" : ");
        Serial.println( byteDecode(fileSize).c_str());
        file = root.openNextFile();
    }  

    char jsonString[1024];
    serializeJson(dataJson, jsonString);
    return jsonString;
    
 }


/****************************************
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




/****************************************
* Get JSON Data
*
* Package up current bench data into JSON string
***/
String Webserver::getDataJSON() { 

  extern struct DeviceStatus status;
  extern struct ConfigSettings config;
  
  Maths _maths;
  
  // TODO: Check scope of these... this->?????
  float mafFlowCFM = _maths.calculateMafFlowCFM();
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
  double pitotPressure = _maths.calculatePitotPressure(INWG);
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

  // Ref Pressure Voltage
  int refPressRaw = analogRead(REF_PRESSURE_PIN);
  double refMillivolts = (refPressRaw * (5.0 / 1024.0)) * 1000;
  dataJson["PREF_MV"] = String(refMillivolts);

  // Maf Voltage
  int mafFlowRaw = analogRead(MAF_PIN);
  double mafMillivolts = (mafFlowRaw * (5.0 / 1024.0)) * 1000;
  dataJson["MAF_MV"] = String(mafMillivolts);

  // Pitot Voltage
  int pitotRaw = analogRead(PITOT_PIN);
  double pitotMillivolts = (pitotRaw * (5.0 / 1024.0)) * 1000;
  dataJson["PITOT_MV"] = String(pitotMillivolts);

  char jsonString[1024];
  serializeJson(dataJson, jsonString);
  return jsonString;

}




/****************************************
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


