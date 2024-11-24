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
#include <ESPAsyncWebServer.h>
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
		StaticJsonDocument<LIFT_DATA_JSON_SIZE> liftDataJson; 

		String getSystemStatusJSON();		
		void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
		static void processUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);
		static void parseConfigurationForm(AsyncWebServerRequest *request);
		static void parseCalibrationForm(AsyncWebServerRequest *request);
		static void parseOrificeForm(AsyncWebServerRequest *request);

		int decodeMessageHeader (char *data);
		static String processTemplate(const String& var);
	
	private:
		
		void sendIndexPage();
		// void uploadFile();
		String index_html;
		void parseLiftData(StaticJsonDocument<LIFT_DATA_JSON_SIZE> liftData);
		
	public:
	
		Webserver() {
			server = NULL;
			events = NULL;
		}
		
		AsyncEventSource *events;
		
		void begin();
		void sendWebSocketMessage(String jsonValues);
		void parseConfigSettings(StaticJsonDocument<CONFIG_JSON_SIZE> configData);
		StaticJsonDocument<CONFIG_JSON_SIZE> loadConfig ();
		void createConfigFile ();
		String getValveDataJSON();
		
		StaticJsonDocument<1024> getSDFile(String filename);
		StaticJsonDocument<1024> getSDFileList(String filename);
		bool writeToSDFile(const char* filePath, const char* data);
		bool appendToSDFile(const char* filePath, const char* data);
		const char* readSDFile(const char* filePath);

		void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
		void createDir(fs::FS &fs, const char * path);
		void removeDir(fs::FS &fs, const char * path);
		void readFile(fs::FS &fs, const char * path);
		void writeFile(fs::FS &fs, const char * path, const char * message);
		void appendFile(fs::FS &fs, const char * path, const char * message);
		void renameFile(fs::FS &fs, const char * path1, const char * path2);
		void deleteFile(fs::FS &fs, const char *path);
		void testFileIO(fs::FS &fs, const char * path);

		static void parseLiftDataForm(AsyncWebServerRequest *request);
		static void parseUserFlowTargetForm(AsyncWebServerRequest *request);
		static String processLandingPageTemplate(const String& var);

		static void toggleFlowDiffTile (); 
		static void processUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);



		
};
