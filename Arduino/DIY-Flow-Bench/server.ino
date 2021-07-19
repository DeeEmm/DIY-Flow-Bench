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

//const char* LANGUAGE_FILE = "EN_Language.h";

String hostname = HOSTNAME; 
bool apMode = false;



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
 * 404 Error
 * 
 ***/
void error404() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
 
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
 
  server.send(404, "text/plain", message);
}




/****************************************
 * Load From SPIFFS
 ***/
bool loadFromSPIFFS(String path) {
  String dataType = "text/html";
 
  Serial.print("Requested page -> ");
  Serial.println(path);
  if (SPIFFS.exists(path)){
      File dataFile = SPIFFS.open(path, "r");
      if (!dataFile) {
          error404();
          return false;
      }
 
      if (server.streamFile(dataFile, dataType) != dataFile.size()) {
        Serial.println("Sent less data than expected!");
      }else{
          Serial.println("Page served!");
      }
 
      dataFile.close();
  }else{
      error404();
      return false;
  }
  return true;
}



/****************************************
 * Load From Server Root
 ***/
void handleRoot() {
    loadFromSPIFFS("/index.html");  
}


// void sendData() {
//   long randNumber;
//   randNumber = random(300);
//   server.send(200, "text/event-stream", String(randNumber));
// }

