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
// #include <MD_REncoder.h>
#include <math.h>
#include <Preferences.h>

#include "datahandler.h"
#include "constants.h"
#include "system.h"
#include "structs.h"
#include "mafdata.h"

#include "hardware.h" 
#include "sensors.h"
#include "calculations.h"
#include "webserver.h"
#include "publichtml.h" 
#include "messages.h"
#include "API.h"
#include "Wire.h"


// Initiate Structs
BenchSettings settings;
DeviceStatus status;
SensorData sensorVal;
ValveLiftData valveData;
Language language;
CalibrationData calVal;
Configuration config;
Pins pins;

// Initiate Classes
Preferences _prefs;
DataHandler _data;
API _api;
Calculations _calculations;
Hardware _hardware;
Messages _message;
Sensors _sensors;
Webserver _webserver;
PublicHTML _public_html;

// Set up semaphore signalling between tasks
SemaphoreHandle_t i2c_task_mutex;
TaskHandle_t sensorDataTask = NULL;
TaskHandle_t enviroDataTask = NULL;
// portMUX_TYPE mmux = portMUX_INITIALIZER_UNLOCKED;

char charDataJSON[256];
String jsonString;

// set up task timers to measure task frequency
int adcStartTime =  micros();
int bmeStartTime =  micros();



/***********************************************************
 * @brief TASK: Get bench sensor data (ADS1115 - MAF/RefP/DiffP/Pitot)
 * @param i2c_task_mutex Semaphore handshake with TASKpushData / TASKgetEnviroData
 * @struct sensorVal global struct containing sensor values
 * @remarks Interrogates ADS1115 ADC and saves sensor values to struct
 * */	
void TASKgetSensorData( void * parameter ){

  extern struct DeviceStatus status;
  extern struct SensorData sensorVal;
  extern struct CalibrationData calVal;
  extern struct BenchSettings settings;
  extern struct Configuration config;
  
  Sensors _sensors;
  Hardware _hardware;

  int sensorINT;

  for( ;; ) {
    // Check if semaphore available
    if (millis() > status.adcPollTimer){

      if (xSemaphoreTake(i2c_task_mutex, 50 / portTICK_PERIOD_MS)==pdTRUE) {
      // if (xSemaphoreTake(i2c_task_mutex,portMAX_DELAY)==pdTRUE) {
        status.adcScanTime = (micros() - adcStartTime); // how long since we started the timer? 
        status.adcPollTimer = millis() + config.iADC_SCAN_DLY; // Only reset timer when task executes

        sensorVal.VCC_5V_BUS = _hardware.get5vSupplyVolts();
        sensorVal.VCC_3V3_BUS = _hardware.get3v3SupplyVolts();

        switch (settings.bench_type){

          case MAF_BENCH:
            if (config.iMAF_SRC_TYP != SENSOR_DISABLED) {
              sensorVal.FlowKGH = _sensors.getMafFlow();
              sensorVal.FlowCFMraw = _calculations.convertFlow(sensorVal.FlowKGH);
            }
          break;

          case ORIFICE_BENCH:
            sensorVal.FlowCFMraw = _sensors.getDifferentialFlow();
          break;

          case VENTURI_BENCH:
            //TODO
          break;

          case PITOT_BENCH:
            //TODO
          break;

          default:
            // Error bench type unknown
            _message.debugPrintf("Unknown Bench Type\n");
          break;

        }

        // Apply Flow calibration and leak offsets
        sensorVal.FlowCFM = sensorVal.FlowCFMraw  - calVal.leak_cal_baseline - calVal.leak_cal_offset  - calVal.flow_offset;

        // Apply Data filters...
        switch (settings.data_filter_type) {

          case MEDIAN:
            // Rolling Median      
            sensorVal.AverageCFM += ( sensorVal.FlowCFM - sensorVal.AverageCFM ) * 0.1f; // rough running average.
            sensorVal.MedianCFM += copysign( sensorVal.AverageCFM * 0.01, sensorVal.FlowCFM - sensorVal.MedianCFM );
            sensorVal.FlowCFM = sensorVal.MedianCFM;
          break;

          case AVERAGE:
            //TODO - Cyclic average
            // create array / stack of 'Cyclical Average Buffer' length
            // push value to stack - repeat this for size of stack 
            
            // calculate average of values on stack
            // average results with current value - this is our displayed value
            // push current (raw) value on to stack, remove oldest value
            // repeat
            sensorVal.FlowCFM = sensorVal.FlowCFM;
          break;

          case MODE:
            //TODO - Mode
            // return most common value over x number of cycles (requested by @black-top)
            sensorVal.FlowCFM = sensorVal.FlowCFM;
          break;

          case NONE:
          default:
            // No filter
            sensorVal.FlowCFM = sensorVal.FlowCFM;
          break;

        }

        // convert to standard flow
        sensorVal.FlowSCFM = _calculations.convertToSCFM(sensorVal.FlowCFM, settings.standardReference);

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
          
        if (config.iPREF_SENS_TYP != SENSOR_DISABLED) {
          sensorVal.PRefKPA = _sensors.getPRefValue();
          sensorVal.PRefH2O = _calculations.convertPressure(sensorVal.PRefKPA, INH2O);
          if (settings.std_adj_flow == 1) {
            sensorVal.FlowADJ = _calculations.convertFlowDepression(sensorVal.PRefH2O, settings.adj_flow_depression, sensorVal.FlowSCFM);
          } else {
            sensorVal.FlowADJ = _calculations.convertFlowDepression(sensorVal.PRefH2O, settings.adj_flow_depression, sensorVal.FlowCFM);
          }
          sensorVal.FlowADJSCFM = _calculations.convertToSCFM(sensorVal.FlowADJ, settings.standardReference );
        } else {
          sensorVal.PRefKPA = 0.0f;
          sensorVal.PRefH2O = 0.0f;         
        }

        if (config.iPDIFF_SENS_TYP != SENSOR_DISABLED) {
          sensorVal.PDiffKPA = _sensors.getPDiffValue();
          sensorVal.PDiffH2O = _calculations.convertPressure(sensorVal.PDiffKPA, INH2O) - calVal.pdiff_cal_offset;
        } else {
          sensorVal.PDiffKPA = 0.0f;
          sensorVal.PDiffH2O = 0.0f;
        }

        if (config.iPITOT_SENS_TYP != SENSOR_DISABLED) {
          sensorVal.PitotKPA = _sensors.getPitotValue() - calVal.pitot_cal_offset;
          sensorVal.PitotH2O = _calculations.convertPressure(sensorVal.PitotKPA, INH2O) ;
          sensorVal.PitotVelocity = _sensors.getPitotVelocity();
        } else {
          sensorVal.PitotKPA = 0.0f;
          sensorVal.PitotH2O = 0.0f;
          sensorVal.PitotVelocity = 0.0f;
        }

        if (config.bSWIRL_ENBLD) {
          // TODO #227
            // uint8_t Swirl = Encoder.read();

            // if (Swirl == DIR_CW) {
            //   sensorVal.Swirl = Encoder.speed();
            // } else {
            //   sensorVal.Swirl = Encoder.speed() * -1;
            // }
        } else {
          sensorVal.Swirl = 0;
        }

        xSemaphoreGive(i2c_task_mutex); // Release semaphore        
        adcStartTime = micros(); // start the timer as we leave task
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
  extern struct Configuration config;

  Calculations _calculations;

  for( ;; ) {
    if (millis() > status.bmePollTimer){
      // if (xSemaphoreTake(i2c_task_mutex,portMAX_DELAY)==pdTRUE) { // Check if semaphore available
      if (xSemaphoreTake(i2c_task_mutex, 50 / portTICK_PERIOD_MS)==pdTRUE) { // Check if semaphore available
        status.bmeScanTime = (micros() - bmeStartTime); // how long since we started the timer? 
        status.bmePollTimer = millis() + config.iBME_SCAN_MS; // Only reset timer when task executes
        
        sensorVal.TempDegC = _sensors.getTempValue();

        // TEST BME commissioning
        // _message.debugPrintf("BME680 refTempDegC_INT: %d",sensorVal.test );
        // REMEMBER NO BREAKING TASKS WITHIN INTTERUPT ROUTINES (LIKE SERIAL PRINT)
        
        sensorVal.TempDegF = _calculations.convertTemperature(_sensors.getTempValue(), DEGF);
        sensorVal.BaroHPA = _sensors.getBaroValue();
        sensorVal.BaroPA = sensorVal.BaroHPA * 100.00F;
        sensorVal.BaroKPA = sensorVal.BaroPA * 0.001F;
        sensorVal.RelH = _sensors.getRelHValue();
        sensorVal.PitotVelocity = _sensors.getPitotVelocity();
        sensorVal.PitotDelta = _calculations.convertPressure(_sensors.getPitotValue(),KPA, INH2O);
        xSemaphoreGive(i2c_task_mutex); // Release semaphore
        bmeStartTime = micros(); // start the timer as we leave task
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

  extern struct Pins pins;
  extern struct Configuration config;

  #ifdef VERBOSE
    settings.verbose_print_mode = true; 
  #endif

  // REVIEW
  // set message queue length
  // xQueueCreate( 8, 1024);
  // xQueueCreate( 256, 2048);
  // xQueueCreate( 1024, 4096);
    
  // Initialise Data environment
  _data.begin();
  
  // Initialise Hardware
  _hardware.begin();

  // Initialise sensors
  _sensors.begin();

  // Confirm default core - NOTE: setup() and loop() are automatically created on default core 
  uint8_t defaultCore = xPortGetCoreID();                   // This core (1)
  uint8_t secondaryCore = (defaultCore > 0 ? 0 : 1);        // Secondary core (0)

  // Set up semaphore handshaking
  i2c_task_mutex = xSemaphoreCreateMutex();

  #ifdef WEBSERVER_ENABLED  
    _webserver.begin();
  #endif

  // xTaskCreatePinnedToCore(TASKgetSensorData, "GET_SENS_DATA", SENSOR_TASK_MEM_STACK, NULL, 2, &sensorDataTask, secondaryCore); 
  xTaskCreate(TASKgetSensorData, "GET_SENS_DATA", SENSOR_TASK_MEM_STACK, NULL, 2, &sensorDataTask); 

  // xTaskCreatePinnedToCore(TASKgetEnviroData, "GET_ENVIRO_DATA", ENVIRO_TASK_MEM_STACK, NULL, 2, &enviroDataTask, secondaryCore); 
  xTaskCreate(TASKgetEnviroData, "GET_ENVIRO_DATA", ENVIRO_TASK_MEM_STACK, NULL, 2, &enviroDataTask); 

  if (config.bSWIRL_ENBLD){
    // TODO #227
    // MD_REncoder Encoder = MD_REncoder(SWIRL_ENCODER_A, SWIRL_ENCODER_B);
  }

  // Report free stack and heap to serial monitor
  _message.serialPrintf("Stack Free Memory: EnviroTask=%s / SensorTask=%s \n", _calculations.byteDecode(uxTaskGetStackHighWaterMark(enviroDataTask)), _calculations.byteDecode(uxTaskGetStackHighWaterMark(sensorDataTask))); 
  _message.serialPrintf("Free Heap=%s / Max Allocated Heap=%s \n", _calculations.byteDecode(ESP.getFreeHeap()), _calculations.byteDecode(ESP.getMaxAllocHeap())); 

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
  if (settings.api_enabled) {        
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
          
          // Build Server Side Events (SSE) data
          switch (status.GUIpage) {
            case INDEX_PAGE:{
              jsonString = _data.buildIndexSSEJsonData();
              break;
            }
            case MIMIC_PAGE:{
              jsonString = _data.buildMimicSSEJsonData();
              break;
            }
          }
          // Push SSE data to client
          _webserver.events->send(String(jsonString).c_str(),"JSON_DATA",millis()); 
 
          // Release semaphore
          xSemaphoreGive(i2c_task_mutex); 
        }
    }
  #endif

  if (status.shouldReboot) {
    _message.serialPrintf("Rebooting...");
    delay(100);
    ESP.restart();
  }

  vTaskDelay( 1 );  //mSec delay to prevent Watch Dog Timer (WDT) triggering for empty task
  
}
