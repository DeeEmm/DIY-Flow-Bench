/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file api.h
 * 
 * @brief API class header file
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


class API {

	friend class Hardware;
	friend class Sensors;
	friend class Maths;
	friend class Messages;
	friend class Webserver;

	private:
		uint32_t calcCRC (const char* str);
		String getConfigJSON();

		bool streamMafData;
	
	public:
		API();
		void ParseMessage(char apiMessage);
		uint32_t CRC;	
	
};
