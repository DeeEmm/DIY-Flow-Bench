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

	
	friend class Messages;
	friend class Hardware;
	friend class Sensors;
	friend class AsyncWebServer;
	friend class AsyncWebSocket;
	friend class AsyncEventSource;
	friend class ArduinoJSON;
	
	
	protected:

		void loop();

    	AsyncWebServer *server;
		JsonDocument dataJson; 
		JsonDocument liftDataJson; 

		String getSystemStatusJSON();		
		void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
		static void processUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);
		// static void parseConfigurationForm(AsyncWebServerRequest *request);
		static void checkUpdate(AsyncWebServerRequest *request);
		static void saveCalibrationForm(AsyncWebServerRequest *request);
		static void parseOrificeForm(AsyncWebServerRequest *request);

		static void saveConfigurationForm(AsyncWebServerRequest *request);
		static void saveSettingsForm(AsyncWebServerRequest *request);
		static void savePinsForm(AsyncWebServerRequest *request);

		int decodeMessageHeader (char *data);



	private:
		
		void sendIndexPage();
		// void uploadFile();
		String index_html;
		void parseLiftData(JsonDocument liftData);
		
	public:
	
		Webserver() {
			server = NULL;
			events = NULL;
		}
		
		AsyncEventSource *events;
		
		void begin();
		void sendWebSocketMessage(String jsonValues);
		void createSettingsFile ();
		String getLiftDataJSON();
		
		JsonDocument getSDFile(String filename);
		JsonDocument getSDFileList(String filename);
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
		

		static void saveLiftDataForm(AsyncWebServerRequest *request);
		static void parseUserFlowTargetForm(AsyncWebServerRequest *request);

		static void clearLiftData(AsyncWebServerRequest *request);

		static void toggleFlowDiffTile (); 
		static void fileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

		static String processIndexPageTemplate(const String& var);
		static String processLandingPageTemplate(const String& var);
		static String processindexJavascriptTemplate(const String& var);
		static String processSettingsPageTemplate(const String& var);
		static String processDataPageTemplate(const String& var);
		static String processPinsPageTemplate(const String& var);
		static String processConfigPageTemplate(const String& var);
		static String processMimicPageTemplate(const String& var);
		static String processCalibrationPageTemplate(const String& var);
		static String processOrificePageTemplate(const String& var);
		static String processDatagraphPageTemplate(const String& var);
		

		static String processLanguageTemplateVars(const String& var);

		
};
