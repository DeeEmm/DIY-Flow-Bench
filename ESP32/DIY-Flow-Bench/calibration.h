/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file calibration.h
 * 
 * @brief Calibration class header file
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

#include <ArduinoJson.h>


class Calibration {

	friend class Sensors;
	friend class Maths;

	public:
		Calibration();
		bool setFlowOffset();
		double getFlowOffset();
		bool setLeakOffset();
		double getLeakOffset();
		double getLeakOffsetReverse();
		void writeCalibrationFile(String data, String filename);
		void saveCalibrationData();
		void createCalibrationFile ();
		bool setPdiffCalOffset();
		double getPdiffCalOffset();
		bool setPitotCalOffset();
		double getPitotCalOffset();
		StaticJsonDocument<1024> loadCalibrationFile();		


	
	private:

};
