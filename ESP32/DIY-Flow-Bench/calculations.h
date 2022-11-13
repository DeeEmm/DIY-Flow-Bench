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
		double MOLECULAR_WEIGHT_DRY_AIR;
		bool streamMafData = false;
		
		// int _mafDataUnit;

	public:
		Calculations();
		double convertFlowDepression(double oldPressure, double newPressure, double inputFlow);
		double convertPressure(double baroPressureKpa, int units);
		double convertTemperature(double refTempDegC, int units);
		double convertRelativeHumidity(double relativeHumidity, int units);

		double calculateVaporPressure(int units);
		double calculateSpecificGravity();
		double convertMassFlowToVolumetric(double massFlowKgh);
		double calculateFlowCFM();

		double startupBaroPressure;

		// long _mafLookupTable[][2];

};
