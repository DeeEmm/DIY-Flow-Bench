/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file hardware.h
 * 
 * @brief Hardware class header file
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

class Hardware {

	
	friend class Messages;
	friend class Maths;

	protected:
		uint8_t		_i2cAddress;
		uint32_t	_gain;
		uint32_t	_mux;
		uint8_t		_rate;			  
        uint32_t 	_config;

	public:
		Hardware();
		void begin ();
		void getI2CList();
		void getI2CDeviceList();
		void initaliseIO ();
		
		int32_t getADCRawData(int channel);
		double get3v3SupplyVolts();
		double get5vSupplyVolts();
		double getADCVolts(int channel);
		bool benchIsRunning();	
		void checkRefPressure();
		void setVFDRef();
		void setBleedValveRef();
		
		void benchOn();
		void benchOff();
		float uptime();

		void stepperTest();


	private:
		// void configurePins ();


};
