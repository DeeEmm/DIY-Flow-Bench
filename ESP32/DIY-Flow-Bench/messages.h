/***********************************************************
* The DIY Flow Bench project
* https://diyflowbench.com
* 
* messages.h - messages header file
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
// #include "configuration.h"

class Messages {

	private:
		
		char printBuffer[PRINT_BUFFER_LENGTH];
		
		
	public:
		Messages();
		
		void Handler(const String langPhrase);
		void begin(void);
		void beginSerial(void);

		size_t serialPrintTestf(char *buf, char *format, ...);
		size_t serialPrintf(const std::string format, ...);
		size_t blobPrintf(std::string format, ...);
		size_t debugPrintf(const std::string format, ...);
		size_t statusPrintf(const std::string format, ...);
		
		// void * ICACHE_FLASH_ATTR Messages::serialPrintf(const char *s, ...);
		
		char floatBuffer[16];

};