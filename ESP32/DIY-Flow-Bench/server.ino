/****************************************
 * The DIY Flow Bench project
 * https://diyflowbench.com
 * 
 * server.ino - Webserver functions
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
 * DECLARE VARS
 ***/

bool apMode = false;
#define FILESYSTEM SPIFFS

void sendIndexPage();
void uploadFile();
void onBody();

/****************************************
 * BYTE DECODE 
 ***/
String byteDecode(size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}



/****************************************
 * UPLOAD FILE
 ***/
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  
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
    request->redirect("/");
  }
}



/****************************************
 * INITIALISE SERVER
 ***/
void initialiseServer() {

    extern ConfigSettings config;
    extern DeviceStatus status;

    // Serial
    Serial.begin(config.serial_baud_rate);      
    Serial.println("\r\n");
    Serial.println("DIY Flow Bench ");
    Serial.print("Version: ");
    Serial.println(RELEASE);
    Serial.print("Version: ");
    Serial.println(BUILD_NUMBER);

    // Filesystem
    Serial.print(F("File System Inizializing "));
    if (SPIFFS.begin()){
      Serial.println(F("done."));
    } else {
      Serial.println(F("failed."));
      #if defined FORMAT_FILESYSTEM_IF_FAILED
          SPIFFS.format();
          Serial.println("File System Formatted ");
      #endif
    }
    // Filesystem info
    status.spiffs_mem_size = SPIFFS.totalBytes();
    status.spiffs_mem_used = SPIFFS.usedBytes();
 
    Serial.println("===== File system info =====");
 
    Serial.print("Total space:      ");
    Serial.print(byteDecode(status.spiffs_mem_size));
    Serial.println("byte");
 
    Serial.print("Total space used: ");
    Serial.print(byteDecode(status.spiffs_mem_used));
    Serial.println("byte");
 
    Serial.println();

    // API
    #if defined API_ENABLED
      Serial.println("Serial API Enabled");
    #endif

    // WiFi    
    Serial.println("Connecting to WiFi");
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.wifi_ssid.c_str(), config.wifi_pswd.c_str());
    while (WiFi.status() != WL_CONNECTED && config.wifi_timeout > 0) {
      delay(1000);
      Serial.print(".");
    } 
    if (WiFi.status() == WL_CONNECTED) {  
      // Connection success     
      Serial.print("Connected to ");
      Serial.println(config.wifi_ssid);
      status.local_ip_address = WiFi.localIP().toString();
      Serial.print("IP address: ");
      Serial.println(status.local_ip_address);
    } else {
      // Did not connect - Go into AP Mode
      apMode = true;
      Serial.println("failed!");
      Serial.print("Creating WiFi AP (");
      Serial.print(config.wifi_ap_ssid);
      Serial.println(")");
      WiFi.softAP(config.wifi_ap_ssid.c_str(), config.wifi_ap_pswd.c_str());
      status.local_ip_address = WiFi.softAPIP().toString();
      Serial.print("AP IP address: ");
      Serial.println(status.local_ip_address);     
    }

    // Set up Multicast DNS
    if(!MDNS.begin(config.hostname.c_str())) {
      Serial.println("Error starting mDNS");
    } else {
      Serial.print("Access via: http://");      
      Serial.print(config.hostname.c_str());      
      Serial.println(".local");      
    }

    // Server Request handlers     
    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
      Serial.print("Upload Request Called ");
      request->send(200);
    }, handleUpload);

    server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request){    
      if (SPIFFS.exists("/index.html.gz")){
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html.gz", "text/html", false);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
      } else {
        request->send(SPIFFS, "/index.html", "text/html");
      }
      
    });

    server.on("/index.html", HTTP_ANY, [](AsyncWebServerRequest *request){
      if (SPIFFS.exists("/index.html.gz")){
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html.gz", "text/html", false);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
      } else {
        request->send(SPIFFS, "/index.html", "text/html");
      }
    });

    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/style.css", "text/css");
    });
    
    server.on("/javascript.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/javascript.js", "text/javascript");
    });
    
    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){              
          String downloadFilename = request->url();
          downloadFilename.remove(0,9);
          Serial.print("Request Delete File: ");
          Serial.println(downloadFilename);
          request->send(SPIFFS, downloadFilename, String(), true);
    });

    // NOTE" this acts as a catch-all and allows us to process any request not specifically declared above
    server.onNotFound( []( AsyncWebServerRequest * request )
    {
      Serial.printf("NOT_FOUND: ");
      if (request->method() == HTTP_GET)
        Serial.printf("GET");
      else if (request->method() == HTTP_POST)
        Serial.printf("POST");
      else if (request->method() == HTTP_DELETE)
        Serial.printf("DELETE");
      else if (request->method() == HTTP_PUT)
        Serial.printf("PUT");
      else if (request->method() == HTTP_PATCH)
        Serial.printf("PATCH");
      else if (request->method() == HTTP_HEAD)
        Serial.printf("HEAD");
      else if (request->method() == HTTP_OPTIONS)
        Serial.printf("OPTIONS");
      else
        Serial.printf("UNKNOWN");
      Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());
      request->send( 404, "text/plain", "Not found." );
    });

    ws.onEvent(onEvent);
    server.onFileUpload(handleUpload);
    server.addHandler(&ws);
    server.begin();
    Serial.println("Server Running");

}


/****************************************
 * Push Server Data to client via websocket
 ***/
void notifyClients(String jsonValues) {
  ws.textAll(String(jsonValues));
}


/****************************************
 * Decode Message Schema
 * Pulls SCHEMA value from JSON string
 ***/
 int decodeMessageSchema (char *data) {

  StaticJsonDocument<1024> messageData;
  DeserializationError error = deserializeJson(messageData, data);
  int schema = messageData["SCHEMA"].as<int>();  

  Serial.print("Decoded Message Schema: ");
  Serial.println(schema, DEC);

  return schema;

}



/****************************************
 * Process Websocket Message
 ***/
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  
  extern WebsocketData socketData;

  AwsFrameInfo *info = (AwsFrameInfo*)arg;
    
  StaticJsonDocument<1024> messageData;
  DeserializationError err = deserializeJson(messageData, data);
  if (err) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(err.c_str());
  }
  
  int schema = messageData["SCHEMA"].as<int>(); 

  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    
    data[len] = 0;

    Serial.print("SCHEMA: ");
    Serial.println(schema);

    switch (schema) {

      case GET_FLOW_DATA:
        Serial.println("Get Data");
        notifyClients(getDataJson());
      break;      

      case CONFIG:
      case LOAD_CONFIG:
        Serial.println("Load Config");
        notifyClients(loadConfig());      
      break;      

      case CALIBRATE:
        Serial.println("Calibrate");
        if (benchIsRunning()){
// TODO:          setCalibrationOffset();                         
        }

      break;      

      case FILE_LIST:
        Serial.println("File List");
        notifyClients(getFileListJSON());      
      break;      

      case SYS_STATUS:
        Serial.println("Send Status");
        notifyClients(getSystemStatusJSON());      
      break;      

      case SAVE_CONFIG:
        Serial.println("Save Config");
        saveConfig((char*)data);
        notifyClients(loadConfig());      
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
        notifyClients(getFileListJSON());      
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
      notifyClients(getDataJson());
    }
    
  }
}

/****************************************
 * Websocket Event Listener
 ***/
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}



/****************************************
 * Get SPIFFS File List in JSON format
 ***/
 String getFileListJSON () {

    StaticJsonDocument<1024> dataJson;    
    dataJson["SCHEMA"] = FILE_LIST;

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
String getSystemStatusJSON() {
  
  extern DeviceStatus status;
  StaticJsonDocument<1024> dataJson;    

  dataJson["SCHEMA"] = SYS_STATUS;
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
 * onBody - serve gzipped page if available 
 ***/
void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  if (SPIFFS.exists("/index.html.gz")){
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html.gz", "text/html", false);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  } else {
    request->send(SPIFFS, "/index.html", "text/html");
  }
}


