


/****************************************
 * Process Request
 *
 * TODO - Adapt old TCmenu data to work with websockets / HTML5
 * 
 * NOTE: displayed values need to be multiplied by divisor setting in TC menu to display decimal points
 ***/
void processRequest()
{

    float mafFlowCFM = getMafFlowCFM();
    float refPressure = getRefPressure(INWG);   

    // Main Menu
    // Flow Rate
    if (mafFlowCFM > MIN_FLOW_RATE)
    {
 //       menuFlow.setCurrentValue(mafFlowCFM * 10);   
    } else {
 //       menuFlow.setCurrentValue(0);   
    }

    // Temperature
 //   menuTemp.setCurrentValue(getTemp(DEGC) * 10);   
    
    // Baro
 //   menuBaro.setCurrentValue(getBaroPressure(KPA) * 10);   
    
    // Relative Humidity
 //   menuRelH.setCurrentValue(getRelativeHumidity(PERCENT) * 10);   

    // Pitot
    double pitotPressure = getPitotPressure(INWG);
    // Pitot probe displays as a percentage of the reference pressure
    double pitotPercentage = ((getPitotPressure(INWG) / refPressure) * 10);
 //   menuPitot.setCurrentValue(pitotPercentage);  
    
    // Reference pressure
 //   menuPRef.setCurrentValue(refPressure * 10);   
    
    // Adjusted Flow
    // get the desired bench test pressure
//    double desiredRefPressureInWg = menuARef.getCurrentValue();
    // convert from the existing bench test
//    double adjustedFlow = convertFlowDepression(refPressure, desiredRefPressureInWg, mafFlowCFM);
    // Send it to the display
 //   menuAFlow.setCurrentValue(adjustedFlow * 10); 

    // Version and build
    String versionNumberString = String(MAJOR_VERSION) + '.' + String(MINOR_VERSION);
    String buildNumberString = String(BUILD_NUMBER);
//    menuVer.setTextValue(versionNumberString.c_str());
//    menuBld.setTextValue(buildNumberString.c_str());    

    // Ref Presure Voltage
    int refPressRaw = analogRead(REF_PRESSURE_PIN);
    double refMillivolts = (refPressRaw * (5.0 / 1024.0)) * 1000;
//    menuSensorTestPRef.setCurrentValue(refMillivolts);

    // Maf Voltage
    int mafFlowRaw = analogRead(MAF_PIN);
    double mafMillivolts = (mafFlowRaw * (5.0 / 1024.0)) * 1000;
//    menuSensorTestMAF.setCurrentValue(mafMillivolts);

    // Pitot Voltage
    int pitotRaw = analogRead(PITOT_PIN);
    double pitotMillivolts = (pitotRaw * (5.0 / 1024.0)) * 1000;
//    menuSensorTestPitot.setCurrentValue(pitotMillivolts);

}





/****************************************
 * CALLBACK FUNCTION
 * menuCallback_Calibrate
 ***/
void menuCallback_Calibrate() {

    // Perform calibration
    float flowCalibrationOffset = setCalibrationOffset();

    char flowCalibrationOffsetText[12]; // Buffer big enough?
    dtostrf(flowCalibrationOffset, 6, 2, flowCalibrationOffsetText); // Leave room for too large numbers!

    // Display the value on the main screen
    displayDialog(LANG_CAL_OFFET_VALUE, flowCalibrationOffsetText);
}



/****************************************
 * CALLBACK FUNCTION
 * menuCallback_leakTestCalibration
 ***/
void menuCallback_leakTestCalibration() {

    float calibrationValue = leakTestCalibration();


    char calibrationValueText[12]; // Buffer big enough?
    dtostrf(calibrationValue, 6, 2, calibrationValueText); // Leave room for too large numbers!

    // Display the value on popup on the main screen
    displayDialog(LANG_LEAK_CAL_VALUE, calibrationValueText);  
    sendSerial(String(calibrationValue)); 
}



/****************************************
 * CALLBACK FUNCTION
 * menuCallback_LeakTest
 ***/
void menuCallback_LeakTest() {

    int testResult = leakTest();

    //compare calibration data from NVM
    if (testResult == LEAK_TEST_PASS)
    {
       // Display result on the main screen
       errorVal = LEAK_TEST_PASS;      
    } else {
       // Display result on the main screen 
       errorVal = LEAK_TEST_FAILED;       
    }

}



