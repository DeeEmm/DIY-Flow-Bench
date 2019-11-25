/****************************************
 * controls.h -  HMI functions - user input and menu system
 * This file is part of the DIY Flow Bench project. For more information including usage and licensing please refer to: https://github.com/DeeEmm/DIY-Flow-Bench
 ***/



/****************************************
 * GLOBAL VARIABLE DECLARATIONS
 * We declare them here but they can be over-written in configuration.h
 * This allows for user-defined configurations to be maintained within configuration.h
 ***/


// rotary encoder pin
// click button pin
// bench start pin
// bench stop pin



/****************************************
 * CONFIGURE CONTROLS
 ***/
void configureControls ()
{

}


/****************************************
 * TEST INPUTS
 ***/
bool testInputs ()
{
    // Here we are checking is clickwheel / encoder input / buttons have been operated
    // if true then return inputStatusChanged
    return true;

    //NOTE: for some inputs an interrupt may be more appropriate (encoder?)

}


/****************************************
 * DECODE INPUTS
 ***/
void decodeInputs ()
{

}


/****************************************
 * PARSE CONTROLS
 ***/
void parseControls ()
{

}

/****************************************
 * UPDATE MENU
 ***/
void updateMenu ()
{
    //update menu data based on parsed controls data
}


/****************************************
 * CONFIGURE SENSORS
 ***/
void writeOutputs ()
{

}
