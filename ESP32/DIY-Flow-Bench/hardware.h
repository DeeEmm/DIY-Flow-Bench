/***********************************************************
* The DIY Flow Bench project
* https://diyflowbench.com
* 
* Hardware.h - Hardware header file
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

class Hardware {
	
	friend class Messages;
	friend class Maths;


	public:
		Hardware();
		void begin ();
		//int * getI2CList();
		void getI2CList();
		int getADCRawData(int channel);
		int getSupplyMillivolts();
		bool benchIsRunning();	
		void checkRefPressure();
		
	private:
		void configurePins ();
		uint8_t buffer[3];
		uint8_t m_bitShift; 
};