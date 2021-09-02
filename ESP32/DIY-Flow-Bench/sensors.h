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
    void begin();
	void initialise();
    float getMafValue();
	float getTempValue();
	float getBaroValue();
	float getRelHValue();
	float getPRefValue();
	float getPDiffValue();
	float getPitotValue();
	
	void mafFreqCountISR();
	void mafSetupISR(uint8_t irq_pin, void (*ISR_callback)(void), int value);
	
	float startupBaroPressure;
	volatile uint64_t StartValue;                 
	volatile uint64_t PeriodCount; 
	hw_timer_t * timer;                   
  
private:
	int _unit;
	String _mafSensorType;
	int _mafOutputType;
	int _mafDataUnit;
	
	
	String _tempSensorType;
	String _baroSensorType;
	String _relhSensorType;
	String _prefSensorType;
	String _pdiffSensorType;
	String _pitotSensorType;    
	
	

  
};

