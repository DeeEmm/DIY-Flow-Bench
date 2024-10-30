/***********************************************************
 * Language Constants
 *
 * TODO:(#12) Translations
 ***/
#ifndef LANGDATA
#define LANGDATA

#include "Arduino.h"

struct Translator {
    String LANG_BLANK = ""; 
    String LANG_NULL = "NULL";
    String LANG_NO_ERROR = "Status OK";
    String LANG_SERVER_RUNNING = "Server Running";
    String LANG_WARNING = "Warning!";
    String LANG_FLOW_LIMIT_EXCEEDED = "Warning! Flow Limit Error";
    String LANG_REF_PRESS_LOW = "Warning! Low Reference Pressure";
    String LANG_LEAK_TEST_PASS = "Leak test OK";
    String LANG_LEAK_TEST_FAILED = "Leak test fail";
    String LANG_ERROR_LOADING_CONFIG = "Error loading config file";
    String LANG_ERROR_SAVING_CONFIG = "Error saving config file";
    String LANG_SAVING_CONFIG = "Saving config file";
    String LANG_SAVING_CALIBRATION = "Error saving calibration file";
    String LANG_ERROR_LOADING_FILE = "Error loading file";
    String LANG_DHT11_READ_FAIL = "DHT11 Read fail";
    String LANG_BME280_READ_FAIL = "BME280 Read fail";
    String LANG_LOW_FLOW_CAL_VAL = "Low Cal Value: ";
    String LANG_HIGH_FLOW_CAL_VAL = "High Cal Value: ";
    String LANG_REF_PRESS_VALUE = "Ref Press Val: ";
    String LANG_NOT_ENABLED = "Not Enabled";
    String LANG_START_REF_PRESSURE = "Using Startup Ref Pressure";
    String LANG_FIXED_VALUE = "Fixed value: ";
    String LANG_CALIBRATING = "Calibrating FLow Offset...";
    String LANG_LEAK_CALIBRATING = "Calibrating Leak Test...";
    String LANG_CAL_OFFET_VALUE = "Cal Value: ";
    String LANG_LEAK_CAL_VALUE = "Leak Cal Value: ";
    String LANG_RUN_BENCH_TO_CALIBRATE = "Bench must be running to calibrate";
    String LANG_BENCH_RUNNING = "Bench running";
    String LANG_BENCH_STOPPED = "Bench stopped";
    String LANG_DEBUG_MODE = "Debug Mode";
    String LANG_DEV_MODE = "Developer Mode";
    String LANG_SYSTEM_REBOOTING = "System Rebooting";
    String LANG_CANNOT_DELETE_INDEX = "Cannot delete index.html (overwrite it instead!)";
    String LANG_DELETE_FAILED = "File Delete Failed";
    String LANG_INVALID_ORIFICE_SELECTED = "Invalid Orifice selected";
    String LANG_ORIFICE_CHANGE = "Orifice Plate Changed";
    String LANG_UPLOAD_FAILED_NO_SPACE = "Upload rejected, not enough space";
    String LANG_FILE_UPLOADED = "File uploaded";
    
};

// Default web page served when no index.html has been loaded.
const String LANG_INDEX_HTML = "<!DOCTYPE HTML> <html lang='en'> <HEAD> <title>DIY Flow Bench</title> <meta name='viewport' content='width=device-width, initial-scale=1'> <script> function onFileUpload(event) { this.setState({ file: event.target.files[0] }); const { file } = this.state; const data = new FormData; data.append('data', file); fetch('/api/file/upload', { method: 'POST', body: data }).catch(e => { console.log('Request failed', e); }); } </script> <style> body, html { height: 100%; margin: 0; font-family: Arial; font-size: 22px } a:link { color: #0A1128; text-decoration: none } a:visited, a:active { color: #0A1128; text-decoration: none } a:hover { color: #666; text-decoration: none } .headerbar { overflow: hidden; background-color: #0A1128; text-align: center } .headerbar h1 a:link, .headerbar h1 a:active, .headerbar h1 a:visited, .headerbar h1 a:hover { color: white; text-decoration: none } .align-center { text-align: center } .file-upload-button { padding: 12px 0px; text-align: center } .button { display: inline-block; background-color: #008CBA; border: none; border-radius: 4px; color: white; padding: 12px 12px; text-decoration: none; font-size: 22px; margin: 2px; cursor: pointer; width: 150px } #footer { clear: both; text-align: center } .file-upload-button { padding: 12px 0px; text-align: center } .file-submit-button { padding: 12px 0px; text-align: center; font-size: 15px; padding: 6px 6px; } .input_container { border: 1px solid #e5e5e5; } input[type=file]::file-selector-button { background-color: #fff; color: #000; border: 0px; border-right: 1px solid #e5e5e5; padding: 10px 15px; margin-right: 20px; transition: .5s; } input[type=file]::file-selector-button:hover { background-color: #eee; border: 0px; border-right: 1px solid #e5e5e5; } </style> </HEAD> <BODY> <div class='headerbar'> <h1><a href='/'>DIY Flow Bench</a></h1> </div> <br> <div class='align-center'> <p>Welcome to the DIY Flow Bench.</p> <p>Please upload the index.html file to get started.</p> <br> <form method='POST' action='/api/file/upload' enctype='multipart/form-data'> <div class=\"input_container\"> <input type=\"file\" name=\"upload\" id=\"fileUpload\"> <input type='submit' value='Upload' class=\"button file-submit-button\"> </div> </form> </div> <br> <div id='footer'><a href='https://diyflowbench.com' target='new'>DIYFlowBench.com</a></div> <br> </BODY> </HTML>";
#endif