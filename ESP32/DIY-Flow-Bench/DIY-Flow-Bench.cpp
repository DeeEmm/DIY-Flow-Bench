/***********************************************************
 * @name The DIY Flow Bench project
 * @details Measure and display volumetric air flow using an ESP32 & Automotive MAF sensor
 * @link https://diyflowbench.com
 * @author DeeEmm aka Mick Percy deeemm@deeemm.com
 * 
 * @file DIY-Flow-Bench.ino
 * 
 * @brief define variables and configure hardware
 * 
 * @remarks For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions
 * You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * @license This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 * Except where noted this project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 * The standard project board is the ESP32DUINO used in conjunction with the DIY-Flow-Bnch shield
 * Other ESP32 based boards and custom shields can be made to work. 
 * You can define custom pin definitions to suit your project in pins.h
 * 
 * Default Temperature / Barometric Pressure / Relative Humidity uses a BME280 device connected via I2C
 * The generic I2C address for the BME280 is 0x77 / 0x76 (NOTE: BME NOT BMP!!)
 *
 * Default MAF / Reference Pressure / Pitot / Differential Pressure sensors use ADS1115 (recommended) or ADS1015 ADCs via I2C
 * The generic I2C address for the ADS is 0x48 but can also be set to 0x49 / 0x4A / 0x4B by pin linking at the device
 *
 * Default MAF unit recommended is the GM LS2 MAF (ACDELCO_ 92281162.h) This will measure up to approx 277cfm
 * Other MAF sensors are supported by creation of MAF Data file - See mafDATA/ for examples
 * Note that currently only MAF style benches are supported. D/P Orifice and Pitot support to come.
 *
 * DEPENDENCIES
 * This program has a number of core libraries that must be available for it to work. Please refer to platformio.ini
 * 
 *
 **/
#include <Arduino.h>
#include "freertos/semphr.h"
#include "esp_task_wdt.h"

#include "constants.h"
#include "configuration.h"
#include "structs.h"
#include "pins.h"

#include "mafData/maf.h"
#include "hardware.h"
#include "sensors.h"
#include "calculations.h"
#include "webserver.h"
#include "messages.h"
#include "API.h"
#include "Wire.h"

#include LANGUAGE_FILE

// Initiate Structs
ConfigSettings config;
CalibrationSettings calibration;
DeviceStatus status;
FileUploadData fileUploadData;
SensorData sensorVal;
Translator translate;

// Initiate Classes
API _api;
Calculations _calculations;
Hardware _hardware;
Messages _message;
Sensors _sensors;
Webserver _webserver;

// Set up semaphore signalling between tasks
SemaphoreHandle_t i2c_task_mutex;
TaskHandle_t bmeTaskHandle = NULL;
TaskHandle_t adcTaskHandle = NULL;
TaskHandle_t sseTaskHandle = NULL;
portMUX_TYPE mmux = portMUX_INITIALIZER_UNLOCKED;




/***********************************************************
 * @brief TASK: Get bench sensor data
 * @param i2c_task_mutex Semaphore handshake with TASKpushData / TASKgetEnviroData
 * @struct sensorVal global struct containing sensor values
 * */	
void TASKgetBenchData( void * parameter ){

  extern struct DeviceStatus status;
  extern struct SensorData sensorVal;
  Calculations _calculations;
  int sensorINT;

  for( ;; ) {
    // Check if semaphore available
    if (millis() > status.adcPollTimer){
      if (xSemaphoreTake(i2c_task_mutex,portMAX_DELAY)==pdTRUE) {
        status.adcPollTimer = millis() + ADC_SCAN_DELAY_MS; // Only reset timer when task executes

        #ifdef MAF_IS_ENABLED
        // Get MAF Value and truncate to 2DP
        sensorVal.MAF = _sensors.getMafRaw();
        sensorVal.FlowCFM = _calculations.calculateFlowCFM(sensorVal.MAF);
        #endif
        
        #ifdef PREF_IS_ENABLED 
        // Get Ref Pressure Value and truncate to 2DP by casting to int
        // sensorINT = int(_sensors.getPRefValue() * 100 + .5); 
        // sensorVal.PRefKPA = (double)sensorINT / 100; 
        sensorVal.PRefKPA = _sensors.getPRefValue();
        #endif

        #ifdef PDIFF_IS_ENABLED
        // Get Differential Pressure Value and truncate to 2DP
        // sensorINT = _sensors.getPDiffValue() * 100 + .5; 
        // sensorVal.PDiffKPA = (double)sensorINT / 100; 
        sensorVal.PDiffKPA = _sensors.getPDiffValue();
        #endif

        #ifdef PITOT_IS_ENABLED
        // Get Pitot Pressure Value and truncate to 2DP
        // sensorINT = _sensors.getPitotValue() * 100 + .5; 
        // sensorVal.PitotKPA = (double)sensorINT / 100; 
        sensorVal.PitotKPA = _sensors.getPitotValue();
        #endif
        xSemaphoreGive(i2c_task_mutex); // Release semaphore        
      }   
    }
  }
    vTaskDelay( 1 );  // mSec delay to prevent Watch Dog Timer (WDT) triggering and yield if required
}



/***********************************************************
 * @brief TASK: Get environental sensor data
 * @param i2c_task_mutex Semaphore handshake with TASKpushData / TASKgetBenchData
 * @struct sensorVal global struct containing sensor values
 * @struct status global struct containing system status values
 * */	
void TASKgetEnviroData( void * parameter ){

  extern struct DeviceStatus status;
  extern struct SensorData sensorVal;

  for( ;; ) {
    if (millis() > status.bmePollTimer){
      if (xSemaphoreTake(i2c_task_mutex,portMAX_DELAY)==pdTRUE) { // Check if semaphore available
        status.bmePollTimer = millis() + BME_SCAN_DELAY_MS; // Only reset timer when task executes
        
        sensorVal.TempDegC = _sensors.getTempValue();
        sensorVal.BaroHPA = _sensors.getBaroValue();
        sensorVal.BaroKPA = sensorVal.BaroHPA / 10;
        sensorVal.RelH = _sensors.getRelHValue();
        xSemaphoreGive(i2c_task_mutex); // Release semaphore
      }
    }
    vTaskDelay( 1 ); // mSec delay to prevent Watch Dog Timer (WDT) triggering and yield if required
	}
}



/***********************************************************
 * @brief Push data to browser using Server Side Events (SSE)
 * @param i2c_task_mutex Semaphore handshake with TASKgetEnviroData / TASKgetBenchData
 * @param sensorVal global struct containing sensor values
 * @struct status global struct containing system status values
*/
void TASKpushSseData( void * parameter ){ 

  extern struct DeviceStatus status;

  for( ;; ) {
    if (millis() > status.statusUpdateRate) {        
      if (xSemaphoreTake(i2c_task_mutex,portMAX_DELAY)==pdTRUE){ // Check if semaphore available
        status.statusUpdateRate = millis() + STATUS_UPDATE_RATE; // Only reset timer when task executes
        
        #ifdef WEBSERVER_ENABLED
          #ifdef MAF_STYLE_BENCH
          _webserver.events->send(String(sensorVal.FlowCFM).c_str(),(char*)"FLOW", millis());
          #else
          _webserver.events->send(String(sensorVal.PDiffKPA).c_str(),(char*)"PDIFF", millis());
          #endif
          _webserver.events->send(String(sensorVal.PRefKPA).c_str(),(char*)"PREF", millis());
          _webserver.events->send(String(sensorVal.PitotKPA).c_str(),(char*)"PITOT", millis());
          _webserver.events->send(String(sensorVal.TempDegC).c_str(),(char*)"TEMP", millis()); 
          _webserver.events->send(String(sensorVal.BaroHPA).c_str(),"BARO",millis()); 
          _webserver.events->send(String(sensorVal.RelH).c_str(),"RELH",millis());         
        #endif
        xSemaphoreGive(i2c_task_mutex); // Release semaphore
      }
    }
    vTaskDelay( 1 ); // mSec delay to prevent Watch Dog Timer (WDT) triggering and yield if required
	}

}



/***********************************************************
 * @brief Default setup function
 * @details Initialises system and sets up core tasks
 * @note We can assign tasks to specific cores if required (currently disabled)
 ***/
void setup(void) {
  
  _message.serialPrintf("\r\nDIY Flow Bench \nVersion: %s \nBuild: %s \n", RELEASE, BUILD_NUMBER);    

  // We need to call Wire globally so that it is available to both hardware and sensor classes so lets do that here
  Wire.begin (SCA_PIN, SCL_PIN); 
  Wire.setClock(400000);

  _hardware.begin();
  _sensors.begin();

  if (config.api_enabled) _message.serialPrintf("Serial API Enabled \n");

  // Confirm default core - NOTE: setup() and loop() are automatically created on default core 
  uint8_t defaultCore = xPortGetCoreID();                   // This core (1)
  uint8_t secondaryCore = (defaultCore > 0 ? 0 : 1);        // Free core (0)

  // Set up semaphore handshaking
  i2c_task_mutex = xSemaphoreCreateMutex();

  #ifdef WEBSERVER_ENABLED // Compile time directive used for testing
  _webserver.begin();
  xTaskCreate(TASKpushSseData, "PUSH_SSE_DATA", 1200, NULL, 2, &sseTaskHandle); 
  // xTaskCreatePinnedToCore(TASKpushSseData, "PUSH_SSE_DATA", 1200, NULL, 2, &sseTaskHandle, defaultCore);  // Assign to default core
  #endif

  #ifdef ADC_IS_ENABLED // Compile time directive used for testing
  xTaskCreate(TASKgetBenchData, "GET_BENCH_DATA", 2800, NULL, 2, &adcTaskHandle); 
  // xTaskCreatePinnedToCore(TASKgetBenchData, "GET_BENCH_DATA", 1200, NULL, 2, &adcTaskHandle, secondaryCore);  // Assign to secondary core
  #endif

  #ifdef BME_IS_ENABLED // Compile time directive used for testing
  xTaskCreate(TASKgetEnviroData, "GET_ENVIRO_DATA", 2400, NULL, 2, &bmeTaskHandle); 
  // xTaskCreatePinnedToCore(TASKgetEnviroData, "GET_ENVIRO_DATA", 2400, NULL, 2, &bmeTaskHandle, secondaryCore); // Assign to secondary core
  #endif
  
}




/***********************************************************
 * @brief MAIN LOOP
 * @details Process non-critical tasks here 
 * 
 * NOTE: Use non-breaking delays for throttling events
 ***/
void loop () {
  

  // Process API comms
  if (config.api_enabled) {        
      if (millis() > status.apiPollTimer) {
        status.apiPollTimer = millis() + API_SCAN_DELAY_MS; 
        if (Serial.available() > 0) {
          status.serialData = Serial.read();
          _api.ParseMessage(status.serialData);
        }
    }                            
  }
  
  // TODO: PID Vac source Analog VFD control [if PREF not within limits]

  
  vTaskDelay( 1 );  //mSec delay to prevent Watch Dog Timer (WDT) triggering for empty task
  
}
