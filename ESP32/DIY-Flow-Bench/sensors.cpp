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
#include <vector>

#include "constants.h"
#include "structs.h"
#include "calculations.h"

#include <Wire.h>
#include "hardware.h"
#include "sensors.h"
#include "messages.h"
#include "driver/pcnt.h"
#include "mafData.h"

#define TINY_BME280_I2C
#include "TinyBME280.h" 
tiny::BME280 _BME280Sensor;
// TwoWire I2CBME = TwoWire(0);

// #include "DeeEmm_BME680.h" // TODO #233



/***********************************************************
 * @brief Class constructor
 ***/
Sensors::Sensors() {

}

// REVIEW - Frequency style MAF Begin
// NOTE: Voodoo needed to get interrupt to work within class structure for frequency measurement. 
// We declare a new instance of the Sensor class so that we can use it for the MAF ISR
// https://forum.arduino.cc/t/pointer-to-member-function/365758
// Sensors __mafVoodoo;


/***********************************************************
 * @name begin
 * @brief Sensor initialisation and set up
 ***/
void Sensors::begin () {

	Messages _message;

	_message.serialPrintf("Initialising Sensors \n");

	extern struct DeviceStatus status;
    extern struct Language language;
	extern struct Configuration config;
	extern struct Pins pins;
	extern int mafOutputType;

	MafData _maf(config.iMAF_SENS_TYP);

	//initialise BME280
	if (config.iBME_TYP == BOSCH_BME280) {

		// uint8_t I2CAddress = (unsigned int)config_ADDR;

		_message.serialPrintf("Initialising BME280: ( Address: %u )\n", config.iBME_ADDR);	
		
		if (_BME280Sensor.beginI2C((int)config.iBME_ADDR) == false) {
			_message.serialPrintf("BME sensor did not respond. \n");
			_message.serialPrintf("Please check wiring and I2C address\n");
			_message.serialPrintf("BME I2C address %s set in configuration.h. \n", config.iBME_ADDR);
			while(1); //Freeze
		} else {
			_message.serialPrintf("BME280 Initialised\n");
		}

		_BME280Sensor.setTempOverSample(1);
		_BME280Sensor.setPressureOverSample(1);
		_BME280Sensor.setHumidityOverSample(1);
		_BME280Sensor.setStandbyTime(0);
		_BME280Sensor.setFilter(0);
		_BME280Sensor.setMode(tiny::Mode::NORMAL); // NORMAL / FORCED / SLEEP

	}


	//initialise BME680
	if (config.iBME_TYP == BOSCH_BME680) {

		// TODO #233
		// BME680_Class _BME680Sensor;
	
		// _message.serialPrintf("Initialising BME680: ( Address: %u )\n", config.iBME_ADDR);	

		// // while (!_BME680Sensor.begin(I2C_STANDARD_MODE, (int)config.iBME_ADDR)) { 
		// while (!_BME680Sensor.begin(I2C_STANDARD_MODE, (int)config.iBME_ADDR)) { 
		// 	_message.serialPrintf("-  Unable to find BME680. Trying again in 5 seconds.\n");
		// 	delay(5000);
		// }  

		// _BME680Sensor.setOversampling(TemperatureSensor, Oversample16);  
		// _BME680Sensor.setOversampling(HumiditySensor, Oversample16);     
		// _BME680Sensor.setOversampling(PressureSensor, Oversample16);     
		// _BME680Sensor.setOversampling(GasSensor, SensorOff);   
		// _BME680Sensor.setIIRFilter(IIR4);  							
		// _BME680Sensor.setGas(0, 0); // Turns off gas measurements
	
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
		__mafVoodoo.mafSetupISR(MAF, []{__mafVoodoo.mafFreqCountISR();}, FALLING);
		timer = timerBegin(0, 2, true);                                  
		timerStart(timer);	
	}
*/	


	// Set status values for GUI
	status.mafSensor = _maf.getCurrentType();
	status.baroSensor = getSensorType(config.iBARO_SENS_TYP);
	status.tempSensor  = getSensorType(config.iTEMP_SENS_TYP);
	status.relhSensor = getSensorType(config.iRELH_SENS_TYP);
	status.prefSensor = getSensorType(config.iPREF_SENS_TYP);
	status.pdiffSensor = getSensorType(config.iPDIFF_SENS_TYP);
	status.pitotSensor = getSensorType(config.iPITOT_SENS_TYP);

	// END System status definitions

	_message.serialPrintf("Sensors Initialised \n");



}
/***********************************************************
  * @brief Get Sensor Type
  * @note returns description for statis pane.
  ***/
String Sensors::getSensorType(int sensorType) {

	extern struct Language language;

	String sensorDescription;

	switch (sensorType) {

		case SENSOR_DISABLED: {
			sensorDescription = language.LANG_NOT_ENABLED;
			break;
		}
		case MPXV7007: {
			sensorDescription = "MPXV7007";
			break;
		}
		case MPXV7025: {
			sensorDescription = "MPXV7025";
			break;
		}
		case LINEAR_ANALOG: {
			sensorDescription = "ANALOG PIN";
			break;
		}	
		case XGZP6899A007KPDPN: {
			sensorDescription = "XGZP6899A007KPDPN";
			break;
		}
		case XGZP6899A010KPDPN: {
			sensorDescription = "XGZP6899A010KPDPN";
			break;
		}
		case M5STACK_TubePressure: {
			sensorDescription = "TubePressure";
			break;
		}
		case BOSCH_BME280: {
			sensorDescription = "BME280";
			break;
		}
		case BOSCH_BME680: {
			sensorDescription = "BME680";
			break;
		}
		case SIMPLE_TEMP_DHT11: {
			sensorDescription = "DHT11";
			break;
		}
		case FIXED_VAL: {
			sensorDescription = "Fixed Value";
			break;
		}		
		case MPX4115: {
			sensorDescription = "MPX4115";
			break;
		}		
		default: {
			sensorDescription = language.LANG_NOT_ENABLED;
			break;
		}

	}

	return sensorDescription;
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
	extern struct Configuration config;
	extern struct Pins pins;


	switch (config.iMAF_SRC_TYP) {

		case SENSOR_DISABLED: {
			return 0;
			break;
		}

		case LINEAR_ANALOG: {
			long mafFlowRaw = analogRead(pins.MAF);
			break;
		}

		case ADS_ADC:{
			sensorVal.MafRAW = _hardware.getADCRawData(config.iMAF_ADC_CHAN);
			break;
		}

		default: {
			return 0;
			break;
		}

	}

	return sensorVal.MafRAW;


}




/***********************************************************
 * @brief getMafVolts: Returns MAF signal in Volts
 ***/
double Sensors::getMafVolts() {

	extern struct Configuration config;
	extern struct Pins pins;
	Hardware _hardware;
	double sensorVolts = 0.00F;

	switch (config.iMAF_SRC_TYP) {

		case SENSOR_DISABLED: {
			return 0;
			break;
		}

		case LINEAR_ANALOG: {
			long mafRaw = analogRead(pins.MAF);
			sensorVolts = static_cast<double>(mafRaw) * (_hardware.get3v3SupplyVolts() / 4095.00F);
			break;
		}

		case ADS_ADC:{
			sensorVolts = _hardware.getADCVolts(config.iMAF_ADC_CHAN);
			break;
		}

		default: {
			// return a fixed value to aid fault diagnosis
			sensorVolts = 1.0;
			return sensorVolts;
			break;
		}
	}

	// Trim value	
	sensorVolts += config.dMAF_MV_TRIM;

	// Lets make sure we have a valid value to return
	if (sensorVolts > 0) {
		return sensorVolts; 
	} else {
		return 0.0;
	}	
	
}





/***********************************************************
 * @brief Returns MAF mass flow value in KG/H 
 * @note Interpolates lookupvalue from datatable key>value pairs
 * @returns Mass flow in KG/H
 *
 ***/
double Sensors::getMafFlow(int units) {

	extern struct DeviceStatus status;
	extern struct SensorData sensorVal;
	extern struct BenchSettings settings;
	extern struct SensorData sensorVal;
	extern struct Configuration config;

	Hardware _hardware;
	Messages _message;
	Calculations _calculations;
	MafData _maf(config.iMAF_SENS_TYP);

	double flowRateCFM = 0.0;
	double flowRateMGS = 0.0;
	double flowRateKGH = 0.0;
	double lookupValue = 0.0;
	double oldMafArea = 0.0;
	double newMafArea = 0.0;
	double velocity1 = 0.0;
	double velocity2 = 0.0;
	double mafVelocity = 0.0;
	double transposedflowRateKGH = 0.0;
	double mafVolts = 0.0;
	u_int mafMilliVolts = 0;
	double MafFlow = 0.0f;
    float vPower = 1.0f;


	// get MAF Coefficients
	float mafCoeff0 = _maf.getCoefficient(0);
    float mafCoeff1 = _maf.getCoefficient(1);
	float mafCoeff2 = _maf.getCoefficient(2);
	float mafCoeff3 = _maf.getCoefficient(3);
	float mafCoeff4 = _maf.getCoefficient(4);
	float mafCoeff5 = _maf.getCoefficient(5);
	float mafCoeff6 = _maf.getCoefficient(6);

	// Get MAF Volts
	sensorVal.MafVolts = this->getMafVolts();

	// VCC deviation correction
	mafVolts = ((5.0f / _hardware.get5vSupplyVolts()) * sensorVal.MafVolts);
	
	mafMilliVolts = mafVolts * 1000;

	// 6th degree polynomial calculation (Coefficients stored in mafData class)
	// flowRateKGH = mafCoeff0 + (mafCoeff1 * mafMilliVolts) + (mafCoeff2 * pow(mafMilliVolts, 2)) + (mafCoeff3 * pow(mafMilliVolts, 3)) + (mafCoeff4 * pow(mafMilliVolts, 4)) + (mafCoeff5 * pow(mafMilliVolts, 5)) + (mafCoeff6 * pow(mafMilliVolts, 6));

	// flowRateKGH = _maf.calculateFlow(mafVolts);

	// Alternate method
	// 6th degree polynomial calculation (Coefficients stored in mafData class)
    for(int i = 0; i < 7; i++) {
        flowRateKGH += _maf.getCoefficient(i) * vPower;
        vPower *= mafMilliVolts;
    }

	

	status.mafDiameter = _maf.getDiameter();



	// Now that we have a converted flow value we can translate it for different housing diameters
	if (settings.maf_housing_diameter > 0 && status.mafDiameter > 0 && settings.maf_housing_diameter != status.mafDiameter) { 
		// We are running a custom MAF Housing, lets translate the flow rates to the new diameter

		// NOTE this transfer function is actually for volumetric flow (m3/s) but we can assume that the mass to volumetric conversion is ratiometric at any given instant in time.
		// This is because any environmental influences apply equally to both parts of the equation and cancel each other out. 
		// So we can reduce the mass flow conversion to the same simple ratio that we use for volumetric flow. 
		// In short, the relationship between flow and pipe area applies equally to both mass flow and volumetric flow for any given instant in time.

		// Q = V*A
		// V = Q/A
		// A = PI*r^2
		// Q1 = Q2
		// V1*A1 = V2*A2
		// where Q = volumetric flow (m3/s) | A = area(m2) | V = velocity
		
		// Calculate original MAF area
		oldMafArea = (PI * pow((static_cast<double>(status.mafDiameter) / 2.0F), 2.00F)) / 1000000.00F;
		// Calculate new MAF area
		newMafArea = (PI * pow((static_cast<double>(settings.maf_housing_diameter) / 2.00F), 2.00F)) / 1000000.00F;

		// scale the result with the new pipe area and convert back to mass flow
		transposedflowRateKGH = (flowRateKGH / oldMafArea) * newMafArea;

		return transposedflowRateKGH;

	} else { 

		// lets send the standard MAF data
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
	extern struct Pins pins;
	extern struct SensorData sensorVal;

	double sensorVolts = 0.0;

	switch (config.iPREF_SRC_TYP) 	{

		case ADS_ADC : {
			sensorVolts = _hardware.getADCVolts(config.iPREF_ADC_CHAN);
			break;
		}

		case LINEAR_ANALOG : {
			long refPressRaw = analogRead(pins.PREF);
			sensorVolts = static_cast<double>(refPressRaw) * (_hardware.get3v3SupplyVolts() / 4095.00F);
			break;
		}

		default: {
			// return a fixed value to aid fault diagnosis
			sensorVolts = 1.0;
			return sensorVolts;
			break;
		}
	} 


	// Trim value
	sensorVolts += config.dPREF_MV_TRIM;

	// Lets make sure we have a valid value to return
	if (sensorVolts > 0.0) { 
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
	Messages _message;

	extern struct BenchSettings settings;
	extern struct Configuration config;
	extern struct SensorData sensorVal;

	double returnVal = 0.0;
	sensorVal.PRefVolts = getPRefVolts();

	switch (config.iPREF_SENS_TYP)  {

		case LINEAR_ANALOG: 	
				returnVal = sensorVal.PRefVolts * config.dPREF_ALG_SCALE;
		break;

		case MPXV7007:
			// Vout = Vcc x (0.057 x sensorVal + 0.5) --- Transfer function formula from MPXV7007DP Datasheet
			returnVal = ((sensorVal.PRefVolts / _hardware.get5vSupplyVolts()) -0.5) / 0.057;
		break;

		case MPXV7025:
			// Vout = Vcc x (0.018 x P + 0.5)
			// P = ((Vout / Vcc) - 0.5 ) / 0.018 )
			returnVal = ((sensorVal.PRefVolts / _hardware.get5vSupplyVolts()) -0.5) / 0.018;
		break;

		case XGZP6899A007KPDPN :
			// Linear response. Range = 0.5 ~ 4.5 = -7 ~ 7kPa
			returnVal = sensorVal.PRefVolts * 3.5 - 8.75;

		case XGZP6899A010KPDPN:
			// Linear response. Range = 0.5 ~ 4.5 = -10 ~ 10kPa
			returnVal = sensorVal.PRefVolts * 5 - 12.5;
		break;

		case M5STACK_TubePressure:
			//P: Actual test pressure value, unit (Kpa)
			//Vout: sensor voltage output value
			//P = (Vout-0.1)/3.0*300.0-100.0
			returnVal = (sensorVal.PRefVolts-0.1)/3.0*300.0-100.0;	
		break;

		default:
			returnVal = config.iFIXED_PREF_VAL;
		break;
	}

	// Flip negative values
	double pRefComp = fabs(returnVal);

	// Lets make sure we have a valid value to return
	if (pRefComp > settings.min_bench_pressure) {
		return returnVal;
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
	extern struct Pins pins;

	double sensorVolts = 0.0;

	switch (config.iPDIFF_SRC_TYP) 	{

		case ADS_ADC : {
			sensorVolts = _hardware.getADCVolts(config.iPDIFF_ADC_CHAN);
			break;
		}

		case LINEAR_ANALOG : {
			long pDiffRaw = analogRead(pins.PDIFF);
			sensorVolts = static_cast<double>(pDiffRaw) * (_hardware.get3v3SupplyVolts() / 4095.00F);
			break;
		}

		default: {
			// return a fixed value to aid fault diagnosis
			sensorVolts = 1.0;
			return sensorVolts;
			break;
		}
	} 


	// Trim vlaue
	sensorVolts += config.dPDIFF_MV_TRIM;

	// Lets make sure we have a valid value to return
	if (sensorVolts > 0) {
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

	extern struct BenchSettings settings; 
	extern struct Configuration config;
	extern struct SensorData sensorVal;

	Hardware _hardware;

	double returnVal = 0.0;
	sensorVal.PDiffVolts = this->getPDiffVolts();

	switch (config.iPDIFF_SENS_TYP)  {

			case LINEAR_ANALOG: 	
					returnVal = sensorVal.PDiffVolts * config.dPDIFF_SCALE;
			break;

			case MPXV7007:
				// Vout = Vcc x (0.057 x sensorVal + 0.5) --- Transfer function formula from MPXV7007DP Datasheet
				returnVal = ((sensorVal.PDiffVolts / _hardware.get5vSupplyVolts()) -0.5) / 0.057;
			break;

			case MPXV7025:
				// Vout = Vcc x (0.018 x P + 0.5)
				// P = ((Vout / Vcc) - 0.5 ) / 0.018 )
				returnVal = ((sensorVal.PDiffVolts / _hardware.get5vSupplyVolts()) -0.5) / 0.018;
			break;

			case XGZP6899A007KPDPN:
				// Linear response. Range = 0.5 ~ 4.5 = -7 ~ 7kPa
				returnVal = sensorVal.PDiffVolts * 3.5 - 8.75;

			case XGZP6899A010KPDPN:
				// Linear response. Range = 0.5 ~ 4.5 = -10 ~ 10kPa
				returnVal = sensorVal.PDiffVolts * 5 - 12.5;
			break;

			case M5STACK_TubePressure:
				//P: Actual test pressure value, unit (Kpa)
				//Vout: sensor voltage output value
				//P = (Vout-0.1)/3.0*300.0-100.0
				returnVal = (sensorVal.PDiffVolts-0.1)/3.0*300.0-100.0;	
			break;

			default:
				returnVal = config.iFIXD_PDIFF_VAL;
			break;
		}

	// Flip negative values
	double pDiffComp = fabs(returnVal);

	// Lets make sure we have a valid value to return - check it is above minimum threshold
	if (pDiffComp > settings.min_bench_pressure) { 
		return returnVal;
	} else { 
		return 0.0001; // return small non zero value to prevent divide by zero errors (will be truncated to zero in display)
	}	

}




/***********************************************************
 * @brief Get pitot Volts
 * @returns Pitot Pressure in Volts
 ***/
double Sensors::getPitotVolts() {

	extern struct Configuration config;
	extern struct Pins pins;

	Hardware _hardware;

	double sensorVolts = 0.0;

		switch (config.iPITOT_SRC_TYP) 	{

		case ADS_ADC : {
			sensorVolts = _hardware.getADCVolts(config.iPITOT_ADC_CHAN);
			break;
		}

		case LINEAR_ANALOG : {
			long pDiffRaw = analogRead(pins.PITOT);
			sensorVolts = static_cast<double>(pDiffRaw) * (_hardware.get3v3SupplyVolts() / 4095.00F);
			break;
		}

		default: {
			// return a fixed value to aid fault diagnosis
			sensorVolts = 1.0;
			return sensorVolts;
			break;
		}
	} 

	

	// Trim value
	sensorVolts += config.dPITOT_MV_TRIM;
	
	// Lets make sure we have a valid value to return
	if (sensorVolts > 0) {
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
	
	extern struct BenchSettings settings;
	extern struct CalibrationData calVal;
	extern struct Configuration config;
	extern struct SensorData sensorVal;

	Calculations _calculations;
	Hardware _hardware;

	double pitotPressure = 0.0;
	double airDensity = 0.0;
	double airVelocity = 0.0;
	double totalPressure = 0.0;
	double staticPressure = 0.0;	
	sensorVal.PitotVolts = this->getPitotVolts();

	double returnVal = 0.0;

	switch (config.iPITOT_SENS_TYP)  {

		case LINEAR_ANALOG: 	
				returnVal = sensorVal.PitotVolts * config.dPITOT_SCALE;
		break;

		case MPXV7007:
			// Vout = Vcc x (0.057 x sensorVal + 0.5) --- Transfer function formula from MPXV7007DP Datasheet
			returnVal = ((sensorVal.PitotVolts / _hardware.get5vSupplyVolts()) -0.5) / 0.057;
		break;

		case MPXV7025:
			// Vout = Vcc x (0.018 x P + 0.5)
			// P = ((Vout / Vcc) - 0.5 ) / 0.018 )
			returnVal = ((sensorVal.PitotVolts / _hardware.get5vSupplyVolts()) -0.5) / 0.018;
		break;

		case XGZP6899A007KPDPN:
			// Linear response. Range = 0.5 ~ 4.5 = -7 ~ 7kPa
			returnVal = sensorVal.PitotVolts * 3.5 - 8.75;

		case XGZP6899A010KPDPN:
			// Linear response. Range = 0.5 ~ 4.5 = -10 ~ 10kPa
			returnVal = sensorVal.PitotVolts * 5 - 12.5;
		break;

		case M5STACK_TubePressure:
			//P: Actual test pressure value, unit (Kpa)
			//Vout: sensor voltage output value
			//P = (Vout-0.1)/3.0*300.0-100.0
			returnVal = (sensorVal.PitotVolts-0.1)/3.0*300.0-100.0;	
		break;

		default:
			returnVal = config.iFIXD_PDIFF_VAL;
		break;
	}


	// Flip negative values
	double pitotComp = fabs(returnVal); 

	// Lets make sure we have a valid value to return - check it is above minimum threshold
	if (pitotComp > settings.min_bench_pressure) { 
		return pitotComp;
	} else { 
		return 0.0001; // return small non zero value to prevent divide by zero errors (will be truncated to zero in display)
	}	

}






/***********************************************************
 * @brief get Pitot velocity in m/sec
 * @returns Pitot velocity in metres per second
 * @note Default sensor MPXV7007DP - Datasheet - https://www.nxp.com/docs/en/data-sheet/MPXV7007.pdf
 ***/
double Sensors::getPitotVelocity() {
	
	extern struct BenchSettings settings;
	extern struct SensorData sensorVal;
	extern struct CalibrationData calVal;

	Calculations _calculations;

	double pitotPressure = 0.0;
	double sensorVolts = this->getPitotVolts();
	double airDensity = 0.0;
	double airVelocity = 0.0;
	double totalPressure = 0.0;
	double staticPressure = 0.0;	
	
	pitotPressure = sensorVal.PitotKPA;

	// get air density
	airDensity = _calculations.calculateAirDensity(sensorVal.TempDegC, sensorVal.BaroKPA, sensorVal.RelH);

	// Convert Pitot pressure to velocity (m3/min)
	// airVelocity = sqrt(2 * (pitotPressure - sensorVal.PRefKPA) / airDensity );
	airVelocity = sqrt(2 * (pitotPressure) / airDensity );

	// Lets make sure we have a valid value to return - check it is above minimum threshold 
	// TODO - 'borrowing' min_bench_pressure
	double pitotComp = fabs(airVelocity);
	if (pitotComp > settings.min_bench_pressure) { 
		return pitotComp;
	} else { 
		return 0.0001; // return small non zero value to prevent divide by zero errors (will be truncated to zero in display)
	}	

}




/***********************************************************
 * @name getTempValue
 * @brief Returns temperature in Deg C
 * @return refTempDegC
 ***/
double Sensors::getTempValue() {
	
	Hardware _hardware;
	Messages _message;

	extern struct SensorData sensorVal;
	extern struct Configuration config;
	extern struct Pins pins;

	double  refTempDegC;
	int32_t  refTempDegC_INT;
	int32_t  unusedRH, unusedBaro, unusedGas;
	

	switch (config.iTEMP_SENS_TYP) {

		case LINEAR_ANALOG: {
			long rawTempValue = analogRead(pins.TEMPERATURE);	
			double tempVolts = rawTempValue * (_hardware.get3v3SupplyVolts() / 4095.0);	
			tempVolts += config.dTEMP_MV_TRIM;		
			refTempDegC = tempVolts * config.dTEMP_ALG_SCALE;
			refTempDegC +=  config.dTEMP_FINE_TUNE;
			break;
		}

		case BOSCH_BME280 : {
			refTempDegC = _BME280Sensor.readFixedTempC() / 100.00F;
			break;
		}

		case BOSCH_BME680 : {
			// _BME680Sensor.getSensorData(refTempDegC_INT, unusedRH, unusedBaro, unusedGas, false);
			// refTempDegC = double(refTempDegC_INT / 100.00F);
			// sensorVal.test = refTempDegC_INT;
			break;
		}

		case SIMPLE_TEMP_DHT11: {
			// NOTE DHT11 sampling rate is max 1HZ. We may need to slow down read rate to every few secs
			// int err = SimpleDHTErrSuccess;
			// if ((err = dht11.read(&refTemp, &refRelh, NULL)) != SimpleDHTErrSuccess) {
			// _message.Handler(language.LANG_DHT11_READ_FAIL); // Set error to display on screen
			// refTempDegC = 0;        
			// } else {
			// refTempDegC = refTemp;
			// }	
			// refTempDegC +=  dTEMP_FINE_TUNE;
			break;
		}

		default: {
			refTempDegC = config.dFIXED_TEMP_VAL;
			break;
		}

	}

	// Trim value
	refTempDegC += config.dTEMP_FINE_TUNE;

	return refTempDegC;
}



/***********************************************************
 * @name getBaroValue
 * @brief Barometric pressure in hPa
 * @returns baroPressureHpa
 * @note 1 kPa = 10 hPa | 1 hPa = 1 millibar
 ***/
double Sensors::getBaroValue() {
	
	Hardware _hardware;
	Messages _message;

	extern struct Configuration config;
	extern struct Pins pins;

	double baroPressureHpa;
	int32_t  baroPressureHpa_INT;
	int32_t  unusedTemp;   
	int32_t  unusedRH;   
	int32_t  unusedGas;   


	switch (config.iBARO_SENS_TYP) {

		case LINEAR_ANALOG: {
			long rawBaroValue = analogRead(pins.REF_BARO);
			double baroVolts = rawBaroValue * (_hardware.get3v3SupplyVolts() / 4095.0);
			baroVolts += config.dBARO_MV_TRIM;		
			baroPressureHpa = baroVolts * config.dBARO_ALG_SCALE;
			baroPressureHpa += config.dBARO_FINE_TUNE;
			break;
		}

		case MPX4115: {
			// Datasheet - https://html.alldatasheet.es/html-pdf/5178/MOTOROLA/MPX4115/258/1/MPX4115.html
			// Vout = VS (P x 0.009 – 0.095) --- Where VS = Supply Voltage (Formula from Datasheet)
			baroPressureHpa = map(_hardware.getADCRawData(config.iBARO_ADC_CHAN), 0, 4095, 15000, 115000);
			baroPressureHpa += config.dBARO_FINE_TUNE;
			break;
		}

		case BOSCH_BME280: {
			baroPressureHpa = _BME280Sensor.readFixedPressure() / 100.00F; 
			break;
		}

		case BOSCH_BME680: {
			// _BME680Sensor.getSensorData(unusedTemp, unusedRH, baroPressureHpa_INT, unusedGas);
			// baroPressureHpa = baroPressureHpa_INT / 100.00F; 
			// break;
		}

		case REF_PRESS_AS_BARO: {
			// No baro sensor defined so use value grabbed at startup from reference pressure sensor
			// NOTE will only work for absolute style pressure sensor like the MPX4250
			baroPressureKpa = startupBaroPressure; 
			baroPressureKpa += config.dBARO_FINE_TUNE;
			break;
		}

		default: {
			baroPressureKpa = config.dFIXD_BARO_VAL;
			break;
		}
	}

	// Trim Value
	baroPressureHpa += config.dBARO_FINE_TUNE;

	return baroPressureHpa;

}



/***********************************************************
 * @name getRelHValue
 * @brief Returns Relative Humidity in %
 * @returns relativeHumidity
 ***/
double Sensors::getRelHValue() {
	
	Hardware _hardware;
	Messages _message;

	extern struct Configuration config;
	extern struct Pins pins;

	double relativeHumidity;
	int32_t  relativeHumidity_INT;
	int32_t  unusedTemp;   
	int32_t  unusedBaro;   
	int32_t  unusedGas;   		

	switch (config.iRELH_SENS_TYP){

		case LINEAR_ANALOG: {
			long rawRelhValue = analogRead(pins.HUMIDITY);
			double relhVolts = rawRelhValue * (_hardware.get3v3SupplyVolts() / 4095.0);
			relhVolts += config.dRELH_MV_TRIM;		
			relativeHumidity = relhVolts * config.dRELH_ALG_SCALE;
			relativeHumidity += config.dRELH_FINE_TUNE;
			break;
		}

		case SIMPLE_TEMP_DHT11: {
			// NOTE DHT11 sampling rate is max 1HZ. We may need to slow down read rate to every few secs
			// int err = SimpleDHTErrSuccess;
			// if ((err = dht11.read(&refTemp, &refRelh, NULL)) != SimpleDHTErrSuccess) {
			// _message.Handler(language.LANG_DHT11_READ_FAIL); // Set error to display on screen
			// relativeHumidity = 0;        
			// } else {
			// relativeHumidity = refRelh;
			// }
			// relativeHumidity + config.dRELH_FINE_TUNE;
			break;
		}

		case BOSCH_BME280: {
			relativeHumidity = _BME280Sensor.readFixedHumidity() / 1000.00F;
			break;
		}

		case BOSCH_BME680: {
			// _BME680Sensor.getSensorData(unusedTemp, relativeHumidity_INT, unusedBaro, unusedGas);
			// relativeHumidity = relativeHumidity_INT / 1000.00F;
			break;
		}

		default: {
			relativeHumidity = config.dFIXED_RELH_VAL; // (36%)
			break;
		}


	}

	// Trim Value
	relativeHumidity += config.dRELH_FINE_TUNE;

	return relativeHumidity;
	
}
