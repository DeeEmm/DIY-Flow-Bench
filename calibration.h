/****************************************
 * calibration.h - perform calibration functions
 * This file is part of the DIY Flow Bench project. For more information including usage and licensing please refer to: https://github.com/DeeEmm/DIY-Flow-Bench
 ***/


// Define Constants (add additional sensors incrementally)
const int SIEMENS_5WK9605 = 1;
// NOTE do not overwrite the following - please leave as an example for future users
const int SOME_OTHER_LINEAR_SENSOR = 99; 
const int SOME_OTHER_NON_LINEAR_SENSOR = 100;


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
float loadMafData (int mafType = 1)
{

    float mafScalingFactor = 1.0; // volts per cfm

    //select MAF Type
    switch(mafType) {

        case SIEMENS_5WK9605 : 
            mafScalingFactor = 1.1; // need to update with actual value for our MAF (assuming it is linear)
        break;

        case SOME_OTHER_LINEAR_SENSOR : // Linear scaling factor for CFM versus VOLTS
            mafScalingFactor = 1.1; 
        break;

        case SOME_OTHER_NON_LINEAR_SENSOR : // Non-Linear relationship between CFM versus VOLTS

            // non-linear so we need to use a lookup table
            // read CFM data from delineated file (data in 0.1v increments - which should give us a 0.5cfm resolution)
            // write data to global array [voltage, cfm] 
            // NOTE - ARRAY NEEDS TO HAVE GLOBAL SCOPE OR THIS WILL NOT WORK !!!!!
            // set scaling factor to zero - this is how we know to use lookup table instead
            mafScalingFactor = 0;
        break;
    }

    return mafScalingFactor;
}
