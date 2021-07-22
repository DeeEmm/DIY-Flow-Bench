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
 * DECLARE GLOBALS
 ***/



String localIpAddress;
bool apMode = false;
#define FILESYSTEM SPIFFS
bool ledState = 0;



/****************************************
 * FORMAT BYTES
 ***/
String formatBytes(size_t bytes) {
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
    unsigned int totalBytes = SPIFFS.totalBytes();
    unsigned int usedBytes = SPIFFS.usedBytes();
 
    Serial.println("===== File system info =====");
 
    Serial.print("Total space:      ");
    Serial.print(totalBytes);
    Serial.println("byte");
 
    Serial.print("Total space used: ");
    Serial.print(usedBytes);
    Serial.println("byte");
 
    Serial.println();
    // Serial.println("Filesystem contents:");
    // FILESYSTEM.begin();
    // File root = FILESYSTEM.open("/");
    // File file = root.openNextFile();
    // while(file){
    //     String fileName = file.name();
    //     size_t fileSize = file.size();
    //     Serial.print( fileName.c_str());
    //     Serial.print(" : ");
    //     Serial.println( formatBytes(fileSize).c_str());
    //     file = root.openNextFile();
    // }


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
      localIpAddress = WiFi.localIP().toString();
      Serial.print("IP address: ");
      Serial.println(localIpAddress);
    } else {
      // Did not connect - Go into AP Mode
      apMode = true;
      Serial.println("failed!");
      Serial.print("Creating WiFi AP (");
      Serial.print(config.wifi_ap_ssid);
      Serial.println(")");
      WiFi.softAP(config.wifi_ap_ssid.c_str(), config.wifi_ap_pswd.c_str());
      localIpAddress = WiFi.softAPIP().toString();
      Serial.print("AP IP address: ");
      Serial.println(localIpAddress);     
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
 * Send Serial Message
 ***/
void sendSerial(String message)
{   

    Serial.print(message);

    #if defined SERIAL1_ENABLED
        Serial1.print(message);
    #endif

}




/****************************************
 * Push Server Data 
 ***/
void notifyClients(String jsonValues) {
  ws.textAll(String(jsonValues));
}



/****************************************
 * Process Websocket Message
 ***/
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    
    if (strcmp((char*)data, "refresh") == 0) {
      // This is where we assign data
      ledState = !ledState;
      notifyClients(getDataJson());

    } else if (strcmp((char*)data, "loadConfig") == 0) {
      Serial.println("Load Config");
      notifyClients(loadConfig());

    } else {
      Serial.println("Save Config");
      saveConfig(data, len);
      notifyClients(loadConfig());
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





