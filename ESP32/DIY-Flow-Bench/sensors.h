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

	friend class mafData;

	private:
		int _unit;
		char* _mafSensorType;
		int _mafOutputType;
		int _mafDataUnit;
		float refPressMillivolts;
		float pDiffMillivolts;
		float pitotMillivolts;
		float mafMillivolts;

		
		char* _tempSensorType;
		char* _baroSensorType;
		char* _relhSensorType;
		char* _prefSensorType;
		char* _pdiffSensorType;
		char* _pitotSensorType;    
		
		// Temperature coefficients
		uint16_t dig_T1;
		int16_t dig_T2;
		int16_t dig_T3;
		int32_t t_fine;
		int32_t t_var1;
		int32_t t_var2;

		//long adc_t;
		
		
		// Pressure coefficients
		uint16_t dig_P1;
		int16_t dig_P2;
		int16_t dig_P3;
		int16_t dig_P4;
		int16_t dig_P5;
		int16_t dig_P6;
		int16_t dig_P7;
		int16_t dig_P8;
		int16_t dig_P9;
		long adc_p;
		
		// Humidity
		uint8_t dig_H1;
		int16_t dig_H2;
		uint8_t dig_H3;
		int16_t dig_H4;
		int16_t dig_H5;
		int8_t  dig_H6;
		long adc_h;
		double var_H;

		// ADC
		float adc_min;
		float adc_fsd;

		// BME280 
		
		// private functions
		bool BME280init(void);
		uint8_t BME280Read8(uint8_t reg);
		uint16_t BME280Read16(uint8_t reg);
		uint16_t BME280Read16LE(uint8_t reg);
		int16_t BME280ReadS16(uint8_t reg);
		int16_t BME280ReadS16LE(uint8_t reg);
		uint32_t BME280Read24(uint8_t reg);
		void writeRegister(uint8_t reg, uint8_t val);
		float BME280GetTemperature(void);
		// float BME280GetPressure(void);
		float BME280GetHumidity(void);
		void BME280WriteRegister(uint8_t reg, uint8_t val);
		
		//unsigned int data[8];
		//unsigned int buffer[25];


	public:
		Sensors();
		void begin();
		void initialise();
		void getBME280RawData();
		int convertADCtoMillivolts(int rawVal);
		float getMafValue();
		float getMafMillivolts();
		float getTempValue();
		float getBaroValue();
		double getRelHValue();
		float getAltitude();
		float getPRefMillivolts();
		float getPRefValue();
		float getPDiffMillivolts();
		float getPDiffValue();
		float getPitotMillivolts();
		float getPitotValue();
		
		void mafFreqCountISR();
		void mafSetupISR(uint8_t irq_pin, void (*ISR_callback)(void), int value);
		
		float startupBaroPressure;
		volatile uint64_t StartValue;                 
		volatile uint64_t PeriodCount; 
		hw_timer_t * timer;         
		bool isTransport_OK;          
		// float refTempDegC;
		float baroPressureKpa;
		uint32_t baroPressureHpa;
		float baroPressurePa;
		float relativeHumidity;
  
};
