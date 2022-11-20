/***********************************************************
* The DIY Flow Bench project
* https://diyflowbench.com
*
* DIY-Flow-Bench.ino - Define main processes
*
* Open source flow bench project to measure and display volumetric air flow using an ESP32 and Automotive MAF sensor
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
* This program has a number of core libraries that must be available for it to work as noted below.
*
*
****/
#include <Arduino.h>
#include "freertos/semphr.h"

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
SemaphoreHandle_t xSemaphore = NULL;
TaskHandle_t hardwareTaskHandle = NULL;
TaskHandle_t webserverTaskHandle = NULL;

int sensorINT;


/***********************************************************
 * WEBSERVER LOOP (Runs on CPU Core 1)
 *
 * Creates server instance 
 * Processes websocket and serial API data
 *
 ***/
 void webloop(void *pvParameter) {
   
  // NOTE: Default core is usually core 1 so lets make sure we are on core 1
  _message.debugPrintf("Webserver process assigned to CPU core %d \n", xPortGetCoreID());
  // Print Stack HWM (high water mark)    
  _message.debugPrintf("Webserver Stack HWM: %d \n", uxTaskGetStackHighWaterMark(NULL));

  _webserver.begin();

  _message.serialPrintf("Webserver initialisation complete \n");

  // Infinite Loop
  for(;;) {

    // Process API comms
    if (config.api_enabled) {        
      if (Serial.available() > 0) {
      status.serialData = Serial.read();
      _api.ParseMessage(status.serialData);
      }                            
    }
   
    // Push data to websocket
    config.refresh_rate = config.refresh_rate < MIN_REFRESH_RATE ? MIN_REFRESH_RATE : config.refresh_rate;
    if ((millis() > status.pollTimer) && status.liveStream) {
    status.pollTimer = millis() + config.refresh_rate; 
    // TODO: We need to make sure that message has been sent
    _webserver.sendWebSocketMessage(_webserver.getDataJSON());
    }
   
    _webserver.webskt->cleanupClients();
    
    xSemaphoreGive(xSemaphore);
   
    //mSec delay to prevent Watch Dog Timer (WDT) triggering and yield if required
    vTaskDelay( 1 ) ; 
   
   }
   
 }
 
 
 


/***********************************************************
 * HARDWARE LOOP (Runs on secondary CPU core 0)
 *
 * Manages hardware configuration and setup
 * Processes sensor data 
 * TODO: Makes sensor data available to webserver via Data Queue
 *
 ***/
void mainloop (void *pvParameter) {
  
  extern struct SensorData sensorVal;
  extern struct DeviceStatus status;

  _hardware.begin();
  

  // Initialise sensors
  _sensors.begin();
  

  // NOTE: Default core is usually core 1
  _message.debugPrintf("Hardware process assigned to CPU core : %d \n", xPortGetCoreID());
  // Print Stack HWM (high water mark)
  _message.debugPrintf("Main Stack HWM: %d \n", uxTaskGetStackHighWaterMark(NULL));
  
  _message.serialPrintf("Hardware initialisation complete \n");
  
  // Infinite Loop
  for(;;) {
  
    if(xSemaphoreTake(xSemaphore, MAX_SEMAPHORE_DELAY)) {
  
      #ifdef ADC_IS_ENABLED    
        
        if (millis() > status.adcPollTimer) { // Non breaking delay for ADC read
          status.adcPollTimer = millis() + ADC_SCAN_DELAY_MS; 
        
          // Get ADC sensor data value and store it in global vars so it can be accessed by webserver
          for (int adcChannel = 0; adcChannel <= 3; adcChannel++) {
            switch (adcChannel) {
            
              case MAF_ADC_CHANNEL:
                sensorVal.MAF = _sensors.getMafValue();
              break;
              
              case PREF_ADC_CHANNEL:
                sensorVal.PRefKPA = _sensors.getPRefValue();
              break;
              
              case PDIFF_ADC_CHANNEL:
                sensorVal.PDiffKPA = _sensors.getPDiffValue();
              break;
              
              case PITOT_ADC_CHANNEL:
                sensorVal.PitotKPA = _sensors.getPitotValue();
              break; 
            }
          }   
        }
      #endif
        
      // Non breaking delay for sensor update
      if (millis() > status.bmePollTimer) {
        status.bmePollTimer = millis() + BME_SCAN_DELAY_MS; 
       
        // Get BME sensor data and store it in global struct so that they can be accessed by webserver   
        // TODO: CHANGE TO QUEUE
        sensorVal.TempDegC = _sensors.getTempValue();
        sensorVal.BaroKPA = _sensors.getBaroValue();
        sensorVal.RelH  = _sensors.getRelHValue();
      }
      
      // mSec delay to prevent Watch Dog Timer (WDT) triggering and yield if required
      vTaskDelay( 1 ); 
      
    }
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

  _webserver.begin();
  
 
  
    
    
    
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

  
}


/***********************************************************
 * MAIN LOOP
 * 
 * NOTE: Use non-breaking delays for throttling events
 ***/
void loop () {
  
  #ifdef ADC_IS_ENABLED    
    
    if (millis() > status.adcPollTimer) { // Non breaking delay for ADC read
      status.adcPollTimer = millis() + ADC_SCAN_DELAY_MS; 
    
      // Get ADC sensor data value and store it in global vars so it can be accessed by webserver
      for (int adcChannel = 0; adcChannel <= 3; adcChannel++) {
        switch (adcChannel) {
        
          case MAF_ADC_CHANNEL:
            sensorINT = _sensors.getMafValue() * 100 + .5; 
            sensorVal.MAF = (double)sensorINT / 100; 
            _webserver.events->send(String(sensorVal.MAF).c_str(),(char*)"FLOW", millis());
          break;
          
          case PREF_ADC_CHANNEL:
            sensorINT = _sensors.getPRefValue() * 100 + .5; 
            sensorVal.PRefKPA = (double)sensorINT / 100; 
            _webserver.events->send(String(sensorVal.PRefKPA).c_str(),(char*)"PREF", millis());
          break;
          
          case PDIFF_ADC_CHANNEL:
            sensorINT = _sensors.getPDiffValue() * 100 + .5; 
            sensorVal.PDiffKPA = (double)sensorINT / 100; 
            _webserver.events->send(String(sensorVal.PDiffKPA).c_str(),(char*)"DIFF", millis());
          break;
          
          case PITOT_ADC_CHANNEL:
            sensorINT = _sensors.getPitotValue() * 100 + .5; 
            sensorVal.PitotKPA = (double)sensorINT / 100; 
            _webserver.events->send(String(sensorVal.PitotKPA).c_str(),(char*)"PITOT", millis());
          break; 
        }
      }   
    }
  #endif
  

  // Non breaking delay for sensor update
  if (millis() > status.bmePollTimer) {
    status.bmePollTimer = millis() + BME_SCAN_DELAY_MS; 
   	  
    // Push Temperature value to browser using SSE (Truncate to 2 DP by casting to int)
    sensorINT = _sensors.getTempValue() * 100 + .5; 
    sensorVal.TempDegC = (double)sensorINT / 100; 
     _webserver.events->send(String(sensorVal.TempDegC).c_str(),(char*)"TEMP", millis());


    // Push Baro to browser using SSE
	  sensorINT = _calculations.convertPressure(_sensors.getBaroValue(), HPA) * 100 + .5;
    sensorVal.BaroKPA = (double)sensorINT / 100;
    _webserver.events->send(String(sensorVal.BaroKPA).c_str(),"BARO",millis());
    
    // Get RelH to browser using SSE
	  sensorINT = _sensors.getRelHValue() * 100 + .5;
    sensorVal.RelH = (double)sensorINT / 100;
    _webserver.events->send(String(sensorVal.RelH).c_str(),"RELH",millis());


  }



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
