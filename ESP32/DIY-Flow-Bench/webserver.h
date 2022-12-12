/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file webserver.h
 * 
 * @brief Webserver class header file
 * 
 * @remarks For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * @license This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 ***/
#pragma once


#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebserver.h>
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
		StaticJsonDocument<1024> dataJson; 

		String getFileListJSON ();
		String getSystemStatusJSON();		
		String byteDecode(size_t bytes);
		void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
		static void processUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
		static void saveConfig(AsyncWebServerRequest *request);
		int decodeMessageHeader (char *data);
		static String processTemplate(const String& var);
		
	
	private:
		
		void sendIndexPage();
		void uploadFile();
		String index_html;
		void resetWifi ( void );
		bool getWifiConnection();
		
	public:
	
		Webserver() {
			server = NULL;
			events = NULL;
		}
		
		AsyncEventSource *events;
		
		void begin();
		void writeJSONFile(String data, String filename);
		String getDataJSON();
		StaticJsonDocument<1024> loadJSONFile(String filename);
		void sendWebSocketMessage(String jsonValues);
		void parseConfigData(StaticJsonDocument<1024> configData);
		StaticJsonDocument<1024> loadConfig ();
		void createConfigFile ();

		
};
