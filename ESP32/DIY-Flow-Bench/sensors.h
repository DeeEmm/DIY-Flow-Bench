/***********************************************************
* The DIY Flow Bench project
* https://diyflowbench.com
* 
* sensors.h - Sensors class header file
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

#include "constants.h"


class Sensors {

public:
	Sensors();
    void Begin();
	void Initialise();
    float getMAF();
	float getTemp();
	float getBaro();
	float getRelH();
	float getPRef();
	float getPDiff();
	float getPitot();
	float startupBaroPressure;
  
private:
	int _unit;
	float _baro;
	float _maf;
	float _relh;
	float _temp;
	float _pref;
	float _pdiff;
	float _pitot;
	String _mafSensorType;
	int _mafOutputType;
	int _mafDataFormat;
	int _mafDataUnit;
	
	String _tempSensorType;
	String _baroSensorType;
	String _relhSensorType;
	String _prefSensorType;
	String _pdiffSensorType;
	String _pitotSensorType;    
  
};

