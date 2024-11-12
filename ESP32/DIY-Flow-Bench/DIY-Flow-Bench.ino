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
#include <MD_REncoder.h>
#include <math.h>

#include "version.h"
#include "constants.h"
#include "configuration.h"
#include "structs.h"
#include "pins.h"

#include "mafData/maf.h"
#include "hardware.h" // bench type config needed here
#include "sensors.h"
#include "calculations.h"
#include "webserver.h" // config loaded here
#include "messages.h"
#include "API.h"
#include "Wire.h"

#include LANGUAGE_FILE

// Initiate Structs
ConfigSettings config;
DeviceStatus status;
FileUploadData fileUploadData;
SensorData sensorVal;
ValveLiftData valveData;
Translator translate;
CalibrationData calVal;

// Initiate Classes
API _api;
Calculations _calculations;
Hardware _hardware;
Messages _message;
Sensors _sensors;
Webserver _webserver;

// Set up semaphore signalling between tasks
SemaphoreHandle_t i2c_task_mutex;
TaskHandle_t sensorDataTask = NULL;
TaskHandle_t enviroDataTask = NULL;
// portMUX_TYPE mmux = portMUX_INITIALIZER_UNLOCKED;

char charDataJSON[256];
String jsonString;

#ifdef SWIRL_IS_ENABLED
  MD_REncoder Encoder = MD_REncoder(SWIRL_ENCODER_PIN_A, SWIRL_ENCODER_PIN_B);
#endif
/***********************************************************
 * @brief TASK: Get bench sensor data (ADS1115 - MAF/RefP/DiffP/Pitot)
 * @param i2c_task_mutex Semaphore handshake with TASKpushData / TASKgetEnviroData
 * @struct sensorVal global struct containing sensor values
 * @remarks Interrogates ADS1115 ADC and saves sensor values to struct
 * */	
void TASKgetSensorData( void * parameter ){

  extern struct DeviceStatus status;
  extern struct SensorData sensorVal;
  Calculations _calculations;
  int sensorINT;

  for( ;; ) {
    // Check if semaphore available
    if (millis() > status.adcPollTimer){
      // if (xSemaphoreTake(i2c_task_mutex,portMAX_DELAY)==pdTRUE) {
      if (xSemaphoreTake(i2c_task_mutex, 50 / portTICK_PERIOD_MS)==pdTRUE) {
        status.adcPollTimer = millis() + ADC_SCAN_DELAY_MS; // Only reset timer when task executes

        // Get flow data
        // Bench is MAF type...
        if (strstr(String(config.bench_type).c_str(), String("MAF").c_str())){
          #ifdef MAF_IS_ENABLED
          sensorVal.FlowKGH = _sensors.getMafFlow();
          sensorVal.FlowCFMraw = _calculations.convertFlow(sensorVal.FlowKGH);
          #endif

        // Bench is ORIFICE type...
        } else if (strstr(String(config.bench_type).c_str(), String("ORIFICE").c_str())){
          sensorVal.FlowCFMraw = _sensors.getDifferentialFlow();


        // Bench is VENTURI type...
        } else if (strstr(String(config.bench_type).c_str(), String("VENTURI").c_str())){

          //TODO

        // Bench is PITOT type...
        } else if (strstr(String(config.bench_type).c_str(), String("PITOT").c_str())){

          //TODO

        // Error bench type unknown
        } else {

          //Do nothing?

        }

        // Apply Flow calibration and leak offsets
        sensorVal.FlowCFM = sensorVal.FlowCFMraw  - calVal.leak_cal_baseline - calVal.leak_cal_offset - calVal.flow_offset;


 
        // Apply Data filters...

        // Rolling Median
        if (strstr(String(config.data_filter_type).c_str(), String("MEDIAN").c_str())){
 
          sensorVal.AverageCFM += ( sensorVal.FlowCFM - sensorVal.AverageCFM ) * 0.1f; // rough running average.
          sensorVal.MedianCFM += copysign( sensorVal.AverageCFM * 0.01, sensorVal.FlowCFM - sensorVal.MedianCFM );
          sensorVal.FlowCFM = sensorVal.MedianCFM;

        //TODO - Cyclic average
        } else if (strstr(String(config.data_filter_type).c_str(), String("AVERAGE").c_str())){

          // create array / stack of 'Cyclical Average Buffer' length
          // push value to stack - repeat this for size of stack 
          
          // calculate average of values on stack
          // average results with current value - this is our displayed value
          // push current (raw) value on to stack, remove oldest value
          // repeat


          
        //TODO - Mode
        } else if (strstr(String(config.data_filter_type).c_str(), String("MODE").c_str())){

          // return most common value over x number of cycles (requested by @black-top)

        }


        // convert to standard flow
        sensorVal.FlowSCFM = _calculations.convertToSCFM(sensorVal.FlowCFM, config.standardReference);


        // Create Flow differential values
        switch (sensorVal.FDiffType) {

        case USERTARGET:
          sensorVal.FDiff = sensorVal.FlowCFM - calVal.user_offset;
          strcpy(sensorVal.FDiffTypeDesc, "User Target (cfm)");
          break;

        case BASELINE:
          sensorVal.FDiff = sensorVal.FlowCFMraw - calVal.flow_offset - calVal.leak_cal_baseline;
          strcpy(sensorVal.FDiffTypeDesc, "Baseline (cfm)");
          break;
        
        case BASELINE_LEAK :
          sensorVal.FDiff = sensorVal.FlowCFMraw - calVal.flow_offset - calVal.leak_cal_baseline - calVal.leak_cal_offset;
          strcpy(sensorVal.FDiffTypeDesc, "Offset (cfm)");
          break;
                
        default:
          break;
        }
        
        
        #ifdef PREF_IS_ENABLED 
        sensorVal.PRefKPA = _sensors.getPRefValue();
        sensorVal.PRefH2O = _calculations.convertPressure(sensorVal.PRefKPA, INH2O);
        if (config.std_adj_flow == 1) {
          sensorVal.FlowADJ = _calculations.convertFlowDepression(sensorVal.PRefH2O, config.adj_flow_depression, sensorVal.FlowSCFM);
        } else {
          sensorVal.FlowADJ = _calculations.convertFlowDepression(sensorVal.PRefH2O, config.adj_flow_depression, sensorVal.FlowCFM);
        }
        #endif

        #ifdef PDIFF_IS_ENABLED
        sensorVal.PDiffKPA = _sensors.getPDiffValue();
        sensorVal.PDiffH2O = _calculations.convertPressure(sensorVal.PDiffKPA, INH2O);
        #endif

        #ifdef PITOT_IS_ENABLED
        sensorVal.PitotKPA = _sensors.getPitotValue();
        sensorVal.PitotH2O = _calculations.convertPressure(sensorVal.PitotKPA, INH2O);
        #endif

        #ifdef SWIRL_IS_ENABLED
          uint8_t Swirl = Encoder.read();

          if (Swirl == DIR_CW) {
            sensorVal.Swirl = Encoder.speed();
          } else {
            sensorVal.Swirl = Encoder.speed() * -1;
          }

        #endif



        xSemaphoreGive(i2c_task_mutex); // Release semaphore        
      }   
    }
  }
    vTaskDelay( VTASK_DELAY_ADC );  // mSec delay to prevent Watch Dog Timer (WDT) triggering and yield if required
}



/***********************************************************
 * @brief TASK: Get environental sensor data (BME280 - Temp/Baro/RelH)
 * @param i2c_task_mutex Semaphore handshake with TASKpushData / TASKgetSensorData
 * @struct sensorVal global struct containing sensor values
 * @struct status global struct containing system status values
 * @remarks Interrogates BME280 and saves sensor data to struct
 * */	
void TASKgetEnviroData( void * parameter ){

  extern struct DeviceStatus status;
  extern struct SensorData sensorVal;
  Calculations _calculations;

  for( ;; ) {
    if (millis() > status.bmePollTimer){
      // if (xSemaphoreTake(i2c_task_mutex,portMAX_DELAY)==pdTRUE) { // Check if semaphore available
      if (xSemaphoreTake(i2c_task_mutex, 50 / portTICK_PERIOD_MS)==pdTRUE) { // Check if semaphore available
        status.bmePollTimer = millis() + BME_SCAN_DELAY_MS; // Only reset timer when task executes
        
        sensorVal.TempDegC = _sensors.getTempValue();
        sensorVal.TempDegF = _calculations.convertTemperature(_sensors.getTempValue(), DEGF);
        sensorVal.BaroHPA = _sensors.getBaroValue();
        sensorVal.BaroPA = sensorVal.BaroHPA * 100.00F;
        sensorVal.BaroKPA = sensorVal.BaroPA * 0.001F;
        sensorVal.RelH = _sensors.getRelHValue();
        xSemaphoreGive(i2c_task_mutex); // Release semaphore
      }
    }
    vTaskDelay( VTASK_DELAY_BME ); // mSec delay to prevent Watch Dog Timer (WDT) triggering and yield if required
	}
}





/***********************************************************
 * @brief Default setup function
 * @details Initialises system and sets up core tasks
 * @note We can assign tasks to specific cores if required (currently disabled)
 ***/
void setup(void) {

  // REVIEW
  // set message queue length
  xQueueCreate( 256, 2048);
  
  // We need to call Wire globally so that it is available to both hardware and sensor classes so lets do that here
  Wire.begin (SDA_PIN, SCL_PIN); 
  Wire.setClock(100000);
  // Wire.setClock(300000); // ok for wemos D1
  // Wire.setClock(400000);
    
  _hardware.begin();
  _sensors.begin();

  // Confirm default core - NOTE: setup() and loop() are automatically created on default core 
  uint8_t defaultCore = xPortGetCoreID();                   // This core (1)
  uint8_t secondaryCore = (defaultCore > 0 ? 0 : 1);        // Secondary core (0)

  // Set up semaphore handshaking
  i2c_task_mutex = xSemaphoreCreateMutex();

  #ifdef WEBSERVER_ENABLED // Compile time directive used for testing
  _webserver.begin();
  #endif

  #ifdef ADC_IS_ENABLED // Compile time directive used for testing
  xTaskCreatePinnedToCore(TASKgetSensorData, "GET_SENSOR_DATA", SENSOR_TASK_MEM_STACK, NULL, 2, &sensorDataTask, secondaryCore); 
  #endif

  #ifdef BME_IS_ENABLED // Compile time directive used for testing
  xTaskCreatePinnedToCore(TASKgetEnviroData, "GET_ENVIRO_DATA", ENVIRO_TASK_MEM_STACK, NULL, 2, &enviroDataTask, secondaryCore); 
  #endif

  #ifdef SWIRL_IS_ENABLED
  Encoder.begin();
  #endif

}




/***********************************************************
 * @brief MAIN LOOP
 * @details processes API requests 
 * @details pushes data to clients via SSE
 * 
 * @note Use non-breaking delays for throttling events
 ***/
void loop () {
  

  // Process API comms
  if (config.api_enabled) {        
    if (millis() > status.apiPollTimer) {
      if (xSemaphoreTake(i2c_task_mutex, 50 / portTICK_PERIOD_MS)==pdTRUE){ // Check if semaphore available
        status.apiPollTimer = millis() + API_SCAN_DELAY_MS; 

        if (Serial.available() > 0) {
          status.serialData = Serial.read();
          _api.ParseMessage(status.serialData);
        }
        xSemaphoreGive(i2c_task_mutex); // Release semaphore
      }
    }                            
  }
  
  // TODO: PID Vac source Analog VFD control [if PREF not within limits]
  // _hardware.setVFDRef();
  // _hardware.setBleedValveRef();
  
  #ifdef WEBSERVER_ENABLED
  if (millis() > status.browserUpdateTimer) {        
      // if (xSemaphoreTake(i2c_task_mutex,portMAX_DELAY)==pdTRUE){ // Check if semaphore available
      if (xSemaphoreTake(i2c_task_mutex, 50 / portTICK_PERIOD_MS)==pdTRUE){ // Check if semaphore available
        status.browserUpdateTimer = millis() + STATUS_UPDATE_RATE; // Only reset timer when task executes
        
        // Push data to client using Server Side Events (SSE)
        jsonString = _webserver.getDataJSON();
        _webserver.events->send(String(jsonString).c_str(),"JSON_DATA",millis()); // Is String causing message queue issue?

        xSemaphoreGive(i2c_task_mutex); // Release semaphore
      }
  }
  #endif

  if(status.shouldReboot){
    _message.serialPrintf("Rebooting...");
    delay(100);
    ESP.restart();
  }

  vTaskDelay( 1 );  //mSec delay to prevent Watch Dog Timer (WDT) triggering for empty task
  
}
