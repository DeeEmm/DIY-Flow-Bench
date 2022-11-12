/***********************************************************
 * Language Constants
 *
 * NOTE: Language constants declared in constants.h
 * NOTE: Language Values need to be updated in both places
 *
 * TODO:(#12) Translations
 ***/
#ifndef LANGDATA
#define LANGDATA


struct translator {
    char* LANG_VAL_NULL = "NULL";
    char* LANG_VAL_NO_ERROR = "Status OK";
    char* LANG_VAL_BENCH_RUNNING = "Bench Pressure OK";
    char* LANG_VAL_WEBSERVER_RUNNING = "Webserver Running";
    char* LANG_VAL_WARNING = "Warning!";
    char* LANG_VAL_FLOW_LIMIT_EXCEEDED = "Warning! Flow Limit Error";
    char* LANG_VAL_REF_PRESS_LOW = "Warning! Low Reference Pressure";
    char* LANG_VAL_LEAK_TEST_PASS = "Leak test OK";
    char* LANG_VAL_LEAK_TEST_FAILED = "Leak test fail";
    char* LANG_VAL_ERROR_LOADING_CONFIG = "Error loading config file";
    char* LANG_VAL_ERROR_SAVING_CONFIG = "Error saving config file";
    char* LANG_VAL_SAVING_CONFIG = "Saving config file";
    char* LANG_VAL_SAVING_CALIBRATION = "Error saving calibration file";
    char* LANG_VAL_ERROR_LOADING_FILE = "Error loading file";
    char* LANG_VAL_CAL_OFFET_VALUE = "Cal Value: ";
    char* LANG_VAL_LEAK_CAL_VALUE = "Leak Cal Value: ";
    char* LANG_VAL_DHT11_READ_FAIL = "DHT11 Read fail";
    char* LANG_VAL_BME280_READ_FAIL = "BME280 Read fail";
    char* LANG_VAL_LOW_FLOW_CAL_VAL = "Low Cal Value: ";
    char* LANG_VAL_HIGH_FLOW_CAL_VAL = "High Cal Value: ";
    char* LANG_VAL_REF_PRESS_VALUE = "Ref Press Val: ";
    char* LANG_VAL_NOT_ENABLED = "Not Enabled";
    char* LANG_VAL_START_REF_PRESSURE = "Using Startup Ref Pressure";
    char* LANG_VAL_FIXED_VALUE = "Fixed value: ";
    char* LANG_VAL_RUN_BENCH_TO_CALIBRATE = "Bench must be running to calibrate";
};


#endif