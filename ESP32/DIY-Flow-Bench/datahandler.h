/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file datahandler.h
 * 
 * @brief Data Handler class header file
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
#include "system.h"
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "constants.h"

class DataHandler {

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

		String byteDecode(size_t bytes);
    	AsyncWebServer *tempServer;

    private:

		String getFileListJSON ();


    public:
		
        DataHandler() {
			tempServer = NULL;
			tempServerEvents = NULL;
		}

		AsyncEventSource *tempServerEvents;

		void begin();
		void createConfigFile ();
		void writeJSONFile(String data, String filename, int dataSize);
		void createLiftDataFile();
		void createCalibrationFile ();
		StaticJsonDocument<CONFIG_JSON_SIZE> loadJSONFile(String filename);
		void beginSerial(void);
		StaticJsonDocument<CONFIG_JSON_SIZE> loadConfig ();
		StaticJsonDocument<LIFT_DATA_JSON_SIZE> loadLiftData ();
		static void clearLiftDataFile(AsyncWebServerRequest *request);
		StaticJsonDocument<1024> loadCalibrationData ();
		void parseCalibrationData(StaticJsonDocument<1024> calibrationData);
		String getDataJSON();
		static void fileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
		void bootLoop();
		String getRemote(const char* serverName);
		bool checkUserFile(int filetype = PINSFILE); 
		bool checkSubstring(std::string firstString, std::string secondString);
		void loadPinsData ();
		void parsePinsData(StaticJsonDocument<1024> pinData);

};