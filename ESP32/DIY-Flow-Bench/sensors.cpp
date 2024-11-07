/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file sensors.cpp
 * 
 * @brief Sensors class
 * 
 * @remarks For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * @license This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 ***/

#include "Arduino.h"


#include "constants.h"
#include "structs.h"

#include <Wire.h>
#include "hardware.h"
#include "sensors.h"
#include "messages.h"
#include "driver/pcnt.h"


// HACK THIS IS JUST A TEMP MEASURE AS configuration.h file now being moved into cinfiguration.json
// need to add conditional load - only load for MAF style benches
#define MAF_DATA_FILE "mafData/BOSCH_0280218067.cpp" 
#include MAF_DATA_FILE



// We are going to include the BME library, even if it is not used as there is no way to exclude it using pre-compile directives if we want to enable it at run time using json configurations
#define TINY_BME280_I2C
#include "TinyBME280.h"
tiny::BME280 _BMESensor;
TwoWire I2CBME = TwoWire(0);


/***********************************************************
 * @brief Class constructor
 ***/
Sensors::Sensors() {

}


/***********************************************************
 * @brief Sensor begin method
 ***/
void Sensors::begin () {

	Messages _message;
	_message.serialPrintf("Initialising Sensors \n");
	Sensors::initialise ();
	_message.serialPrintf("Sensors Initialised \n");
}

// REVIEW - Frequency style MAF Begin
// NOTE: Voodoo needed to get interrupt to work within class structure for frequency measurement. 
// We declare a new instance of the Sensor class so that we can use it for the MAF ISR
// https://forum.arduino.cc/t/pointer-to-member-function/365758
// Sensors __mafVoodoo;


 
/***********************************************************
 * @brief Initialise sensors 
 ***/
void Sensors::initialise () {

	Messages _message;

	extern struct DeviceStatus status;
    extern struct Language language;
	extern struct Configuration config;
	extern struct Pins pins;
	extern int mafOutputType;

	Maf _maf;

	// Initialise  MAF data
	if (config.MAF_IS_ENABLED) {
		
		// get size of the MAF datatable
  		status.mafDataTableRows = mafLookupTable.size() -1;

		// get highest MAF input value from data table
		status.mafDataValMax = mafLookupTable[status.mafDataTableRows][1];
		status.mafDataKeyMax = mafLookupTable[status.mafDataTableRows][0];
		
		// get MAF units
		status.mafUnits = _maf.mafUnits();

		// get MAF data scaling
		status.mafScaling = _maf.mafScaling();

		// get MAF diameter
		status.mafDiameter = _maf.mafDiameter();

	}


	
	//initialise BME
	if (config.BME_IS_ENABLED) {
	
		_message.serialPrintf("Initialising BME280 \n");	
	
		if (_BMESensor.beginI2C(config.BME280_I2C_ADDR) == false)
		{
			_message.serialPrintf("BME sensor did not respond. \n");
			_message.serialPrintf("Please check wiring and I2C address\n");
			_message.serialPrintf("BME I2C address %s set in configuration.h. \n", config.BME280_I2C_ADDR);
			while(1); //Freeze
		} else {
			_message.serialPrintf("BME280 Initialised\n");
		}

		_BMESensor.setTempOverSample(1);
		_BMESensor.setPressureOverSample(1);
		_BMESensor.setHumidityOverSample(1);
		_BMESensor.setStandbyTime(0);
		_BMESensor.setFilter(0);
		_BMESensor.setMode(tiny::Mode::NORMAL); // NORMAL / FORCED / SLEEP

	}




	// REVIEW FREQUENCY BASED MAF - temporarily disabled - need to fix & reenable
	// Set up the MAF ISR if required
	// Note Frequency based MAFs are required to be attached direct to MAF pin for pulse counter to work
	// This means 5v > 3.3v signal conditioning is required on MAF pin (possible simple voltage divider + filter)
	// Need to check out use of ESP PCNT function - https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/pcnt.html
	// ESP32 has max PCNT frequency of 40MHz (half clock speed - 80MHz)
	// Typical MAF is 30Hz -  150Hz
	// Example - https://github.com/espressif/esp-idf/blob/v4.4/examples/peripherals/pcnt/pulse_count_event/main/pcnt_event_example_main.c
	// also this... https://blog.adafruit.com/2018/12/27/frequency-measurement-on-an-esp32-in-micropython-esp32-micropython-python/
	// and this... https://forum.micropython.org/viewtopic.php?f=18&t=5724&p=32921
	// (which has example without interrupts)

/* temp disabled - need to reenable	
	if (_maf.outputType == FREQUENCY) {	
		__mafVoodoo.mafSetupISR(MAF_PIN, []{__mafVoodoo.mafFreqCountISR();}, FALLING);
		timer = timerBegin(0, 2, true);                                  
		timerStart(timer);	
	}
*/	


	// Sensor definitions for system status pane
	// MAF Sensor
	if (config.MAF_IS_ENABLED && config.MAF_SRC_IS_ADC && config.ADC_IS_ENABLED) {
		this->_mafSensorType = _maf.sensorType();
	} else if (config.MAF_IS_ENABLED && config.MAF_SRC_IS_PIN) {
		this->_mafSensorType = _maf.sensorType() + " on GPIO:" + pins.MAF_PIN;
	} else {
		this->_mafSensorType = language.LANG_NOT_ENABLED;
	}

	// Baro Sensor
	#if defined BARO_SENSOR_TYPE_REF_PRESS_AS_BARO
		this->startupBaroPressure = this->getPRefValue();
		this->_baroSensorType = language.LANG_START_REF_PRESSURE;
	#elif defined BARO_SENSOR_TYPE_FIXED_VALUE
		this->startupBaroPressure = FIXED_BARO_VALUE;
		this->_baroSensorType = language.LANG_FIXED_VALUE;
		this->_baroSensorType += FIXED_BARO_VALUE;
	#elif defined BARO_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED
		this->_baroSensorType = "BME280";
	#elif defined BARO_SENSOR_TYPE_MPX4115
		this->_baroSensorType = "MPX4115";
	#elif defined BARO_SENSOR_TYPE_LINEAR_ANALOG
		this->_baroSensorType = "ANALOG PIN: " + REF_BARO_PIN;
	#else 
		this->_baroSensorType = String(language.LANG_FIXED_VALUE) + String(config.FIXED_BARO_VALUE);
	#endif
	
	//Temp Sensor
	#ifdef TEMP_SENSOR_NOT_USED
		this->startupBaroPressure = this->getPRefValue();
		this->_tempSensorType = language.LANG_NOT_ENABLED;
	#elif defined TEMP_SENSOR_TYPE_FIXED_VALUE
		this->_tempSensorType = language.LANG_FIXED_VALUE;
		this->_tempSensorType += FIXED_TEMP_VALUE;
	#elif defined TEMP_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED
		this->_tempSensorType = "BME280";
	#elif defined TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11
		this->_tempSensorType = "Simple DHT11";
	#elif defined TEMP_SENSOR_TYPE_LINEAR_ANALOG
		this->_tempSensorType = "ANALOG PIN: " + TEMPERATURE_PIN;
	#else 
		this->_tempSensorType = String(language.LANG_FIXED_VALUE) + String(config.FIXED_TEMP_VALUE);
	#endif
	
	// Rel Humidity Sensor
	#ifndef RELH_IS_ENABLED
		this->startupBaroPressure = this->getPRefValue();
		this->_relhSensorType = language.LANG_NOT_ENABLED;
	#elif defined RELH_SENSOR_TYPE_FIXED_VALUE
		this->_relhSensorType = language.LANG_FIXED_VALUE;
		this->_relhSensorType += FIXED_RELH_VALUE;
	#elif defined RELH_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED
		this->_relhSensorType = "BME280";
	#elif defined RELH_SENSOR_TYPE_SIMPLE_TEMP_DHT11
		this->_relhSensorType = "Simple DHT11";
	#elif defined RELH_SENSOR_TYPE_LINEAR_ANALOG
		this->_relhSensorType = "ANALOG PIN: " + HUMIDITY_PIN;
	#else 
		this->_relhSensorType = language.LANG_FIXED_VALUE + FIXED_RELH_VALUE;
	#endif

	// reference pressure
	#ifndef PREF_IS_ENABLED
		this->_prefSensorType = language.LANG_NOT_ENABLED;
	#elif defined PREF_SENSOR_TYPE_MPXV7007 && defined ADC_IS_ENABLED
		this->_prefSensorType = "SMPXV7007";
	#elif defined PREF_SENSOR_TYPE_MPX4250 && defined ADC_IS_ENABLED
		this->_prefSensorType = "MPX4250";
	#elif defined PREF_SENSOR_TYPE_MPXV7025 && defined ADC_IS_ENABLED
		this->_prefSensorType = "MPXV7025";
	#elif defined PREF_SENSOR_TYPE_LINEAR_ANALOG
		this->_prefSensorType = "ANALOG PIN: " + REF_PRESSURE_PIN;
	#elif defined PREF_SENSOR_TYPE_M5STACK_TubePressure && defined ADC_IS_ENABLED
		this->_pitotSensorType = "TubePressure";
	#else 
		this->_prefSensorType = language.LANG_NOT_ENABLED;
	#endif
	
	// differential pressure
	#ifndef PDIFF_IS_ENABLED
		this->_pdiffSensorType = language.LANG_NOT_ENABLED;
	#elif defined PDIFF_SENSOR_TYPE_MPXV7007 && defined ADC_IS_ENABLED
		this->_pdiffSensorType = "SMPXV7007";
	#elif defined PDIFF_SENSOR_TYPE_LINEAR_ANALOG
		this->_pdiffSensorType = "ANALOG PIN: " + DIFF_PRESSURE_PIN;
	#elif defined PDIFF_SENSOR_TYPE_MPXV7025 && defined ADC_IS_ENABLED
		this->_prefSensorType = "MPXV7025";
	#elif defined PITOT_SENSOR_TYPE_M5STACK_TubePressure && defined ADC_IS_ENABLED
		this->_pitotSensorType = "TubePressure";
	#else 
		this->_pdiffSensorType = language.LANG_NOT_ENABLED;
	#endif
	
	// pitot pressure differential
    #ifndef PITOT_IS_ENABLED
		this->_pitotSensorType = language.LANG_NOT_ENABLED;
	#elif defined PITOT_SENSOR_TYPE_MPXV7007 && defined ADC_IS_ENABLED
		this->_pitotSensorType = "SMPXV7007";
	#elif defined PITOT_SENSOR_TYPE_MPXV7025 && defined ADC_IS_ENABLED
		this->_prefSensorType = "MPXV7025";
	#elif defined PITOT_SENSOR_TYPE_LINEAR_ANALOG
		this->_pitotSensorType = "ANALOG PIN: " + PITOT_PIN;
	#elif defined PITOT_SENSOR_TYPE_M5STACK_TubePressure && defined ADC_IS_ENABLED
		this->_pitotSensorType = "TubePressure";
	#else 
		this->_pitotSensorType = language.LANG_NOT_ENABLED;
	#endif
	
	// Set status values for GUI
	status.mafSensor = this->_mafSensorType;
	status.baroSensor = this->_baroSensorType;
	status.tempSensor = this->_tempSensorType;
	status.relhSensor = this->_relhSensorType;
	status.prefSensor = this->_prefSensorType;
	status.pdiffSensor = this->_pdiffSensorType;
	status.pitotSensor = this->_pitotSensorType;

	// END System status definitions

}

// REVIEW - MAF Interrupt service routine setup
/***********************************************************
  * @brief Set up MAF ISR
  *
  * We cannot call a non-static member function directly so we need to encapsulate it
  * This is part of the Voodoo
  ***/
/* TODO: temp disabled - need to reenable	
void Sensors::mafSetupISR(uint8_t irq_pin, void (*ISR_callback)(void), int value) {
  attachInterrupt(digitalPinToInterrupt(irq_pin), ISR_callback, value);
}
*/


// REVIEW - MAF Interrupt service routine
/***********************************************************
 * @brief Interrupt Service Routine for MAF Frequency measurement
 *
 * Determine how long since last triggered (Resides in RAM memory as it is faster)
 ***/
// TODO: Add IRAM_ATTR to Nova-Arduino 
/* TODO: temp disabled - need to reenable	
void IRAM_ATTR Sensors::mafFreqCountISR() {
    uint64_t TempVal = timerRead(timer);            
    PeriodCount = TempVal - StartValue;             
    StartValue = TempVal;                           
}
*/




/***********************************************************
 * @brief Returns RAW MAF Sensor value (ADC Value)
 *
 ***/
long Sensors::getMafRaw() {
	
	Hardware _hardware;
	extern struct SensorData sensorVal;

	#ifdef MAF_IS_ENABLED
	Maf _maf;

	#if defined MAF_SRC_IS_ADC && defined ADC_IS_ENABLED
		sensorVal.MafRAW = _hardware.getADCRawData(MAF_ADC_CHANNEL);
		
	#elif defined MAF_SRC_IS_PIN	
		long mafFlowRaw = analogRead(MAF_PIN);
	#else
		mafFlowRaw = 1;
	#endif

	return sensorVal.MafRAW;

	#else

	return 0; // MAF is disabled so lets return 1

	#endif
}




/***********************************************************
 * @brief getMafVolts: Returns MAF signal in Volts
 ***/
double Sensors::getMafVolts() {
	
	Hardware _hardware;

	extern struct Configuration config;

	double sensorVolts = 0.00F;
	
	#if defined MAF_SRC_IS_ADC && defined MAF_IS_ENABLED && defined ADC_IS_ENABLED
		sensorVolts = _hardware.getADCVolts(MAF_ADC_CHANNEL);
				
	#elif defined MAF_SRC_IS_PIN	
		long mafRaw = analogRead(MAF_PIN);
		sensorVolts = mafRaw * (_hardware.get3v3SupplyVolts() / 4095.0);
	#else
		sensorVolts = 1.0;
		return sensorVolts;
	#endif
	
	// Lets make sure we have a valid value to return
	if (sensorVolts > 0) {
		sensorVolts += config.MAF_MV_TRIMPOT;
		return sensorVolts;
	} else {
		return 0.0;
	}	
	
}





/***********************************************************
 * @brief Returns MAF mass flow value in KG/H 
 * @note Interpolates lookupvalue from datatable key>value pairs
 *
 ***/
double Sensors::getMafFlow(int units) {

	extern struct DeviceStatus status;
	extern struct SensorData sensorVal;
	extern struct BenchSettings settings;
	extern struct Configuration config;

	Hardware _hardware;

	double flowRateCFM = 0.0;
	double flowRateMGS = 0.0;
	double flowRateKGH = 0.0;
	double lookupValue = 0.0;
	double transposedflowRateKGH = 0.0;
	double mafVelocity = 0.0;

	// #include config.MAF_DATA_FILE.str.c_str();
	// #include (String(config.MAF_DATA_FILE).c_str());


//https://stackoverflow.com/questions/76273155/variable-in-include-file-c
// #define FILENAME config.MAF_DATA_FILE

// #define QUOTE(x) _QUOTE(x)
// #define _QUOTE(x) #x

// #define _CONCAT(x, y) x##y.h
// #define CONCAT(x, y) _CONCAT(x, y)

// #define _INCLUDE_FILE(x, y) QUOTE(CONCAT(x, y))
// #define INCLUDE_FILE(x, y) _INCLUDE_FILE(x, y)

// #define MAF_FILE config.MAF_DATA_FILE
// #define STRINGIFY(MAF_FILE)

// read maf.json




	// scale sensor reading to data table size using map function (0-5v : 0-keymax)
	// NOTE Discrepency with MAP calculated value
	// long refValue =  map(this->getMafVolts(), 0, _hardware.get5vSupplyVolts(), 0, status.mafDataKeyMax ); 
	// long refValue =  map(this->getMafVolts(), 0, 5, 0, status.mafDataKeyMax); 
	long refValue = (status.mafDataKeyMax / 5) * this->getMafVolts();

	for (int rowNum = 0; rowNum < status.mafDataTableRows; rowNum++) { // iterate the data table comparing the Lookup Value to the refValue for each row

		long Key = mafLookupTable[rowNum][0]; // Key Value for this row (x2)
		long Val = mafLookupTable[rowNum][1]; // Flow Value for this row (y2)

		// Did we get a match??
		if (refValue == Key) { // Great!!! we've got the exact key value

			lookupValue = Val; // lets use the associated lookup value
			sensorVal.MafLookup = Val;
			break;
			
		// } else if ( Key > refValue && rowNum == 0) { // we were only on the first row so there is no previous value to interpolate with, so lets set the flow value to zero and consider it no flow

		// 	sensorVal.MafLookup = 0;
		// 	return 0; 
		// 	break;

		} else if (Key > refValue && rowNum > 0) { // The value is somewhere between this and the previous key value so let's use linear interpolation to calculate the actual value: 

			long KeyPrev = mafLookupTable[rowNum - 1][0]; // Key value for the previous row (x1)
			long ValPrev = mafLookupTable[rowNum - 1][1]; // Flow value for the previous row (y1)

			// Linear interpolation y = y1 + (x-x1)((y2-y1)/(x2-x1)) where x1+y1 are coord1 and x2_y2 are coord2
			lookupValue = ValPrev + (refValue - KeyPrev)*((Val-ValPrev)/(Key-KeyPrev));
			sensorVal.MafLookup = lookupValue;
			break;   

		} else if (rowNum == status.mafDataTableRows && refValue > Key) { //we're at the largest value, this must be it
			lookupValue = Val; // lets use the associated lookup value
			sensorVal.MafLookup = Val;
			// TODO status message MAX FLOW
			break;
		}

	} 

	// Scale lookup value
	lookupValue *= status.mafScaling; 

	if (status.mafUnits == MG_S) {
		flowRateKGH = lookupValue * 0.0036F;

	} else { // mafUnits is KG/H
		flowRateKGH = lookupValue;
	}

	// Now that we have a converted flow value we can translate it for different housing diameters
	if (settings.maf_housing_diameter > 0 && status.mafDiameter > 0 && settings.maf_housing_diameter != status.mafDiameter) { 
		// We are running a custom MAF Housing, lets translate the flow rates to the new diameter

		// NOTE this transfer function is actually for volumetric flow but we can assume that the mass to volumetric conversion is ratiometric at any given instant in time.
		// This is because any environmental influences apply equally to both parts of the equation and cancel each other out. 
		// So we can reduce the mass flow conversion to the same simple ratio that we use for volumetric flow. 
		// In short, the relationship between flow and pipe area applies equally to both mass flow and volumetric flow for a given instant in time.

		// Q = A*V
		// V = Q/A
		// A = PI*r^2
		// where Q = flow | A = area | V = velocity

		// Calculate the 'velocity' for the original pipe area
		mafVelocity = flowRateKGH / pow(PI * (status.mafDiameter / 2), 2);

		// scale the result with the new pipe area and convert back to mass flow
		transposedflowRateKGH = mafVelocity * PI * pow((settings.maf_housing_diameter / 2), 2);

		return transposedflowRateKGH;

	} else { // lets send the standard MAF data

		return flowRateKGH;
	}


}




/***********************************************************
 * @brief Returns flow value in CFM from differential pressure using ratiometric principle
 * @note uses calibrated orifice flow rate and assumes reference pressure is matched to orifice calibration
 * @note pRef tapped below orifice plate | pDiff tapped above orifice plate
 *
 ***/
double Sensors::getDifferentialFlow() {

	extern struct DeviceStatus status;
	extern struct SensorData sensorVal;
	extern struct BenchSettings settings;
	extern struct Language language;

	Hardware _hardware;
    Messages _message;  

	double flowRateCFM = 0.0;
    double orificeOneFlow = 0.0;
    double orificeOneDepression = 0.0;
	double orificeFlowRate = 0.0;
	double orificeDepression = 0.0;
	double flowRatio = 0.0;

	// convert orifice querystring to char 
	char activeOrifice[status.activeOrifice.length() + 1];
	strcpy(activeOrifice, status.activeOrifice.c_str()); 

	//  switchval = strtol(liftPoint.c_str(), &end, 10); // convert std::str to int

	char orifice = activeOrifice[0];

	// get orifice flow data from active orifice
	switch (orifice)   {

		case '1': 
			orificeFlowRate = settings.orificeOneFlow;
			orificeDepression = settings.orificeOneDepression;
		break;

		case '2': 
			orificeFlowRate = settings.orificeTwoFlow;
			orificeDepression = settings.orificeTwoDepression;
		break;

		case '3': 
			orificeFlowRate = settings.orificeThreeFlow;
			orificeDepression = settings.orificeThreeDepression;
		break;

		case '4': 
			orificeFlowRate = settings.orificeFourFlow;
			orificeDepression = settings.orificeFourDepression;
		break;

		case '5': 
			orificeFlowRate = settings.orificeFiveFlow;
			orificeDepression = settings.orificeFiveDepression;
		break;

		case '6': 
			orificeFlowRate = settings.orificeSixFlow;
			orificeDepression = settings.orificeSixDepression;
		break;

		// We've got here without a valid active orifice so lets set something and send an error message
        default:
			orificeFlowRate = settings.orificeOneFlow;
			orificeDepression = settings.orificeOneDepression;
			_message.serialPrintf("%s\n", "Invalid Orifice Data");
			String statusMessage = language.LANG_INVALID_ORIFICE_SELECTED;
        break;

	}

	status.activeOrificeFlowRate = orificeFlowRate;
	status.activeOrificeTestPressure = orificeDepression;

	// Calculate flow rate based on calibrated orifice data, ref pressure and current pressure drop
	flowRatio = sensorVal.PDiffKPA / sensorVal.PRefKPA;
	flowRateCFM = flowRatio * orificeFlowRate;

	return flowRateCFM;

}








/***********************************************************
 * @brief get Reference Pressure sensor voltage
 * @returns current PRef sensor value in Volts
 ***/
double Sensors::getPRefVolts() {

	Hardware _hardware;

	extern struct Configuration config;

	double sensorVolts = 0.0;

	#if defined PREF_SRC_ADC && defined PREF_IS_ENABLED && defined ADC_IS_ENABLED // use ADC value

		sensorVolts = _hardware.getADCVolts(PREF_ADC_CHANNEL);	

	#elif defined PREF_SRC_PIN && defined PREF_IS_ENABLED // use raw pin value

		long refPressRaw = analogRead(REF_PRESSURE_PIN);
		sensorVolts = refPressRaw * (_hardware.get3v3SupplyVolts() / 4095.0);

	#else // return a fixed value
	
		sensorVolts = 1.0;
		return sensorVolts;
	#endif

	// Lets make sure we have a valid value to return
	if (sensorVolts > 0.0) { 
		sensorVolts += config.PREF_MV_TRIMPOT;
		return sensorVolts;
	} else { 
		return 0.0;
	}


}





/***********************************************************
 * @brief Process Reference Pressure sensor value depending on sensor type
 * @returns Reference pressure in kPa
 * @note Default sensor MPXV7007DP - https://www.nxp.com/docs/en/data-sheet/MPXV7007.pdf
 ***/
double Sensors::getPRefValue() {

	Hardware _hardware;

	extern struct BenchSettings settings;
	extern struct Configuration config;
	
	double sensorVal = 0.0;
	double sensorVolts = this->getPRefVolts();
	
	// Convert value to kPa according to sensor type
	switch (config.PREF_SENSOR_TYPE) {

		case LINEAR_ANALOG:
			sensorVal = this->getPRefVolts() * config.PREF_ANALOG_SCALE;
		break;

		case MPXV7007:
			// Vout = Vcc x (0.057 x sensorVal + 0.5) --- Transfer function formula from MPXV7007DP Datasheet
			sensorVal = ((sensorVolts / _hardware.get5vSupplyVolts()) -0.5) / 0.057;
		break;

		case MPXV7025:
			// Vout = Vcc x (0.018 x P + 0.5)
			// P = ((Vout / Vcc) - 0.5 ) / 0.018 )
			sensorVal = ((sensorVolts / _hardware.get5vSupplyVolts()) -0.5) / 0.018;
		break;

		case XGZP6899A007KPDPN:
			// Linear response. Range = 0.5 ~ 4.5 = -7 ~ 7kPa
			sensorVal = sensorVolts * 3.5 - 8.75;
		break;

		case XGZP6899A010KPDPN:
			// Linear response. Range = 0.5 ~ 4.5 = -10 ~ 10kPa
			sensorVal = sensorVolts * 5 - 12.5;
		break;

		case M5STACK_TubePressure:
			//P: Actual test pressure value, unit (Kpa)
			//Vout: sensor voltage output value
			//P = (Vout-0.1)/3.0*300.0-100.0
			sensorVal = (sensorVolts-0.1)/3.0*300.0-100.0;	
		break;

		case NOT_USED:
			sensorVal = 0.0;
		break;

		default:
			sensorVal = config.FIXED_REF_PRESS_VALUE;
		break;

	}

	// Lets make sure we have a valid value to return - cehck itis above minimum threshold
	double pRefComp = fabs(sensorVal);
	if (pRefComp > settings.min_bench_pressure) { 
		return sensorVal;
	} else { 
		return 0.0001; // return small non zero value to prevent divide by zero errors (will be truncated to zero in display)
	}


}





/***********************************************************
 * @brief Get PDiff Volts
 * @returns Differential Pressure in Volts
 ***/
double Sensors::getPDiffVolts() {
	
	Hardware _hardware;

	extern struct Configuration config;

	double sensorVolts = 0.0;
	
	#if defined PDIFF_SRC_IS_ADC && defined PDIFF_IS_ENABLED && defined ADC_IS_ENABLED // use ADC value

		sensorVolts = _hardware.getADCVolts(PDIFF_ADC_CHANNEL);
				
	#elif defined PDIFF_SRC_IS_PIN	// Use raw pin value

		long pDiffRaw = analogRead(DIFF_PRESSURE_PIN);
		sensorVolts = pDiffRaw * (_hardware.get3v3SupplyVolts() / 4095.0);

	#else // return a fixed value

		sensorVolts = 1.0;
		return sensorVolts;

	#endif
	
	// Lets make sure we have a valid value to return
	if (sensorVolts > 0) {
		sensorVolts += config.PDIFF_MV_TRIMPOT;
		return sensorVolts;
	} else {
		return 0;
	}
	
}




/***********************************************************
 * @brief Get differential pressure in kPa
 * @returns Differential pressure in kPa
 * @note Default sensor is MPXV7007DP - Datasheet - https://www.nxp.com/docs/en/data-sheet/MPXV7007.pdf
 ***/
double Sensors::getPDiffValue() {

	Hardware _hardware;

	extern struct BenchSettings settings;
	extern struct Configuration config;

	double sensorVal = 0.0;
	double sensorVolts = this->getPDiffVolts();

	// Convert value to kPa according to sensor type
	switch (config.PDIFF_SENSOR_TYPE) {

		case NOT_USED:
			sensorVal = 0.0;
		break;

		case LINEAR_ANALOG:
			sensorVal = this->getPDiffVolts() * config.PDIFF_ANALOG_SCALE;
		break;

		case MPXV7007:
			// Vout = Vcc x (0.057 x sensorVal + 0.5) --- Transfer function formula from MPXV7007DP Datasheet
			sensorVal = ((sensorVolts / _hardware.get5vSupplyVolts()) -0.5) / 0.057;
		break;

		case MPXV7025:
			// Vout = Vcc x (0.018 x P + 0.5)
			// P = ((Vout / Vcc) - 0.5 ) / 0.018 )
			sensorVal = ((sensorVolts / _hardware.get5vSupplyVolts()) -0.5) / 0.018;
		break;

		case XGZP6899A007KPDPN:
			// Linear response. Range = 0.5 ~ 4.5 = -7 ~ 7kPa
			sensorVal = sensorVolts * 3.5 - 8.75;
		break;

		case XGZP6899A010KPDPN:
			// Linear response. Range = 0.5 ~ 4.5 = -10 ~ 10kPa
			sensorVal = sensorVolts * 5 - 12.5;
		break;

		case M5STACK_TubePressure:
			//P: Actual test pressure value, unit (Kpa)
			//Vout: sensor voltage output value
			//P = (Vout-0.1)/3.0*300.0-100.0
			sensorVal = (sensorVolts-0.1)/3.0*300.0-100.0;	
		break;

		default:
			sensorVal = config.FIXED_DIFF_PRESS_VALUE;
		break;

	}

	// Lets make sure we have a valid value to return - cehck itis above minimum threshold
	double pDiffComp = fabs(sensorVal);
	if (pDiffComp > settings.min_bench_pressure) { 
		return sensorVal;
	} else { 
		return 0.0001; // return small non zero value to prevent divide by zero errors (will be truncated to zero in display)
	}

}




/***********************************************************
 * @brief Get pitot Volts
 * @returns Pitot Pressure in Volts
 ***/
double Sensors::getPitotVolts() {

	Hardware _hardware;

	extern struct Configuration config;

	double sensorVolts = 0.0;
	
	#if defined PITOT_SRC_IS_ADC && defined PITOT_IS_ENABLED && defined ADC_IS_ENABLED // use ADC value

		sensorVolts = _hardware.getADCVolts(PITOT_ADC_CHANNEL);

	#elif defined PDIFF_SRC_IS_PIN	// use raw pin value

		long pitotRaw = analogRead(PITOT_PIN);
		sensorVolts = pitotRaw * (_hardware.get3v3SupplyVolts() / 4095.0);

	#else // use fixed value

		sensorVolts = 1.0;
		return sensorVolts;

	#endif
	
	// Lets make sure we have a valid value to return
	if (sensorVolts > 0) {
		sensorVolts += config.PITOT_MV_TRIMPOT;
		return sensorVolts;
	} else {
		return 0.0;
	}
	
	
}





/***********************************************************
 * @brief get Pitot value in kPa
 * @returns Pitot pressure differential in kPa
 * @note Default sensor MPXV7007DP - Datasheet - https://www.nxp.com/docs/en/data-sheet/MPXV7007.pdf
 ***/
double Sensors::getPitotValue() {
	
	Hardware _hardware;

	extern struct BenchSettings settings;
	extern struct Configuration config;

	double sensorVal = 0.0;
	double sensorVolts = this->getPitotVolts();
	


	// Convert value to kPa according to sensor type
	switch (config.PITOT_SENSOR_TYPE) {

		case LINEAR_ANALOG:
			sensorVal = this->getPDiffVolts() * config.PITOT_ANALOG_SCALE;
		break;

		case MPXV7007:
			// Vout = Vcc x (0.057 x sensorVal + 0.5) --- Transfer function formula from MPXV7007DP Datasheet
			sensorVal = ((sensorVolts / _hardware.get5vSupplyVolts()) -0.5) / 0.057;
		break;

		case MPXV7025:
			// Vout = Vcc x (0.018 x P + 0.5)
			// P = ((Vout / Vcc) - 0.5 ) / 0.018 )
			sensorVal = ((sensorVolts / _hardware.get5vSupplyVolts()) -0.5) / 0.018;
		break;

		case XGZP6899A007KPDPN:
			// Linear response. Range = 0.5 ~ 4.5 = -7 ~ 7kPa
			sensorVal = sensorVolts * 3.5 - 8.75;
		break;

		case XGZP6899A010KPDPN:
			// Linear response. Range = 0.5 ~ 4.5 = -10 ~ 10kPa
			sensorVal = sensorVolts * 5 - 12.5;
		break;

		case M5STACK_TubePressure:
			//P: Actual test pressure value, unit (Kpa)
			//Vout: sensor voltage output value
			//P = (Vout-0.1)/3.0*300.0-100.0
			sensorVal = (sensorVolts-0.1)/3.0*300.0-100.0;	
		break;

		case NOT_USED:
			sensorVal = 0.0;
		break;

		default:
			// Invalid Sensor selected
			sensorVal = 0.0;
		break;

	}

	// Lets make sure we have a valid value to return - cehck itis above minimum threshold
	double pDiffComp = fabs(sensorVal);
	if (pDiffComp > settings.min_bench_pressure) { 
		return sensorVal;
	} else { 
		return 0.0001; // return small non zero value to prevent divide by zero errors (will be truncated to zero in GUI)
	}

}




/***********************************************************
 * @name getTempValue
 * @brief Returns temperature in Deg C
 * @return refTempDegC
 ***/
double Sensors::getTempValue() {
	
	extern struct Configuration config;

	Hardware _hardware;
	Messages _message;


	double refTempDegC;
	
	#ifdef TEMP_SENSOR_TYPE_LINEAR_ANALOG
	
		long rawTempValue = analogRead(TEMPERATURE_PIN);	
		double tempVolts = rawTempValue * (_hardware.get3v3SupplyVolts() / 4095.0);	
		tempVolts += TEMP_MV_TRIMPOT;		
		refTempDegC = tempVolts * TEMP_ANALOG_SCALE;
		refTempDegC +=  TEMP_FINE_ADJUST;
		
	#elif defined TEMP_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED

		refTempDegC = _BMESensor.readFixedTempC() / 100.00F;

	#elif defined TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11
		// NOTE DHT11 sampling rate is max 1HZ. We may need to slow down read rate to every few secs
		int err = SimpleDHTErrSuccess;
		if ((err = dht11.read(&refTemp, &refRelh, NULL)) != SimpleDHTErrSuccess) {
		  _message.Handler(language.LANG_DHT11_READ_FAIL); // Set error to display on screen
		  refTempDegC = 0;        
		} else {
		  refTempDegC = refTemp;
		}	
		refTempDegC +=  TEMP_FINE_ADJUST;

	#else
		// We don't have any temperature input so we will assume default
		refTempDegC = config.FIXED_TEMP_VALUE;
	#endif
	
	if (config.TEMP_FINE_ADJUST != 0) refTempDegC += config.TEMP_FINE_ADJUST;
	return refTempDegC;
}



/***********************************************************
 * @name getBaroValue
 * @brief Barometric pressure in hPa
 * @returns baroPressureHpa
 * @note 1 kPa = 10 hPa | 1 hPa = 1 millibar
 ***/
double Sensors::getBaroValue() {
	
	extern struct Configuration config;

	Hardware _hardware;
	double baroPressureHpa;
		
	#if defined BARO_SENSOR_TYPE_LINEAR_ANALOG
		
		long rawBaroValue = analogRead(REF_BARO_PIN);
		double baroVolts = rawBaroValue * (_hardware.get3v3SupplyVolts() / 4095.0);
		baroVolts += BARO_MV_TRIMPOT;		
		baroPressureHpa = baroVolts * BARO_ANALOG_SCALE;
		baroPressureHpa += BARO_FINE_ADJUST;
	
	#elif defined BARO_SENSOR_TYPE_MPX4115 && defined ADC_IS_ENABLED
		// Datasheet - https://html.alldatasheet.es/html-pdf/5178/MOTOROLA/MPX4115/258/1/MPX4115.html
		// Vout = VS (P x 0.009 – 0.095) --- Where VS = Supply Voltage (Formula from Datasheet)
		baroPressureHpa = map(_hardware.getADCRawData(BARO_ADC_CHANNEL), 0, 4095, 15000, 115000);
		baroPressureHpa += BARO_FINE_ADJUST;

	#elif defined BARO_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED

		baroPressureHpa = _BMESensor.readFixedPressure() / 100.00F; 
		
	#elif defined BARO_SENSOR_TYPE_REF_PRESS_AS_BARO
		// No baro sensor defined so use value grabbed at startup from reference pressure sensor
		// NOTE will only work for absolute style pressure sensor like the MPX4250
		baroPressureKpa = startupBaroPressure; 
		baroPressureKpa += BARO_FINE_ADJUST;
	#else
		// we don't have any sensor so use default - // NOTE: standard sea level baro pressure is 14.7 psi
		baroPressureKpa = config.FIXED_BARO_VALUE;
	#endif

	// Truncate to 2 decimal places
	// int value = baroPressureKpa * 100 + .5;
    // return (double)value / 100;

	if (config.BARO_FINE_ADJUST != 0) baroPressureKpa += config.BARO_FINE_ADJUST;
	return baroPressureHpa;

}



/***********************************************************
 * @name getRelHValue
 * @brief Returns Relative Humidity in %
 * @returns relativeHumidity
 ***/
double Sensors::getRelHValue() {
	
	extern struct Configuration config;

	Hardware _hardware;

	double relativeHumidity;
		
	#ifdef RELH_SENSOR_TYPE_LINEAR_ANALOG
	
		long rawRelhValue = analogRead(HUMIDITY_PIN);
		double relhVolts = rawRelhValue * (_hardware.get3v3SupplyVolts() / 4095.0);
		relhVolts += RELH_MV_TRIMPOT;		
		relativeHumidity = relhVolts * RELH_ANALOG_SCALE;
		relativeHumidity += RELH_FINE_ADJUST;
	
	#elif defined RELH_SENSOR_TYPE_SIMPLE_RELH_DHT11
	
		// NOTE DHT11 sampling rate is max 1HZ. We may need to slow down read rate to every few secs
		int err = SimpleDHTErrSuccess;
		if ((err = dht11.read(&refTemp, &refRelh, NULL)) != SimpleDHTErrSuccess) {
		  _message.Handler(language.LANG_DHT11_READ_FAIL); // Set error to display on screen
		  relativeHumidity = 0;        
		} else {
		  relativeHumidity = refRelh;
		}
		relativeHumidity + RELH_FINE_ADJUST;

	#elif defined RELH_SENSOR_TYPE_BME280 && defined BME_IS_ENABLED

		relativeHumidity = _BMESensor.readFixedHumidity() / 1000.00F;
		
	#else
		// we don't have a sensor so use nominal fixed value 
		relativeHumidity = config.FIXED_RELH_VALUE; // (36%)
	
	#endif

	if (config.RELH_FINE_ADJUST != 0) relativeHumidity += config.RELH_FINE_ADJUST;
	return relativeHumidity;
	
}

