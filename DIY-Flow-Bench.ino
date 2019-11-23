/****************************************
 * The DIY Flow Bench project
 * A basic flow bench to measure and display volumetric air flow using an Arduino and common automotive MAF sensor.
 * 
 * For more information please visit our GitHub project page: https://github.com/DeeEmm/DIY-Flow-Bench/wiki
 * Or join our Facebook community: https://www.facebook.com/groups/diyflowbench/
 * 
 * This project and all associated files are provided for use under the GNU GPL3 license:
 * https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/LICENSE
 * 
 * Version 1.0.0-alpha
 */

 /****************************************
 * DEV NOTES
 * 
 * Need to consider & organise variable scope.
 * Need to determine function scopes 
 *  
 ***/


#include <Arduino.h>
#include <TM1637Display.h> // 4 char 7 seg display library

#include "configuration.ino"
#include "displays.h"
#include "sensors.h"
#include "controls.h"
#include "calculations.h"
#include "calibration.h"


/****************************************
 * INITIALISATION
 ***/
void setup ()
{
    initaliseDisplays();
}

/****************************************
 * MAIN PROGRAM LOOP
 ***/
void loop ()
{

    readSensors ();
    parseControls();
    calculateMafFlow();
    calculateSensor1Pressure();
    calculateSensor2Pressure();
    updateDisplays();
    writeOutputs();

}
