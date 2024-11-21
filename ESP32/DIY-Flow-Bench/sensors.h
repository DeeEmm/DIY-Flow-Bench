/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file sensors.h
 * 
 * @brief Sensors class header file
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

#include "constants.h"

class Sensors {

	private:
		int _unit;
		int _mafOutputType;
		int _mafDataUnit;
		int mafFlowRaw;

		String _mafSensorType;
		String _tempSensorType;
		String _baroSensorType;
		String _relhSensorType;
		String _prefSensorType;
		String _pdiffSensorType;
		String _pitotSensorType;    

	public:
		Sensors();
		void begin();
		void initialise();
		void getBME280RawData();
		double BME280GetTemperature(void);
		double BME280GetHumidity(void);
		long getMafRaw();
		double getMafFlow(int units = KG_H);
		double getDifferentialFlow();
		double getMafVolts();
		double getTempValue();
		double getBaroValue();
		double getRelHValue();
		double getPRefVolts();
		double getPRefValue();
		double getPDiffVolts();
		double getPDiffValue();
		double getPitotVolts();
		double getPitotValue();
		double getPitotVelocity();
		void mafFreqCountISR();
		void mafSetupISR(uint8_t irq_pin, void (*ISR_callback)(void), int value);

	
		double startupBaroPressure;
		volatile uint64_t StartValue;                 
		volatile uint64_t PeriodCount; 
		hw_timer_t * timer;         
       
		double baroPressureKpa;
		double baroPressureHpa;
		double baroPressurePa;
		double relativeHumidity;
  
};
