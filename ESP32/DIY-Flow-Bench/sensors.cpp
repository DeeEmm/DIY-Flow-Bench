/***********************************************************
 * The DIY Flow Bench project
 * https://diyflowbench.com
 * 
 * Sensors.cpp - Sensors class
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
 * NOTE: Methods return sensor values in (SI) standard units. This allows any sensor to be integrated into the system
 * 
 ***/

#include "Arduino.h"
#include "configuration.h"
#include "constants.h"
#include "pins.h"
#include "structs.h"
#include "hardware.h"
#include "sensors.h"
#include "driver/pcnt.h"

#include LANGUAGE_FILE
//#include MAF_SENSOR_FILE


Sensors::Sensors() {

	// Zero out the sensor values
	this->_maf = 0.0;
	this->_baro = 0.0;
	this->_relh = 0.0;
	this->_temp = 0.0;
	this->_pref = 0.0;
	this->_pdiff = 0.0;
	this->_pitot = 0.0;
	
	
	
	extern String mafSensorType;
	extern int MAFoutputType;
	extern int MAFdataUnit;
	//extern long mafLookupTable[][2];
	
	
	this->_mafSensorType = mafSensorType;
	this->_mafOutputType  = MAFoutputType;
	this->_mafDataUnit = MAFdataUnit;
	

}

void Sensors::Begin () {
	// What hardware / sensors do we have? Lets initialise them and report back to calling function
	
	// Support for ADAFRUIT_BME280 temp, pressure & Humidity sensors
	// https://github.com/adafruit/Adafruit_BME280_Library
	#if defined TEMP_SENSOR_TYPE_ADAFRUIT_BME280 || defined BARO_SENSOR_TYPE_ADAFRUIT_BME280 || defined RELH_SENSOR_TYPE_ADAFRUIT_BME280 
		// TODO: Getting some error messages
		//'Sensors::initialise()::TwoWire Wire', declared using local type 'Sensors::initialise()::TwoWire', is used but never defined 
		// #include <Adafruit_BME280.h> 
		// Adafruit_BME280 adafruitBme280; // Instantiate (create) a BMP280_DEV object and set-up for I2C operation (address 0x77)
		// //I2C address - BME280_I2C_ADDR
		// if (!adafruitBme280.begin()) {  
		// 	return BME280_READ_FAIL;			
		//   Serial.println("Adafruit BME280 Initialisation failed");      
		// } else {
		//   Serial.println("Adafruit BME280 Initialised");      
		// }
	#endif
		
		
	// Support for SPARKFUN_BME280 temp, pressure & Humidity sensors
	// https://learn.sparkfun.com/tutorials/sparkfun-bme280-breakout-hookup-guide?_ga=2.39864294.574007306.1596270790-134320310.1596270790
	#if defined TEMP_SENSOR_TYPE_SPARKFUN_BME280 || defined BARO_SENSOR_TYPE_SPARKFUN_BME280 || defined RELH_SENSOR_TYPE_SPARKFUN_BME280 
		// TODO: There is an issue with this library. Was always a bit flakey but more so now it's inside this class.
		// #include "SparkFunBME280.h"
		// #include "Wire.h"
		// BME280 SparkFunBME280;
		// Wire.begin();
		// SparkFunBME280.setI2CAddress(BME280_I2C_ADDR); 
		// if (SparkFunBME280.beginI2C() == false) //Begin communication over I2C
		// {
		// 	return BME280_READ_FAIL;			
		//   Serial.println("Sparkfun BME280 Initialisation failed");      
		// } else {
		//   Serial.println("Sparkfun BME280 Initialised");      
		// }
	#endif


	// Support for DHT11 humidity / temperature sensors
	// https://github.com/winlinvip/SimpleDHT
	#if defined TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11 || defined RELH_SENSOR_TYPE_SIMPLE_RELH_DHT11 
		#include <SimpleDHT.h>    		 
	#endif

}


void Sensors::Initialise () {

	extern struct DeviceStatus status;

	// Baro Sensor
	#ifdef BARO_SENSOR_TYPE_REF_PRESS_AS_BARO
		this->startupBaroPressure = this->PRef(KPA);
		this->_baroSensorType = LANG_START_REF_PRESSURE;
	#elif defined BARO_SENSOR_TYPE_FIXED_VALUE
		this->startupBaroPressure = DEFAULT_BARO;
		this->_baroSensorType = LANG_FIXED_VALUE;
		this->_baroSensorType += DEFAULT_BARO;
	#elif defined BARO_SENSOR_TYPE_SPARKFUN_BME280
		this->_baroSensorType = "Sparkfun BME280";
	#elif defined BARO_SENSOR_TYPE_ADAFRUIT_BME280
		this->_baroSensorType = "Adafruit BME280";
	#elif defined BARO_SENSOR_TYPE_MPX4115
		this->_baroSensorType = "MPX4115";
	#endif
	
	//Temp Sensor
	#ifdef TEMP_SENSOR_NOT_USED
		this->startupBaroPressure = this->PRef(KPA);
		this->_tempSensorType = LANG_NOT_ENABLED;
	#elif defined TEMP_SENSOR_TYPE_FIXED_VALUE
		this->_tempSensorType = LANG_FIXED_VALUE;
		this->_tempSensorType += DEFAULT_TEMP;
	#elif defined TEMP_SENSOR_TYPE_SPARKFUN_BME280
		this->_tempSensorType = "Sparkfun BME280";
	#elif defined TEMP_SENSOR_TYPE_ADAFRUIT_BME280
		this->_tempSensorType = "Adafruit BME280";
	#elif defined TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11
		this->_tempSensorType = "Simple DHT11";
	#endif
	
	// Rel Humidity Sensor
	#ifdef RELH_SENSOR_NOT_USED
		this->startupBaroPressure = this->PRef(KPA);
		this->_relhSensorType = LANG_NOT_ENABLED;
	#elif defined RELH_SENSOR_TYPE_FIXED_VALUE
		this->_relhSensorType = LANG_FIXED_VALUE;
		this->_relhSensorType += DEFAULT_RELH;
	#elif defined RELH_SENSOR_TYPE_SPARKFUN_BME280
		this->_relhSensorType = "Sparkfun BME280";
	#elif defined RELH_SENSOR_TYPE_ADAFRUIT_BME280
		this->_relhSensorType = "Adafruit BME280";
	#elif defined RELH_SENSOR_TYPE_SIMPLE_TEMP_DHT11
		this->_relhSensorType = "Simple DHT11";
	#endif


	#ifdef PREF_SENSOR_NOT_USED
		this->_prefSensorType = LANG_NOT_ENABLED;
	#elif defined PREF_SENSOR_TYPE_MPXV7007
		this->_prefSensorType = "SMPXV7007";
	#elif defined PREF_SENSOR_TYPE_PX4250
		this->_prefSensorType = "PX4250";
	#endif


    #ifdef PITOT_SENSOR_NOT_USED
		this->_pitotSensorType = LANG_NOT_ENABLED;
	#elif defined PITOT_SENSOR_TYPE_MPXV7007
		this->_pitotSensorType = "SMPXV7007";
	#endif
  
	

	// Set status values for GUI
	status.mafSensor = this->_mafSensorType;
	status.baroSensor = this->_baroSensorType;
	status.tempSensor = this->_tempSensorType;
	status.relhSensor = this->_relhSensorType;
	status.prefSensor = this->_prefSensorType;
	status.pdiffSensor = this->_pdiffSensorType;
	status.pitotSensor = this->_pitotSensorType;

}






/***********************************************************
 * Returns temperature in Deg C
 ***/
float Sensors::getTempValue() {
	
	Hardware _hardware;
	float refTempDegC;


	#ifdef TEMP_SENSOR_TYPE_ADAFRUIT_BME280
		refTempDegC  =  adafruitBme280.readTemperature();
	#elif defined TEMP_SENSOR_TYPE_SPARKFUN_BME280
		refTempDegC =  SparkFunBME280.readTempC();
	#elif defined TEMP_SENSOR_TYPE_SIMPLE_TEMP_DHT11
		// NOTE DHT11 sampling rate is max 1HZ. We may need to slow down read rate to every few secs
		int err = SimpleDHTErrSuccess;
		if ((err = dht11.read(&refTemp, &refRelh, NULL)) != SimpleDHTErrSuccess) {
		  _message.Handler(LANG_DHT11_READ_FAIL); // Set error to display on screen
		  refTempDegC = 0;        
		} else {
		  refTempDegC = refTemp;
		}	
	#elif defined TEMP_SENSOR_TYPE_LINEAR_ANALOG
		refTempDegC = analogRead(TEMPERATURE_PIN);
		_temp = refTempDegC * TEMP_ANALOG_SCALE;
	#else
		// We don't have any temperature input so we will assume default
		refTempDegC = DEFAULT_TEMP;
	#endif
	
	refTempDegC += TEMP_TRIMPOT;

	return _temp;
}



/***********************************************************
 * Returns Barometric pressure in kPa
 ***/
float Sensors::getBaroValue() {
	
	Hardware _hardware;
	float baroKpaRaw;
	
	//#elif defined BARO_SENSOR_TYPE_LINEAR_ANALOG
	baroKpaRaw = analogRead(REF_BARO_PIN);
	
	double baroMillivolts = (baroKpaRaw * (_hardware.getSupplyMillivolts() / 4095.0)) * 1000;
	baroMillivolts += BARO_TRIMPOT;		
	
	
	_baro = baroMillivolts * BARO_ANALOG_SCALE;
	
	
	
	return _baro;
}



/***********************************************************
 * Returns Relative Humidity in %
 ***/
float Sensors::getRelHValue() {
	
	Hardware _hardware;
	float relhRaw;
	
	
	//#elif defined RELH_SENSOR_TYPE_LINEAR_ANALOG
	relhRaw = analogRead(HUMIDITY_PIN);
	
	double relhMillivolts = (relhRaw * (_hardware.getSupplyMillivolts() / 4095.0)) * 1000;
	relhMillivolts += RELH_TRIMPOT;
	
	_relh = relhMillivolts * RELH_ANALOG_SCALE;



	return _relh;
}



/***********************************************************
 * Returns Reference pressure in kPa
 ***/
float Sensors::getPRefValue() {
	
	Hardware _hardware;

	// Ref Pressure Voltage
	int refPressRaw = analogRead(REF_PRESSURE_PIN);
	double prefMillivolts = (refPressRaw * (_hardware.getSupplyMillivolts() / 4095.0)) * 1000;
	prefMillivolts += PREF_TRIMPOT;

	return _pref;
}



/***********************************************************
 * Returns Pressure differential in kPa
 ***/
float Sensors::getPDiffValue() {
	
	Hardware _hardware;

	return _pdiff;
}



/***********************************************************
 * Returns Pitot pressure differential in kPa
 ***/
float Sensors::getPitotValue() {
	
	Hardware _hardware;

	// Pitot Voltage
	int pitotRaw = analogRead(PITOT_PIN);
	double pitotMillivolts = (pitotRaw * (_hardware.getSupplyMillivolts() / 4095.0)) * 1000;
	pitotMillivolts += PITOT_TRIMPOT;

	return _pitot;
}





/***********************************************************
 * Returns RAW MAF Sensor value
 *
 * MAF decode is done in Maths.cpp
 ***/
float Sensors::getMafValue() {
	
	Hardware _hardware;
	
	switch (this->_mafOutputType) {
		case VOLTAGE:
		{
			int mafFlowRaw = analogRead(MAF_PIN);
			double mafMillivolts = (mafFlowRaw * (_hardware.getSupplyMillivolts() / 4095.0)) * 1000;
			mafMillivolts += MAF_TRIMPOT;
			return mafMillivolts;
			
			
		}
		break;
		
		case FREQUENCY:
		{
			//pinMode(MAF_PIN, INPUT);
			//attachInterrupt(MAF_PIN, Ext_INT1_ISR, RISING);
			
			double mafFrequency = 0.0;
			mafFrequency += MAF_TRIMPOT;
			return mafFrequency;
		}
		break;
		
	}
	
	return _maf;

}



/*
The MAF output frequency is a function of the power required to keep the air flow sensing elements (hot wires) at a fixed temperature above the ambient temperature. Air flowing through the sensor cools the sensing elements. The amount of cooling is proportional to the amount of air flow. The MAF sensor requires a greater amount of current in order to maintain the hot wires at a constant temperature as the air flow increases. The MAF sensor converts the changes in current draw to a frequency signal read by the PCM. The PCM calculates the air flow (grams per second) based on this signal.
*/



// https://github.com/DavidAntliff/esp32-freqcount/blob/master/frequency_count.c

/*

static void init_pcnt(uint8_t pulse_gpio, uint8_t ctrl_gpio, pcnt_unit_t unit, pcnt_channel_t channel, uint16_t filter_length)
{
	ESP_LOGD(TAG, "%s", __FUNCTION__);

	// set up counter
	pcnt_config_t pcnt_config = {
		.pulse_gpio_num = pulse_gpio,
		.ctrl_gpio_num = ctrl_gpio,
		.lctrl_mode = PCNT_MODE_DISABLE,
		.hctrl_mode = PCNT_MODE_KEEP,
		.pos_mode = PCNT_COUNT_INC,  // count both rising and falling edges
		.neg_mode = PCNT_COUNT_INC,
		.counter_h_lim = 0,
		.counter_l_lim = 0,
		.unit = unit,
		.channel = channel,
	};

	ESP_ERROR_CHECK(pcnt_unit_config(&pcnt_config));

	// set the GPIO back to high-impedance, as pcnt_unit_config sets it as pull-up
	ESP_ERROR_CHECK(gpio_set_pull_mode(pulse_gpio, GPIO_FLOATING));

	// enable counter filter - at 80MHz APB CLK, 1000 pulses is max 80,000 Hz, so ignore pulses less than 12.5 us.
	ESP_ERROR_CHECK(pcnt_set_filter_value(unit, filter_length));
	ESP_ERROR_CHECK(pcnt_filter_enable(unit));
}

void frequency_count_task_function(void * pvParameter)
{
	frequency_count_configuration_t configuration;

	assert(pvParameter);
	ESP_LOGI(TAG, "Core ID %d", xPortGetCoreID());

	configuration = *(frequency_count_configuration_t*)pvParameter;
	frequency_count_configuration_t *task_inputs = &configuration;

	ESP_LOGI(TAG, "pcnt_gpio %d, pcnt_unit %d, pcnt_channel %d, rmt_gpio %d, rmt_clk_div %d, sampling_period_seconds %f, sampling_window_seconds %f, filter_length %d",
		task_inputs->pcnt_gpio,
		task_inputs->pcnt_unit,
		task_inputs->pcnt_channel,
		task_inputs->rmt_gpio,
		task_inputs->rmt_clk_div,
		task_inputs->sampling_period_seconds,
		task_inputs->sampling_window_seconds,
		task_inputs->filter_length);

	init_rmt(task_inputs->rmt_gpio, task_inputs->rmt_channel, task_inputs->rmt_clk_div);
	init_pcnt(task_inputs->pcnt_gpio, task_inputs->rmt_gpio, task_inputs->pcnt_unit, task_inputs->pcnt_channel, task_inputs->filter_length);

	// assuming 80MHz APB clock
	const double rmt_period = (double)(task_inputs->rmt_clk_div) / 80000000.0;

	const size_t items_size = RMT_MEM_BLOCK_BYTE_NUM * task_inputs->rmt_max_blocks;
	rmt_item32_t * rmt_items = malloc(items_size);
	assert(rmt_items);
	memset(rmt_items, 0, items_size);
	int num_rmt_items = create_rmt_window(rmt_items, task_inputs->sampling_window_seconds, rmt_period);
	assert(num_rmt_items <= task_inputs->rmt_max_blocks * RMT_MEM_ITEM_NUM);

	TickType_t last_wake_time = xTaskGetTickCount();
	double frequency_hz;

	while (1)
	{
		// clear counter
		ESP_ERROR_CHECK(pcnt_counter_clear(task_inputs->pcnt_unit));

		// start sampling window
		ESP_ERROR_CHECK(rmt_write_items(task_inputs->rmt_channel, rmt_items, num_rmt_items, false));

		// call wndow-start callback if set
		if (task_inputs->window_start_callback)
		{
			(task_inputs->window_start_callback)();
		}

		// wait for window to finish
		ESP_ERROR_CHECK(rmt_wait_tx_done(task_inputs->rmt_channel, portMAX_DELAY));

		// read counter
		int16_t count = 0;
		ESP_ERROR_CHECK(pcnt_get_counter_value(task_inputs->pcnt_unit, &count));

		// TODO: check for overflow?

		frequency_hz = count / 2.0 / task_inputs->sampling_window_seconds;

		// call the frequency update callback
		if (task_inputs->frequency_update_callback)
		{
			(task_inputs->frequency_update_callback)(frequency_hz);
		}

		ESP_LOGD(TAG, "counter %d, frequency %f Hz", count, frequency_hz);

		int delay_time = task_inputs->sampling_period_seconds * 1000 / portTICK_PERIOD_MS;
		if (delay_time > 0)
		{
			vTaskDelayUntil(&last_wake_time, delay_time);
		}
	}

	free(rmt_items);
	free(task_inputs);  // TODO: avoid this if not dynamically allocated
	vTaskDelete(NULL);
}
*/