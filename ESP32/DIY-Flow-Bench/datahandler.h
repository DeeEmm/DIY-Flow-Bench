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
// #include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

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



    private:




    public:
		
        DataHandler() {

		}

		void begin();
		void createConfigFile ();
		void writeJSONFile(String data, String filename, int dataSize);
		void createLiftDataFile();
		void createCalibrationFile ();
		StaticJsonDocument<CONFIG_JSON_SIZE> loadJSONFile(String filename);


};