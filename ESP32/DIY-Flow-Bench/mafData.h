/***********************************************************
* The DIY Flow Bench project
* https://diyflowbench.com
* 
* mafData.h - MafData header file
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


class MafData {

	public:
		MafData();
		String mafSensorType ();
		int MAFdataFormat ();
		int MAFdataUnit ();
		int MAFoutputType();
		long getMafData();
	
		long mafLookupTable[][2];
	
	private:

};