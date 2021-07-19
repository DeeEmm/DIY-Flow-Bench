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


String hostname = HOSTNAME; 
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

    // Serial
    Serial.begin(SERIAL_BAUD_RATE);      
    Serial.println("\r\n");
    Serial.println("DIY Flow Bench ");
    Serial.println("Version: " RELEASE);
    Serial.println("Build: " BUILD_NUMBER);

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
    Serial.println("Filesystem contents:");
    FILESYSTEM.begin();
    File root = FILESYSTEM.open("/");
    File file = root.openNextFile();
    while(file){
        String fileName = file.name();
        size_t fileSize = file.size();
        Serial.print( fileName.c_str());
        Serial.print(" : ");
        Serial.println( formatBytes(fileSize).c_str());
        file = root.openNextFile();
    }


    // API
    #if defined API_ENABLED
      Serial.println("Serial API Enabled");
    #endif

    // WiFi    
    Serial.println("Connecting to WiFi");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PSWD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.print("Connected to ");
    Serial.println(WIFI_SSID);
    Serial.print("IP address: ");
    localIpAddress = WiFi.localIP().toString();
    Serial.println(WiFi.localIP());

    
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
    
    if (strcmp((char*)data, "toggle") == 0) {
      // This is where we assign data
      ledState = !ledState;
      notifyClients(getJsonData());
    }

    if (strcmp((char*)data, "getValues") == 0) {
      notifyClients(getJsonData());
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





