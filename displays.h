/****************************************
 * displays.h   
 * Initalise and communicate with displays
 * This file is part of the DIY Flow Bench project. For more information including usage and licensing please refer to: https://github.com/DeeEmm/DIY-Flow-Bench
 ***/

#include "libraries/TM1637Display.h"

/****************************************
 * GLOBAL VARIABLE DECLARATIONS
 ***/

// Constants
#define LINE_1 1
#define LINE_2 2
#define LINE_3 3
#define LINE_4 4
#define FLOW_CFM 1
#define REF_PRESS 2
#define VAC 3
#define PITOT 4
#define FOURX20CHAR_DISPLAY 1
#define TFT_DISPLAY 2
#define OLED_DISPLAY 3
#define FOURX7SEG_DISPLAY 1

// Common variables
int displayDataCFM = 0;
int displayDataPressure = 0;
int displayDataVacuum = 0;
int displayDataPitot = 0;
int displayDataByValue[] = {displayDataCFM, displayDataPressure, displayDataVacuum, displayDataPitot};

// Display types
int mainDisplayType = FOURX20CHAR_DISPLAY;
int flowDisplayType = FOURX7SEG_DISPLAY;
int refDisplayType = FOURX7SEG_DISPLAY;
int vacDisplayType = FOURX7SEG_DISPLAY;
int pitotDisplayType = FOURX7SEG_DISPLAY;

// Active displays
bool flowDisplayEnabled = false;
bool refDisplayEnabled = false;
bool vacDisplayEnabled = false;
bool pitotDisplayEnabled = false;

// Define 4x20 char display
char display4x20DataLine1[] = "";
char display4x20DataLine2[] = "";
char display4x20DataLine3[] = "";
char display4x20DataLine4[] = "";
char *display4x20DataByLine[] = {display4x20DataLine1, display4x20DataLine2, display4x20DataLine3, display4x20DataLine4};    

// Define 4x7 segment display#1
int FOURX7SEG_DISPLAY1_CLK_PIN = 14;
int FOURX7SEG_DISPLAY1_DIO_PIN = 15;

int FOURX7SEG_DISPLAY2_CLK_PIN = 16;
int FOURX7SEG_DISPLAY2_DIO_PIN = 17;


/****************************************
 * INITIALISE DISPLAYS
 ***/
void initialiseDisplays ()
{

    if (flowDisplayEnabled)
    {
        // Create new class instance of FOURX7SEG_DISPLAY 
        //TM1637Display FOURX7SEG_DISPLAY1 = new TM1637Display ();
    }

    if (refDisplayEnabled)
    {
        // Create new class instance of FOURX7SEG_DISPLAY 
    }

    if (vacDisplayEnabled)
    {
        // Create new class instance of FOURX7SEG_DISPLAY 
    }

    if (pitotDisplayEnabled)
    {
        // Create new class instance of FOURX7SEG_DISPLAY 
    }
}






/****************************************
 * 4x20 MAIN DISPLAY
 ***/
void update4x20CharDisplay ()
{

}


/****************************************
 * TFT MAIN DISPLAY
 ***/
void updateTFTDisplay ()
{

}


/****************************************
 * OLED MAIN DISPLAY
 ***/
void updateOLEDDisplay ()
{

}


/****************************************
 * 4x7 SEGMENT DISPLAY
 ***/
void update4x7SegmentDisplay (int displayValue, int displayNumber = 1)
{
    //select display and update it (default is sensor #1 on PCB)
    //display.showNumberDec(displayValue);
}

/****************************************
 * UPDATE DISPLAYS
 * Check enabled displays then update data
 ***/
void updateDisplays()
{
    //main user interface
    switch(mainDisplayType) {

        case FOURX20CHAR_DISPLAY :
            update4x20CharDisplay();
        break;

        case TFT_DISPLAY :
            updateTFTDisplay();
        break;

        case OLED_DISPLAY :
            updateOLEDDisplay();
        break;

    }


    if (flowDisplayEnabled)
    {
        //send data to display
        //get pin values
        //write output ie - displayDataByValue[FLOW_CFM];
    }

    if (refDisplayEnabled)
    {
        //send data to display
        //get pin values
        //write output ie - displayDataByValue[REF_PRESS];
    }

    if (vacDisplayEnabled)
    {
        //send data to display
        //get pin values
        //write output ie - displayDataByValue[VAC];
    }

    if (pitotDisplayEnabled)
    {
        //send data to display
        //get pin values
        //write output ie - displayDataByValue[PITOT];
    }

}
