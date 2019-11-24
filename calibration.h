/****************************************
 * calibration.h - perform calibration functions
 * This file is part of the DIY Flow Bench project. For more information including usage and licensing please refer to: https://github.com/DeeEmm/DIY-Flow-Bench
 ***/


/****************************************
 * WRITE FLOW CALIBRATION DATA TO NVM
 ***/
void setFlowCalibrationValue (int currentFlowValue = 0)
{
    // we need to save the calibration data to non-volatile memory on the arduino

    // we can return boolean to indicate that write has been successful (error checking)
}


/****************************************
 * GET FLOW CALIBRATION DATA FROM NVM
 ***/
int getFlowCalibration ()
{
    int flowCalibrationValue = 0;
    //Retrieve calibration data from NVM
    return flowCalibrationValue;
}


/****************************************
 * WRITE LEAK-TEST CALIBRATION DATA TO NVM
 ***/
void setLeakCalibration (int currentVacValue = 0)
{
    //we need to save the calibration data to non-volatile memory on the arduino

    // we can return boolean to indicate that write has been successful (error checking)
}


/****************************************
 * CHECK LEAK-TEST CALIBRATION DATA FROM NVM
 ***/
bool checkLeakCalibration (int currentVacValue = 0, int toleranceCFM = 0)
{    
    int leakCalibrationValue = 0; 

    // get value from NVM
    // (need to add this)
    //leakCalibrationValue = ??????;

    // lower the test value to define test tolerance
    // NOTE, this will essentially determine bench accuracy
    leakCalibrationValue -= toleranceCFM;

    //compare calibration data from NVM
    if (leakCalibrationValue >= currentVacValue )
    {
        return true;
    } else {
        return false;
    }

}


/****************************************
 * GET FLOW MAP DATA FROM EXTERNAL LOOKUP TABLE
 ***/
void loadMAFLookupTable (int mafType = 1)
{
    //Retrieve calibration data from external data file
    //populate array [voltage, cfm]

    //select MAF Type
    switch(mafType) {

        case 1 : //replace with constant to make code easier to read]
            //read data from file (0.1v increments)
            //write to array [voltage, cfm]
            //return array
        break;

        case 2 : //additional MAF data as required
            
        break;

    }

}
