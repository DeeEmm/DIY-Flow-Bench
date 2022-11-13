/***********************************************************
* The DIY Flow Bench project
* https://diyflowbench.com
* 
* server.h - server header file
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
#pragma once


#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

class Webserver {

	friend class Maths;
	friend class Messages;
	friend class Hardware;
	friend class Settings;
	friend class Sensors;
	friend class AsyncWebServer;
	friend class AsyncWebSocket;
	friend class AsyncEventSource;
	friend class ArduinoJSON;
	
	
	protected:

		void loop();

    	AsyncWebServer *server;

		String getFileListJSON ();
		String getSystemStatusJSON();
		
		StaticJsonDocument<1024> dataJson; 
		
		String byteDecode(size_t bytes);
		void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
		static void processUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
		void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
		int decodeMessageHeader (char *data);
		String processTemplate(const String& var);
		
		// void Handler(String languageString);

	
	private:
		
		#define FILESYSTEM SPIFFS
		
		void sendIndexPage();
		void uploadFile();
		// void onBody();
		String index_html;
		
		
	public:
	
		Webserver() {
			server = NULL;
			webskt = NULL;
		}
		
		AsyncWebSocket *webskt;
		
		void begin();
		void writeJSONFile(String data, String filename);
		String getDataJSON();
		StaticJsonDocument<1024> loadJSONFile(String filename);
		void sendWebSocketMessage(String jsonValues);

};
