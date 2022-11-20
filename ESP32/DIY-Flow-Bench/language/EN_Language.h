/***********************************************************
 * Language Constants
 *
 * TODO:(#12) Translations
 ***/
#ifndef LANGDATA
#define LANGDATA


struct Translator {
    String LANG_VAL_BLANK = "";
    String LANG_VAL_NULL = "NULL";
    String LANG_VAL_NO_ERROR = "Status OK";
    String LANG_VAL_SERVER_RUNNING = "Server Running";
    String LANG_VAL_WARNING = "Warning!";
    String LANG_VAL_FLOW_LIMIT_EXCEEDED = "Warning! Flow Limit Error";
    String LANG_VAL_REF_PRESS_LOW = "Warning! Low Reference Pressure";
    String LANG_VAL_LEAK_TEST_PASS = "Leak test OK";
    String LANG_VAL_LEAK_TEST_FAILED = "Leak test fail";
    String LANG_VAL_ERROR_LOADING_CONFIG = "Error loading config file";
    String LANG_VAL_ERROR_SAVING_CONFIG = "Error saving config file";
    String LANG_VAL_SAVING_CONFIG = "Saving config file";
    String LANG_VAL_SAVING_CALIBRATION = "Error saving calibration file";
    String LANG_VAL_ERROR_LOADING_FILE = "Error loading file";
    String LANG_VAL_DHT11_READ_FAIL = "DHT11 Read fail";
    String LANG_VAL_BME280_READ_FAIL = "BME280 Read fail";
    String LANG_VAL_LOW_FLOW_CAL_VAL = "Low Cal Value: ";
    String LANG_VAL_HIGH_FLOW_CAL_VAL = "High Cal Value: ";
    String LANG_VAL_REF_PRESS_VALUE = "Ref Press Val: ";
    String LANG_VAL_NOT_ENABLED = "Not Enabled";
    String LANG_VAL_START_REF_PRESSURE = "Using Startup Ref Pressure";
    String LANG_VAL_FIXED_VALUE = "Fixed value: ";
    String LANG_VAL_CALIBRATING = "Calibrating FLow Offset...";
    String LANG_VAL_LEAK_CALIBRATING = "Calibrating Leak Test...";
    String LANG_VAL_CAL_OFFET_VALUE = "Cal Value: ";
    String LANG_VAL_LEAK_CAL_VALUE = "Leak Cal Value: ";
    String LANG_VAL_RUN_BENCH_TO_CALIBRATE = "Bench must be running to calibrate";
    String LANG_VAL_BENCH_RUNNING = "Bench running";
    String LANG_VAL_BENCH_STOPPED = "Bench stopped";
    String LANG_VAL_DEBUG_MODE = "Debug Mode";
    String LANG_VAL_DEV_MODE = "Developer Mode";
    String LANG_VAL_SYSTEM_REBOOTING = "System Rebooting";
    String LANG_VAL_CANNOT_DELETE_INDEX = "Cannot delete index.html (overwrite it instead!)";
    String LANG_VAL_DELETE_FAILED = "File Delete Failed";
    
};

// Default web page served when no index.html has been loaded.
const String LANG_VAL_INDEX_HTML = "<!DOCTYPE HTML><html lang='en'><HEAD><title>DIY Flow Bench</title><meta name='viewport' content='width=device-width, initial-scale=1'> <script>function onFileUpload(event){this.setState({file:event.target.files[0]});const{file}=this.state;const data=new FormData;data.append('data',file);fetch('/upload',{method:'POST',body:data}).catch(e=>{console.log('Request failed',e);});}</script> <style>body,html{height:100%;margin:0;font-family:Arial;font-size:22px}a:link{color:#0A1128;text-decoration:none}a:visited,a:active{color:#0A1128;text-decoration:none}a:hover{color:#666;text-decoration:none}.headerbar{overflow:hidden;background-color:#0A1128;text-align:center}.headerbar h1 a:link, .headerbar h1 a:active, .headerbar h1 a:visited, .headerbar h1 a:hover{color:white;text-decoration:none}.align-center{text-align:center}.file-upload-button{padding:12px 0px;text-align:center}.button{display:inline-block;background-color:#008CBA;border:none;border-radius:4px;color:white;padding:12px 12px;text-decoration:none;font-size:22px;margin:2px;cursor:pointer;width:150px}#footer{clear:both;text-align:center}.file-upload-button{padding:12px 0px;text-align:center}input[type='file']{display:none}</style></HEAD><BODY><div class='headerbar'><h1><a href='/' >DIY Flow Bench</a></h1></div> <br><div class='align-center'><p>Welcome to the DIY Flow Bench.</p><p>Please upload the index.html.gz file to get started.</p> <br><form method='POST' action='/upload' enctype='multipart/form-data'> <label for='data' class='file-upload-button button'>Select File</label> <input id='data' type='file' name='wtf'/> <input class='button file-submit-button' type='submit' value='Upload'/></form></div> <br><div id='footer'><a href='https://diyflowbench.com' target='new'>DIYFlowBench.com</a></div> <br></BODY></HTML>";

#endif