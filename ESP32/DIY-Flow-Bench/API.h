/****************************************
* The DIY Flow Bench project
* https://diyflowbench.com
* 
* API.h - API header file
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


class API {

	friend class Hardware;
	friend class Maths;
	friend class Messages;
	friend class Webserver;
	
	public:
		API();
		void ParseMessage(char apiMessage);
//		void Handler(String languageString);
//		void DebugPrint(String message);

	
	private:
		uint16_t calcCRC (char* str);
		bool streamMafData;
			
};