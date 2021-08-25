/***********************************************************
* The DIY Flow Bench project
* https://diyflowbench.com
* 
* maths.h - Maths header file
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


class Maths {

	friend class Sensors;
	friend class Hardware;

	public:
		Maths();
		double convertFlowDepression(float oldPressure, int newPressure, float inputFlow);
		float calculateBaroPressure(int units);
		float calculateRefPressure(int units);
		float calculateTemperature(int units);
		float calculateRelativeHumidity(int units);
		float calculateVaporPressure(int units);
		float calculateSpecificGravity();
		float convertMassFlowToVolumetric(float massFlowKgh);
		float calculateFlowCFM();
		float calculatePitotPressure(int units);
		float calculateDiffPressure(int units);
		
		float startupBaroPressure;
	
	private:
		float MOLECULAR_WEIGHT_DRY_AIR;
		bool streamMafData = false;
		
		int _mafDataUnit;
		long _mafLookupTable[][2];

};
