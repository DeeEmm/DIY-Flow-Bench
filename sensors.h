/****************************************
 * sensors.h -  configure and read sensor data
 * This file is part of the DIY Flow Bench project. For more information including usage and licensing please refer to: https://github.com/DeeEmm/DIY-Flow-Bench
 ***/
 


/****************************************
 * GLOBAL VARIABLE DECLARATIONS
 * We declare them here but they can be over-written in configuration.h
 * This allows for user-defined configurations to be maintained within configuration.h
 ***/

int MAF_PIN = 0;
int REF_PRESSURE_PIN = 11;
int REF_VAC_PIN = 12;
int PITOT_PIN = 13;

int mafValue = 0;
// Pressure initialisation
// Temperature initialisation
// Humidity initialisation
// MAF initialisation


/****************************************
 * READ SENSORS
 ***/
void readSensors() 
{
    //GET SENSOR RAW DATA FROM ASSIGNED PINS
    //read MAF
    //  mafValue = analogRead(MAF_PIN);

}