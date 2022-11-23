/*********************************************************//**
* The DIY Flow Bench project
* https://diyflowbench.com
*
* @file DIY-Flow-Bench.ino
* @brief Define main processes
* @author Mick Percy aka DeeEmm deeemm@deeemm.com
* 
* @remarks Open source flow bench project to measure and display volumetric air flow using an ESP32 and Automotive MAF sensor
* 
* For more information please visit the WIKI on our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
* Or join our support forums: https://github.com/DeeEmm/DIY-Flow-Bench/discussions 
* You can also visit our Facebook community: https://www.facebook.com/groups/diyflowbench/
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
* This program has a number of core libraries that must be available for it to work.
* 
*
**/

#include <Arduino.h>
#include "freertos/semphr.h"
#include "esp_task_wdt.h"

#include "constants.h"
#include "configuration.h"
#include "structs.h"

#include "hardware.h"
#include "sensors.h"
#include "calculations.h"
#include "webserver.h"
#include "messages.h"
#include "API.h"


#include LANGUAGE_FILE

// Initiate Structs
ConfigSettings config;
CalibrationSettings calibration;
DeviceStatus status;
WebsocketData socketData;
FileUploadData fileUploadData;
//mafSensorData mafSensor;
SensorData sensorVal;
Translator translate;

// Initiate Classes
Hardware _hardware;
Webserver _webserver;
Calculations _calculations;
Sensors _sensors;
Messages _message;
API _api;

// Set up semaphore signalling between tasks
SemaphoreHandle_t i2c_task_mutex;
TaskHandle_t bmeTaskHandle = NULL;
TaskHandle_t adcTaskHandle = NULL;
TaskHandle_t sseTaskHandle = NULL;
portMUX_TYPE mmux = portMUX_INITIALIZER_UNLOCKED;

int sensorINT; // reusable INT for truncating to 2 decimal places

 
 

/***********************************************************
 * TASK: Get bench sensor data
 * */	
void TASKgetBenchData( void * parameter ){

  extern struct SensorData sensorVal;
  Calculations _calculations;

  for( ;; ) {
    // Check if semaphore available
    if (millis() > status.adcPollTimer){
      if (xSemaphoreTake(i2c_task_mutex,100)==pdTRUE) {
        status.adcPollTimer = millis() + ADC_SCAN_DELAY_MS; // Only reset timer when task executes
        // Get MAF Value and truncate to 2DP
        sensorVal.FlowMASS = _sensors.getMafValue();
        sensorVal.FlowCFM = _calculations.calculateFlowCFM();
        sensorINT = sensorVal.FlowCFM * 100 + .5; 
        sensorVal.MAF = (double)sensorINT / 100; 
        // Get Ref Pressure Value and truncate to 2DP
        sensorINT = _sensors.getPRefValue() * 100 + .5; 
        sensorVal.PRefKPA = (double)sensorINT / 100; 
        // Get Diffrential Pressure Value and truncate to 2DP
        sensorINT = _sensors.getPDiffValue() * 100 + .5; 
        sensorVal.PDiffKPA = (double)sensorINT / 100; 
        // Get Pitot Pressure Value and truncate to 2DP
        sensorINT = _sensors.getPitotValue() * 100 + .5; 
        sensorVal.PitotKPA = (double)sensorINT / 100; 
        xSemaphoreGive(i2c_task_mutex); // Release semaphore
      }   
    }
  }
    vTaskDelay( 1 );  // mSec delay to prevent Watch Dog Timer (WDT) triggering and yield if required
}



/***********************************************************
 * @brief TASK: Get environental sensor data
 * @param i2c_task_mutex Semaphore handshake with TASKpushData
 * @struct sensorVal global struct containing sensor values
 * @struct status global struct containing system status values
 * */	
void TASKgetEnviroData( void * parameter ){

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
 * @param i2c_task_mutex Semaphore handshake with TASKgetEnviroData
 * @param sensorVal global struct containing sensor values
 * @struct status global struct containing system status values
*/
void TASKpushData( void * parameter ){ 
  for( ;; ) {
    if (millis() > status.statusUpdateRate) {        
      if (xSemaphoreTake(i2c_task_mutex,portMAX_DELAY)==pdTRUE){ // Check if semaphore available
        status.statusUpdateRate = millis() + STATUS_UPDATE_RATE; // Only reset timer when task executes
        #ifdef WEBSERVER_ENABLED
          _webserver.events->send(String(sensorVal.TempDegC).c_str(),(char*)"TEMP", millis()); 
          _webserver.events->send(String(sensorVal.BaroHPA).c_str(),"BARO",millis()); 
          _webserver.events->send(String(sensorVal.RelH).c_str(),"RELH",millis()); 
          _webserver.events->send(String(sensorVal.MAF).c_str(),(char*)"FLOW", millis());
          _webserver.events->send(String(sensorVal.PRefKPA).c_str(),(char*)"PREF", millis());
          _webserver.events->send(String(sensorVal.PDiffKPA).c_str(),(char*)"PDIFF", millis());
          _webserver.events->send(String(sensorVal.PitotKPA).c_str(),(char*)"PITOT", millis());
        #endif
        xSemaphoreGive(i2c_task_mutex); // Release semaphore
      }
    }
    vTaskDelay( 1 ); // mSec delay to prevent Watch Dog Timer (WDT) triggering and yield if required
	}

}



/***********************************************************
 * INITIALISATION
 ***/
void setup(void) {
  
  _message.begin();
  _message.serialPrintf("\r\nDIY Flow Bench \nVersion: %s \nBuild: %s \n", RELEASE, BUILD_NUMBER);  
  _hardware.begin();
  _sensors.begin();
  if (config.api_enabled) _message.serialPrintf("Serial API Enabled \n");
  #ifdef WEBSERVER_ENABLED
  _webserver.begin();
  #endif
  
  
  /* dual core initialisation
  // Confirm default core
  uint8_t defaultCore = xPortGetCoreID();                   // This core (1)
  uint8_t secondaryCore = (defaultCore > 0 ? 0 : 1);        // Free core (0)
  
  _message.serialPrintf("Default core : %d\n", defaultCore);
  _message.serialPrintf("Secondary core : %d\n", secondaryCore);
  
  
  // xSemaphore = xSemaphoreCreateBinary();
  xSemaphore = xSemaphoreCreateMutex();
  
  // NOTE: setup() and loop() are automatically created on default core 
  // Create task on second core for hardware loop (hardware / sensors / etc)
  // xTaskCreatePinnedToCore(mainloop, "HARDWARE CPU", 12288, NULL, 20, NULL, secondaryCore); 
  xTaskCreatePinnedToCore(mainloop, "HARDWARE_CPU", 12288, NULL, 1, &hardwareTaskHandle, secondaryCore); 
  
  // NOTE: We should not need to specifically create webtask to default core
  xTaskCreatePinnedToCore(webloop, "WEBSERVER_CPU", 20480, NULL, 2, &webserverTaskHandle, defaultCore);
  
  // NOTE: to create task on current core 
  // xTaskCreate(webloop, "WEBSERVER_CPU", 20480, NULL, 2, &webserverTaskHandle);
  */

  i2c_task_mutex = xSemaphoreCreateMutex();

  xTaskCreate(TASKgetEnviroData, "BME_TASK", 20480, NULL, 20, &bmeTaskHandle);
  // xTaskCreate(TASKgetBenchData, "ADC_TASK", 20480, NULL, 2, &adcTaskHandle);
  xTaskCreate(TASKpushData, "SSE_TASK", 20480, NULL, 20, &sseTaskHandle);

}






/***********************************************************
 * MAIN LOOP
 * 
 * NOTE: Use non-breaking delays for throttling events
 ***/
void loop () {
  

  // TODO: Fix I2C issues - initially for BME and then also ADC

  // Might also need to look at wifi / I2C memory issue - https://www.esp32.com/viewtopic.php?f=2&t=2178



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


  //mSec delay to prevent Watch Dog Timer (WDT) triggering for empty task
  vTaskDelay( 1 ); 
  
}
