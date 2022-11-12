/***********************************************************
* The DIY Flow Bench project
* https://diyflowbench.com
* 
* calculations.h - Maths header file
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


class Calculations {

	friend class Sensors;
	friend class Hardware;
	friend class mafData;
	
	private:
		float MOLECULAR_WEIGHT_DRY_AIR;
		bool streamMafData = false;
		
		// int _mafDataUnit;

	public:
		Calculations();
		float convertFlowDepression(float oldPressure, float newPressure, float inputFlow);
		float convertPressure(float baroPressureKpa, int units);
		float convertTemperature(float refTempDegC, int units);
		float convertRelativeHumidity(double relativeHumidity, int units);

		float calculateVaporPressure(int units);
		float calculateSpecificGravity();
		float convertMassFlowToVolumetric(float massFlowKgh);
		float calculateFlowCFM();

		float startupBaroPressure;

		// long _mafLookupTable[][2];

};
