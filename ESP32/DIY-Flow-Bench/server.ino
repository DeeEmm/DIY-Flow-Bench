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


/****************************************
 * Byte Decode 
 ***/
String byteDecode(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}



/****************************************
 * INITIALISE SERVER
 ***/
void initialiseServer() {

    extern ConfigSettings config;
    extern DeviceStatus stats;

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
    stats.spiffs_mem_size = SPIFFS.totalBytes();
    stats.spiffs_mem_used = SPIFFS.usedBytes();
 
    Serial.println("===== File system info =====");
 
    Serial.print("Total space:      ");
    Serial.print(stats.spiffs_mem_size);
    Serial.println("byte");
 
    Serial.print("Total space used: ");
    Serial.print(stats.spiffs_mem_used);
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
      stats.local_ip_address = WiFi.localIP().toString();
      Serial.print("IP address: ");
      Serial.println(stats.local_ip_address);
    } else {
      // Did not connect - Go into AP Mode
      apMode = true;
      Serial.println("failed!");
      Serial.print("Creating WiFi AP (");
      Serial.print(config.wifi_ap_ssid);
      Serial.println(")");
      WiFi.softAP(config.wifi_ap_ssid.c_str(), config.wifi_ap_pswd.c_str());
      stats.local_ip_address = WiFi.softAPIP().toString();
      Serial.print("AP IP address: ");
      Serial.println(stats.local_ip_address);     
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
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/index.html", "text/html");
    });

    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/style.css", "text/css");
    });
    server.on("/javascript.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/javascript.js", "text/javascript");
    });
  
    ws.onEvent(onEvent);
    server.addHandler(&ws);

    server.begin();

    Serial.println("Server Running");

}




/****************************************
 * Push Server Data to client
 ***/
void notifyClients(String jsonValues) {
  ws.textAll(String(jsonValues));
}


/****************************************
 * Decode Message Schema
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

  AwsFrameInfo *info = (AwsFrameInfo*)arg;

  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;

  Serial.print("(char*)data: ");
  Serial.println((char*)data);

    switch (decodeMessageSchema((char*)data)) {

      case GET_FLOW_DATA:
        Serial.println("Get Data");
        notifyClients(getDataJson());
      break;      

      case CONFIG:
        Serial.println("Load Config");
        notifyClients(loadConfig());      
      break;      

      case CALIBRATE:
        Serial.println("Calibrate");
        // TODO - Do calibration
//        notifyClients(calibrate());      
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
        //notifyClients(loadConfig());      
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
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
 void *arg, uint8_t *data, size_t len) {
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
 * Get File List in JSON format
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
String getSystemStatusJSON(){
  
  extern DeviceStatus stats;
  StaticJsonDocument<1024> dataJson;    

  dataJson["SCHEMA"] = SYS_STATUS;
  dataJson["SPIFFS_MEM_SIZE"] = stats.spiffs_mem_size;
  dataJson["SPIFFS_MEM_USED"] = stats.spiffs_mem_used;
  dataJson["LOCAL_IP_ADDRESS"] = stats.local_ip_address;
  dataJson["BOARDTYPE"] = stats.boardType;
  dataJson["MAF_SENSOR"] = stats.mafSensor;
  dataJson["PREF_SENSOR"] = stats.prefSensor;
  dataJson["TEMP_SENSOR"] = stats.tempSensor;
  dataJson["RELH_SENSOR"] = stats.relhSensor;
  dataJson["BARO_SENSOR"] = stats.baroSensor;
  dataJson["PITOT_SENSOR"] = stats.pitotSensor;
  dataJson["BOOT_TIME"] = stats.boot_time;
 
  char jsonString[1024];
  serializeJson(dataJson, jsonString);
  return jsonString;

}