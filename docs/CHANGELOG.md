
## Project Status

https://github.com/users/DeeEmm/projects/5/



## Open Issues

https://github.com/DeeEmm/DIY-Flow-Bench/issues



## Contribution Guidelines

https://github.com/DeeEmm/DIY-Flow-Bench/blob/master/.github/CONTRIBUTING.md



## Changelog

- Most recent entry at top !!
- Build numbers follow format YY MM DD VVVV Where VVVV is the incremental daily version
- Build number is automatically created in version.json at compile time

Build Number    | Description of Change
-- | --
2502110001      | Up-rev to RC9
                | Graph download link fallback to target="_blank"
                | Update Documentation
2502060003      | Added VDO 5WK96132Z MAF (BMW M54B30)
2502060002      | Added Bosch 0280218038 MAF (Audi S4)
2502060001      | #138 Updated BOSCH 0280218067 polynomials
2502050009      | #289 Fixed Datagraph data type
2502050006      | Update README
2502050005      | #340 added back data folder
2502050002      | Code Tidy / Formatting / headers
2502050002      | Reformatted Changelog to markdown
.               | Moved python files into 'tools' directory
.               | General tidying
2502040029      | Updated Wiki links for calibration and mimic pages
2502040028      | Added MAF data for BOSCH 0280217531, 0280218019 and 0280217123
2502040017      | Added MAF data for BOSCH_0280218008
2502040016      | Updated MAF data for BOSCH_0280002421 + BOSCH 0280218067
2502020001      | Default pin value set to disabled
2501280078      | Improvements to GUI tile handling
2501280059      | Added coefficients to Mimic page
2501280022      | Improved MAF sensor integration
2501280013      | #341 Change default reference standard to ISO 1585
2501270053      | #338 Reconfigure / optimise miniz library
2501270051      | Upgrade to espressif32 @ 6.10.0 
.               | Increase flash partition size
2501270023      | Update for pioarduino compatibility
2501260023      | #332 Hide Disabled tiles
2501260012      | #329 Rewrote liftdata.json download request handler
.               | Update ArduinoJSON From V6 to V7
2501250016      | Changed static js/css files to be served as gzip
2501250004      | Updated ESPAsyncWebServer Library
2501240012      | Review / Update API
2501240006      | Removed Poll Timers
2501240004      | #325 Removed Semaphore
.               | #325 Added ADC / BME / SSE task sequencing
2501240002      | Optimised Pitot sensor reading 
2501230049      | #324 MAF Dropdown restart warning
2501230048      | #322 Calibration page finalised
2501220008      | #322 Moved API settings to system.h
.               | Moved calibration settings to dedicated page
2501220004      | #322 Removed calibration fields from settings page
2501220001      | #321 Zero Datagraph data
2501210019      | Loop size tuning
2501200054      | #323 Revert timing changes
2501200051      | Removed GUI_BUILD_NUMBER
2501200041      | Loop stack size set in system.h
.               | Loop stack watermark reported on boot
2501200030      | #321 Datagraph clear fixed
2501200001      | Removed fullpath from generated comment in htmlToBytes.py
2501170010      | #316 Updated poll timer values
.               | Removed redundant calls to old ADS1115_lite library
.               | #320 Fixed templating issue
.               | Changed opacity of capture modal on hover (obscured flow tile)
.               | Added htmlToBytes.py to compiler pre-tasks
.               | Fixed Mg/sec conversion for mimic page
2501170004      | Removed refresh rate from GUI
2501170002      | #316 Disable ADC continuous mode
2501170001      | #238 updated wiki links in GUI
2501160023      | #316 Updated ADC-Lite-Test to DEV level
2501160017      | #316 Updated Poll timers
2501150016      | #316 Updated ADC library
.               | Moved ADC + BME poll timers into system.h
.               | Calibrated poll timer values
.               | Updated ADS read frequency
.               | Removed critical settings from GUI
.               | Moved ADC / BME xTasks to core 2
2501140006      | Update labels / terminology in GUI
2501140003      | #316 benchIsRunning ignores pRef if disabled
2501140002      | #316 changed benchIsRunning to use stored value of MAF flow
2501130048      | #275 - Set flow offset for all flow types
2501130047      | Moved data filter alpha values into system.h
.               | Removed iMAF_MIN_VOLTS from GUI
2501130047      | #316 Reset loop delays to 1ms
2501130046      | Default ADC / BME scan count changed
.               | Moved poll timer resets to end of tasks
.               | Removed tasks from core 2
2501130045      | #316 Moved ADC scan delay inside task loop
2501130038      | ADC/BME scan timers include execution time
2501130034      | Added Loop() scan time to mimic
2501130032      | Added A/B tests to API
2501120020      | #316 Optimised MAF flow calcs
.               | Deleted configuration.json (deprecated)
2501120017      | #316 Assign sensor tasks to core 2
2501120007      | #316 Addded scan counters to mimic page
2501110021      | #315 Wifi STA Mode Login issue fixed
.               | Added Accesskey Navigation to GUI
2501100018      | #307 BUG NVM data type for MAF type
2501090031      | #268 increased resolution and converted to INH2O
2501090027      | Added MCU Data to mimic page
2501090012      | #311 Save config values
.               | Changed config number fields resolution
2501090012      | ADC initialisation change
2501090003      | #307 - Validated BOSCH_0280218067 transfer function
2501080013      | Added float flag to conf var initalisation
.               | Added 3v3 + 5v bus voltage vars
.               | Zeroed disabled sensor values
.               | #307 Fixed ADC class instantiation
2501070013      | #307 Update Polynomial Data
2501070010      | Bugfix (Config dropdowns)
.               | #307 Excel Polynomial Test
.               | Removed Deprecated Code
2501060010      | #236 Added Mimic Page
2501050009      | Resolved NVM Save issue
2412310045      | Update Pins configuration
2412290045      | Update Hardware configuration layout
2412290022      | Sensor page validation
2412280024      | Update settings layout
2412260034      | Simplified data filter / rounding code
2412260018      | #303 Reset AP via API
2412240028      | Added Pins and Config pages
2412220046      | Decompress stream direct to server response
2412220036      | Compressed HTML in PROGMEM using miniz
2412220001      | Changed MAF Data to polynomial calculation 
2412180019      | Moved Lift Data into NVM
2412180014      | Moved calibration data in to NVM
2412180009      | Renamed _preferences to _config_pref
2412180008      | Moved Settings into NVM
2412160014      | Move config into NVM
2412150065      | Split HTML files to reduce transaction size
2412150026      | Moved index.html / javascript and css into progmem
2412140053      | Added heap memory to API
2412140026      | Basic Data Modal Added (WIP)
2412140001      | #236 Added Sensor_MV to json data
2412130038      | #288 Sensor status info incorrect
2412130037      | Changed pre:user_actions
2412130025      | #292 Changed Webserver::fileUpload 'final' code
2412130015      | Fixed Issue with pre-compile tasks deleting release
2412120051      | #288 Updated Analog calcs to cast to double
2412120016      | Changed ADS1115 to 8 SPS
.               | #288 removed struct from ADC calc
.               | Renames buildIndexSSEJsonData to buildIndexSSEJsonData
2412120002      | #288 fixed 60mv sensor deviation
2412110117      | Refactored MAF JSON code handling
.               | Removed MAF initialisation from sensors.begin()      
2412110103      | #285 increase template size in DataHandler::loadJSONFile
2412110079      | Fixed MAF file layouts
2412110068      | Moved MAF initalisation into DataHandler
2412110030      | SFLOW value missing when < 0
.               | Linked MAF info to forum thread
2412100102      | Updated PINS files
2412100094      | #281 Pitot GUi fixes
2412100089      | Align GUI build # with Code build #
2412100088      | Integrate GUI Version into gulp tasks
2412100087      | Moved voltage trim outside of validation test
2412100084      | Fixed JSON to Vector conversion
.               | Added verbose print mode
2412090117      | Fixed GUI info for MAF sensor type
2412090109      | Fixed sensor_src type
2412090065      | Fixed XHR form submission
2412090001      | #277 Temperature correction for ISO 5011
2412080010      | GUI fixed version increment in GUI info
2412080005      | Test build target for pre user actions
2412060066      | Reformat JSON Data in API
2412060044      | Updated Serial API responses (inc boot loop)
2412060001      | Update boot loop file prompts to link to Github
2412050201      | Convert MAF JSON to vector of vectors
.               | Change key>val longs to u_ints
2412050090      | Recalculated JSON memory allocation
.               | Increased LOOP_STACK_SIZE
.               | Added MAF / config JSON file dumps to API
2412050001      | Update build number format [2 to 4 digit]
.               | Auto build number update
24120401        | Utilise api/file/upload path for all uploads
.               | Remove redirect from valve lift capture (change to XHR)
.               | Updated gulp to auto version GUI html file
24120302        | #273 Remove redirect from bench on/off
.               | Utilise boot loop for runtime file handling
24120301        | Update index.html
.               | Change I2C addressing to decimal
24120205        | Update MAF.json handling
.               | Address issue with loading configuration.json
24120204        | Update GUI file load management
24120203        | Update sensor type handling
24120202        | Update sensor enable handling
24120201        | MAF data file json format
24120102        | Fixed compile error
24120101        | Tidied up status definitions in sensors.begin
24113002        | Removed precompile macros assoc/w configuration.h
24113001        | #205 Post compile configuration refactor
24112901        | Pitot Tile not displaying correct value for ∆P
.               | Correction in calibaration.cpp for calData pdiff and pitot.
.               | API Corrections
.               | Added merge-firmware.py
.               | Capitalised DOCS + CHANGELOG directories
.               | Change datagraph image download to JPG
24112701        | Create firmware binaries
24112601        | #213 Revert change to allow -ve delta / velocity
24112504        | Datagraph image format (jpg)
24112503        | #213 Pitot offset error
24112502        | #157 Updated datagraph settings
.               | Capture radio retains selection
24112501        | #239 Update API
24112402        | #213 Pitot / differential zeroing
.               | #254 Report sensor voltage to API
.               | Cookie to remember tile status
24112401        | #158 Correction to output
24112301        | #158 Add capture datatype selection
24112203        | #212 Changed Modal Headers
24112202        | #212 Change info modal to two column layout
24112201        | #244 simplification of resolution / accuracy
24112102        | Updated pins files
24112101        | #213 Working pitot velocity
.               | Zero reset for pitot and PDiff sensors
.               | Added pitot velocity calcs
.               | Typecast pins to pin struct from pins.json
.               | Change comparitor for testing pins in initialiseIO 
.               | Added additional checks to initialiseIO
24112001        | Added Language Template vars to GUI
24111901        | Added Create-Issue-Branch workflow
.               | General project refactor
24111801        | Added BME680 Configuration
.               | Forked BME680 library (https://github.com/DeeEmm/BME680)
.               | Moved github documentation into .github folder
24111702        | #213 Pitot changes
.               | GUI Updated
.               | Added help prompt to serial monitor boot sequence
.               | Stepper Unit tests
.               | Updated PINS_WEMOS.json
.               | Updated pins loading and assignment 
24111701        | #237 Update pin assignmnt
24111601        | Code Re-factor
.               | Added check to pins initialisation
24111501        | Added pins.json files
.               | added 'QuickStart.md' to Docs
.               | Updated contributing.md
.               | Created getRemote() function to read remote file
.               | Allow non-specific file name for pins and maf uploads
24111401        | GUI Updated
.               | Added GUI version to info pane
.               | Moved system settings into system.h 
.               | Split webserver methods out into comms / data / server functions
.               | Added pre-compile build vars for version in main code
.               | Tidied platformio.ini
.               | Added auto version for main code
.               | created user-actions.py for compile time tasks (requires python installed)
24111201        | index.html versioning
.               | Move data related methods into dedicated datahandler class
.               | Move pins.h into JSON file
.               | Added index.html versioning
24111107        | #222 Adjusted standard flow
24111106        | #220 Datagraph max value
24111105        | #212 Added scroll to modals
24111104        | #217 add GPT version
24111103        | #217 Scaled MAF housing returns O.O.R. value
24111102        | #212 Modal Window cropped
24111101        | #208 Added SCFM
.               | #212 Display issues
.               | Redirect to Main after config change
24110901        | Changed the pressure ratio formula to allow +/-ve values to be calculated
24110802        | #207 Decimal places on GUI
24110801        | Code tidy up so the pressure sensors are correctly reported in the web GUI info page
24110704        | Updated calls to // #include " version.h"
24110703        | Added call to // version.h in webserver.h
24110702        | Changed min pressure threshold to allow -ve values
.               | Moved version vars into dedicated file
24110701        | M5Stack Tube pressure sensors initialisation
.               | Correction to MAF rescaling function
24110403        | Added HTTP API Call for JSON Data
24110402        | #180 Added ability to truncate / round flow values in GUI
24110401        | #179 - Incorrect convertFlowDepression method
24110302        | Fixed flow thresholds to allow negative flow values in GUI
24110301        | Added M5Stack Core2 Support
.               | Added M5Stack Tube Pressure sensor
.               | Fixed line endings in sensors.cpp
.               | Fixed bug in VelocityToVolumetric method
24110201        | SD Card methods validated 
.               | Missing line terminator #187 
.               | Changed unused pins to '-1' in pins.h
24110101        | Missing line terminator #187 
.               | Added user definable flow differential target
.               | Finalised flow differential code
24103002        | Added additional 'Flow Differential' tile
24103101        | Updated leak cal fucntionality as per discssion #178
.               | Refactored webserver class to reduce spaghetti
.               | General code tidying
24103002        | Reformatted BOM CSV for Git rendering
24103001        | Added manual save function for calibration data
24103001        | Added firmware update via GUI
24102902        | Updated README
24102901        | Fixed bug where filter value was not retained
24102803        | Added data filter for Rolling Median and base code for Cyclic Average and Mode
24102802        | Added MAF Flow tile to dashboard
24102801        | Updated flow calibration routine
.               | Changed partition regime to allow for OTA
.               | General code tidy
.               | Updated language constants
.               | Fixed issue with index.html upload
24102701        | Moved calibration buttons to main tab
24102601        | #178 - Leak Test Calibration data 
.               | #179 - Flow test Calibration
24092301        | Added basic support for orifice / pitot / venturi style benches
24091103        | Updated PCB Readme
24091102        | Added PCB images
24091101        | Updated project PCB files and documentation
24090301        | #164 Baro value display
.               | Removed deprecated methods
.               | Masked zero flow values
23060501        | Added pin map for ESP32_WROOM_32
23052201        | Added xQueueCreate definition to try and address message queue errors
23042401        | Changed CFM to ACFM in GUI 
23042301        | Updated convertFlowDepression() method 
23041401        | Updated package.json...
.               | Moved dependencies into devDependencies section and made private
23041301        | Added graph image capture
.               | General code tidy
23041202        | Added adjustment for humidity and ref pressure to CFM Calc
23041201        | Updated MAf conversion code
.               | Implimented simplified Mass flow to CFM conversion
.               | Updated ACDELCO_92281162 MAF Data
23041102        | Changed 'Save Data' button test to 'Capture'
23041101        | Basic lift data handling finalised
23041001        | Increased loop task size in platformio
23040901        | Fixed error with MPXV7025 info display
.               | Fixed error with MPXV7025 transfer function
.               | Fixed error with MAF Data scaling
23040601        | Added lift data handling
.               | liftdata.json file management in spiffs
.               | liftdata.json export via WebGUI
23040402        | Updated GUI Graph data handling
23040401        | Added updated SIEMENS_5WK9605 MAF Data file
23040302        | Added GUI handling for MAF diameter
23040301        | Added support for Bosch 0280218067 MAF
.               | Added transfer function for MAF housing diameters
.               | Removed Siemens 5WK9605 MAF Data
23033101        | Added diameter var to MAFData files
23033001        | Added yml template for Support forum Posting Guidelines
23032901        | Added support for MPXV7025 pressure sensor
23030801        | Created code to generate lift plot from custom XML element (data array)
23030701        | Created basic lift graph
.               | Fixed mafData file extensions in configuration.h
23030601        | Added Dataview layout
.               | Added basic SD handling
23030202        | Added ability to change bench type in GUI
.               | Fixed AP Mode IP address display in GUI
23030201        | Updated calibration code for flow and leaktest calibration
.               | Added CERN-OSH-W license for Hardware
.               | Updated Hardware BOM
23022801        | Reformatted data tiles to include swirl
.               | Added swirl function
23022501        | Added support for XGZP6899A007 and XGZP6899A010 pressure sensors
23022401        | Decreased task update rates to address message queue overload
.               | Updated README
23022301        | Trunate fractional part of lift interval if interval is intval
23022201        | Reverted temporary swap of Pdiff / Pref Analog channels
.               | Added Data save and Data view modal dialogs 
23021601        | Code Tidying
23021501        | Outline of VFD control
.               | Added adjustment for absolute humidity in flow conversion
.               | Finalised pin mapping for Wemos D1
23021301        | Added pin outs for Arducam boards (UNTESTED)
.               | Updated README
.               | Fixed sensor type display on info page
.               | Tidied pinouts from Wemos D1 (For shield compatibility)
23020901        | Display MAC Address
.               | Some improvements to WiFi
.               | Changed default STA Password
23020801        | Substituted sensors->getmafVolts for hardware->getADCvolts
23020701        | Changed conversion from mass flow to volumetric
.               | Fixed syntax errors in MAF Data files
.               | Renamed main tasks for continuity
.               | Added ATM units to pressure conversion 
.               | Added conversion between mass flow units
.               | Changed MAF datatype to int32
23020201        | Refactored MAF code and Mass to Volumetric conversion
23020102        | Fixed include in MH95_3000_100 MAF Data file
23020101        | Updated ReadMe 
.               | Updated DEV_BRANCH in configuration.h
.               | Updated MAF Data vectors to long datatype
.               | Fixed incorrect build number for RC4 (NOTE prev 31012301)
23013101        | Release Candidate 4
.               | Fixed MAF unit error
.               | Fix Ref depression unit error
.               | Added bench running check to adjusted flow calc
23012701        | Renamed TASKgetBenchData to TASKgetSensorData
23012501        | Fixed checksum (CRC32)     
23012401        | Updated ESP library names for Arduino platform
23012301        | Updated platformio.ini
23012001        | Changed variable names in mafData files
.               | Updated readme
23011601        | Changed wifi connection to return status value 
.               | Changed wifi connection to use waitForConnectResult() instead of status()
23011501        | #114 default to AP mode if Wifi pass unedited
23010601        | Changed template delimiter to use ~ instead of %
23010201        | Added WiFi recovery command to API
.               | Changed INWG to INH2O for continuity in terminology
.               | Added default output for convert pressure calc
.               | Fixed ADS / ADC reference bug
22122401        | Started on development of differential pressure code
.               | Basic orifice control implemented
22122001        | Updated temperature and pressure conversions to accept variable input types
22121901        | Renamed MAF Vars to remove ambiguity
.               | Added ability to change temperature units in GUI
.               | Minor bugfixes
.               | Code tidying
.               | #8 Reference pressure selection for adjusted flow
22121802        | Corrections to MAF flow calcs
.               | Fixed adjusted flow and added to GUI
22121801        | Added startup delay to allow device to wake
.               | Move SSE Push into main loop to improve network stability
.               | Combined SSE events into single SSE event in JSON format
22121501        | Added uptime decode & status message
.               | Wifi connection improvements
22121401        | Updated esphome library versions to use latest
22121201        | Fixed bug #111 wifi connection improvements
22120801        | Fixed ADC disable
22120103        | General code Tidy
22120102        | Updated MAF files
22120101        | Calculations code refactor
22112901        | MAF code refactor. 
.               | Simplified sensor millivolt calcs to use volts
.               | Fixed int to double typecast canary panic in ADC & Sensor functions
.               | Added API call for uptime
.               | Added API Call for Xtask Stack HWM
.               | Changed AsyncTCP and AsyncWebServer libraries to use ESPHome as this is better supported
22112402        | Changed ADS1115 library to use ADS1115_lite
.               | removed support for ADS1105 (not supported by ADS1115_lite library)
.               | Fixed scaling bug in A2D voltage conversion
22112401        | Solved I2C BME280 read issue - Redefinition of BME class within sensor read methods
22112201        | Integrated tiny::BME library
.               | Created tasks for sensor read and data push
.               | Added semaphore to tasks to prevent data corruption
.               | Created hex dump to serve favicon.ico file (browser load error)
.               | Changed sensor methods to use stored sensor values
.               | Started to update comments to Doxygen format
.               | Added WIP suffix to release version (Work In Progress)
.               | Removed I2CDev lbrary as tinyBME uses wire directly
21112202        | Fixed file download
.               | Evaluating tiny::BME library
21112201        | Fixed default web page
20112202        | Browser reboot function improvements
.               | Removed unused constants
.               | Removed deprecated code from webserver class
.               | Added status messages to GUI
.               | Moved default HTML into const
20112201        | Removed Settings class along with settings.cpp & settings.h
.               | Converted BME Sensor update to use Server Sent Events (SSE)
.               | Converted ADC Sensor update to use Server Sent Events (SSE)
.               | Refactoring of HTML and Javascript
.               | Implimented basic on/off fan motor control via broser + API
.               | Implimented browser reboot
.               | Fixed some navigation/url issues
.               | Implimented basic /api/function url schema (on/off/reboot)
18112201        | Changed config form to use POST vars instead of websockets
.               | Moved settings class methods into webserver class
.               | Rewrote WebUI file handler to use POST vars instead of websockets
15112201        | Added template processing using custom template_placeholders
.               | Added headers to javascript.js
.               | Added URL rewrite rule for / & /index.html
13112201        | Reverted char arrays back to strings
.               | Fixed CRC32 checksum for API responses
.               | Fixed ability to completely disable BME280 / ADC1115 / MAF
.               | Fixed Device Status in WebUI
.               | Fixed Sensor values displaying in webUI
.               | Updated Status pane layout in UI
.               | Fixed Baro value
.               | Changed Baro display value to millibar (hPa)
.               | Truncated sensor values to 2 decimal places
.               | Addressed rounding errors by changing floats to doubles
.               | Disabled index.html.gz for event source / template tests
.               | Using minified html instead
12112201        | Redacted SPI calls from within external libraries to allow VSCode debugging
.               | Moved MAFData into MAFData class to help better manage memory
.               | Refactored message calls from char* to std::string
.               | Replaced sensor.get calls in webserver with globals to reduce overheads
.               | Refactored platformIO.ini added dependencies + debug info
.               | Reviewed and updated .gitignore
03112201        | Migrated to VSCode from Arduino-CLI
.               | Code optimisation
01112201        | Refactored API class                - 
31102201        | Moved all webserver functions into webserver class
.               | Code refactor for main
.               | Moved tasks back into core 1 (test inconclusive)
23102201        | Code refactor for printf
.               | Replaced printString instances to char array
.               | Changed serial.print method function to utilise C++ vsnprintf method
21102201        | Split main loop into core 1 and webserver into core 2 (test)
.               | Refactored BME code
13102201        | Updated API functions
.               | Moved Debug_mode into config struct
.               | Added Status streaming mode to API
.               | Added Dev mode option
07072201        | Changed ADC1115 code to use I2CDev
02062201        | Renamed 'maths.h' class to 'calculations.h' (already a library class named math)
23062201        | Refactored I2C code
.               | Added globals for sensor data
.               | Sensor data read in loop and made available to websocket interrupt
.               | Updated hardware schema to V2
22061601        | Improved BME accuracy
22061501        | Removed BME sensors voltage signals from API
.               | Disabled MAF Frequency ISR's
22061301        | Fixed divide by Zero issue for ADC PRef
.               | Added get.sensor.millivolts functions for Pdiff / Pitot and MAF sensors
.               | Replaced MAP function 
.               | Changed Millivolt sensor vars from INT to FLOAT
22061201        | Updated API calls 
.               | Added API debug functions
.               | Added additional debug messages to assist with fault finding
22060901        | Updated pressure sensor algorithm
.               | updated ADC sensor values for supply voltage variation
22060801        | Temporarily replaced BME code with Adafruit library as humidity code was broken
.               | General code refactoring
.               | Addressed NPM dependabot alerts with update of NPM / Gulp and dependencies
21092301        | Reformatted changelog
21092201        | Added user defined vanity title
.               | Dropped support for DHT11
.               | Fixed RH reading
.               | Added ADC1015 support for MAF / PRef / PDiff / Pitot
.               | #97 API disabled by debug mode
.               | Fixed Board voltage ADC code
.               | Change pressure sensor calcs to pascals
.               | Dropped support for MPX5100/5700
.               | NOTE: Remapped pins
21091801        | Custom BME280 code to replace Adafruit and Sparkfun libraries
.               | Added soft reset button to config panel
.               | Added I2C Scanner to serial monitor
21091101        | Added additional dependencies for BME280 - ESP32
21090801        | (temporarily disabled ADC)
.               | Updated BOM in docs folder
21090101        | Added support for 1015 / 1115 ADC for MAF / PRef / PDiff / Pitot
21082503        | #87 Wifi client network
.               | Corrected millivolt calc in Webserver::buildIndexSSEJsonData
21082502        | Reviewed Status and error messages
.               | Changed release version to V2
.               | Package up for Release Candidate 1
21082501        | Checked & updated remaining MAF data files
.               | Hardware::configurePins
.               | Added orifice detection code pins
.               | Started on #84 
.               | General code tidy
21082401        | #81 Review 10-12 bit conversion for all inputs
.               | #26 Frequency based MAF support
21082301        | #78 Added Debug mode to API (Verbose messaging)
.               | Updated several compiler errors from WIN compiler
.               | Converted mafData files to individual classes
.               | Created single header file mafData.h
.               | Created frequency based MAF Data files
.               | #80 Added software trimpots
.               | Converted mafData files back to header files due to conflicts
.               | Renamed mafData files (changed dashes to underscores)
.               | Addressed compiletime issues found in Arduino IDE
.               | Compiling and working on Arduino-CLI
.               | Renamed sensor classes
.               | Tidied up MAF sensor code
21082001        | Added vac control O/P
.               | #75 Fixed calibration data handling
21081902        | #75 Auto create config / calibration json file(s) if not exists
.               | Minor code tidying
.               | Updated README
21081902        | Removed DIY-Flow-Bench header file
.               | Flow calibration
.               | Leak test calibration
.               | added support for analog Temp / Baro / Relh sensors
.               | general code tidy
.               | Updated pin mapping
.               | added vanilla boot handling
.               | #63 Reviewed I/O
.               | #64 GZip compression
.               | #70 Server initialisation
21081301        | Major code refactor to object based structure complete
.               | Converted procedural functions to class + method
.               | Improved file upload handling
.               | Return to file upload modal after file upload
.               | Renamed Schema to Header to remove ambiguity
.               | Added sensor types used to status dialog
.               | Fixed Javascript errors for status websocket update
.               | Fixed timeout for AP mode
.               | Optimised websocket handler browser code
.               | Remove header key from config JSON file
21080701        | #71 Added file download code to File manager
.               | #71 Added file delete code to File Manager
.               | #71 Added file Upload code to File Manager
.               | #65 Removed alarm mute feature
.               | Added data streaming inc refresh rate
.               | Added data streaming controls (on/off buttons)
.               | Started on code refactor
21072701        | Populated modal dialogs for file management and status
.               | Added status info for hardware configuration
.               | Reformatted 
.               | Created UI Graphs for data visualisation
.               | Refactored websocket communications to use JSON data
.               | Moved status and config data into Structs
21072401        | Added calibration settings
.               | Added additional configuration settings
.               | Updated configuration file JSON formatting
.               | Changed config file extension from txt to JSON
.               | Added modal dialogs for file management and status
21072301        | Added Multicast DNS Service
.               | Added fallback to AP mode
.               | Added tabbed interface to webserver
.               | Added configuration settings
21072001        | Code refactor
.               | Created basic Web UI
.               | Asynchronous web sockets
.               | Added Status messages
.               | Added JSON status message to API
21071802        | Added SPIFFS
.               | Added system status message to serial on boot
.               | Serial reporting permanently enabled
.               | Added Wifi Station mode
.               | Added basic web server
21071801        | Ported code to ESP32
.               | Removed TC menu support
.               | Removed external display support
.               | Disabled CRC Checksum
.               | Updated pin mapping
.               | Tested via Serial API
20081901        | Updated Build and Version variable length in TCMenu Designer
20081601        | #46 Added reference pressure offet to volumetric flow calculation
20081201        | #44 Added API call validation
20081101        | Added pins definition for DIYFB shield
.               | Added serial comms handler
.               | Added secondary serial comms for bluetooth
.               | Created digital manometer function (for additional flow / pitot displays)
.               | General code tidying
.               | Renamed pins.h to boards.h
.               | Added PMAS MH95-3000 MAF 
20080903        | Added delimiter to API responses
.               | Tidied API code
20080902        | Added API Enum : Flow:Ref:Temp:Humidity:Baro 
20080901        | Added CRC Checksum function
20080705        | Dropped support for PMX2050 (only posotive range so will not work with vac - D'Oh!)
.               | Updated BOM
.               | Updated hardware schema
.               | Updated readme
20080704        | Added MPX2050 Reference pressure sensor
.               | Improved code for using ref pressure value for baro on system boot
.               | Dropped support for BMP280
20080703        | Updated MPX4250 Ref pressure calculations
20080702        | Added I2C address to Sparkfun BME280 initialisation 
.               | Added configuration defaults for baro / temp / relH
.               | Fixed bench is running conditions / commented out low reference pressure alarm for testing
.               | Added releases to changelog
20080604        | V.1.0-beta.14
.               | #36 MPX4250 scaling error - subtracted atmospheric pressure from reading
.               | #39 Fixed flow value fraction value (displaying zero)
.               | #40 Fixed adjusted flow display
.               | Updated tcmenu
.               | Fixed & tested pitot display
.               | Changed pitot display to show differential as percentage difference (to P Ref)
20080603        | Updated LS2 MAF part number
.               | #37 updated temp / baro / humidty menu functions
.               | Added sensor test menu 
.               | Added Baro menu item
.               | #38 Fixed version + build menu items
20080602        | Update tcmenu layout 
20080601        | #37 Commented out 'freeze' statement (line 139) - while(1); //Freeze 
20080501        | Moved language file call to configation settings
.               | Move pin configuration into pins.h
.               | Refactored menu code and moved calibration into dedicated functions
.               | Removed redundant calibration functions
.               | Rewrote calibration functions
20080404        | Added comments to changelog file
20080403        | Added comments to mafData files
20080402        | Added analog mafData example 
20080401        | V.1.0-beta.13
.               | Fixed some typos and reduced program size
.               | #32 Added additional array type for analog mafData files (memory issue)
.               | #33 Fixed humidity correction is
20080307        | V.1.0-beta.12
.               | Added support for Sparkfun BME280 (works with generic 10 pin BME280)
20080306        | Updated changelog
20080305        | Updated documentation
20080304        | Removed redundant Alpha files
20080303        | Merge V1 branch into master
20080302        | Added raw analog decode to getMafflowCFM() function
20080301        | Added LS2 MAf Data file
.               | Updated MAF file format
20080204        | Differentiate between kg/h & mg/s in API MAF Data command.
.               | Added support for MPX4250 reference pressure sensor
20080203	| Added support for the Generic Commands listed on the API section of the WIKI.  
        	| Added the Serial API Specification to the doc folder
		| Added the beginning of the API layer for the GUI
20080202        | V.1.0-beta.11
.               | Added additional API debug commands
20080201        | Added additional variables to MAF data files #29
20080102        | V.1.0-beta.10
.               | #28 fixed incorrect CFM reading...
.               | Updated getMafFlowCFM function
.               | Corrected mafData files voltage
.               | Added additional functions to API
.               | Update configuration.h set default values
20080101        | Updated API Responses
20072901        | V.1.0-beta.9
.               | Changed format version varables
.               | Added basic serial API functions
.               | Dropped ilib280 support (depreciated)
.               | #19 Added Adafruit BME280 support
.               | Corrected datatypes in mafData files
.               | Added website URL in docs
20041501        | Reorganised project folder structure
20010101        | V.1.0-beta.8
.               | Fixed spelling issue for ilib library calls
	        | Fixed load issue for Baro and Pitot 
19123001        | Fixed float issue for pRef
19122601        | V.1.0-beta.7
.               | #23 DHT11 compile issue
	        | #19 Support for RH sensor
	        | Added MIN_MAF_MILLIVOLTS to filter MAF results 
		| #20 Fixed issue with Pitot sensor decode
19122401        | V.1.0-beta.6
.               | Bugfix release various display bugs
		| Validated Mass > Volumetric flow conversion
		| Validated Flow reference conversion
		| Disabled menu timeout
		| Added MafVolts to display for testing / diagnostics
		| Tidied Display layout
19122301        | V.1.0-beta.5
.               | Changed order of display items for better visibility on 2 line Display
.               | Added one decimal places to display values value (changed divisor to 10 in tcMenu)
.               | Renamed existing BMP280 code references to ADAFRUIT_BMP280
.               | Added support for ILIB_BMP280 library
.               | Added support for DHT11 sensor///////////s
.               | Added mass > volumetric flow compensation calculations
.               | Remapped Siemens_5WK9605 mafdata from cfm to kg/h
19122002        | V.1.0-beta.4
.               | Issue #18 Compile error for BMP280 library
19122001        | V.1.0-beta.3
.               | Added support for BMP280//sdfsdfdsfdf////
.               | Fixed potential MAF conversion issue
.               | Minor bugfixes
19121001        | V.1.0-beta.2
.               | Issue #15 - Shortened menu descriptors
.               | Issue #14 - Sorted data in Delphi MAF file
.               | Issue #16 - Low flow error
19120804        | V.1.0-beta.1
.               | Display Calibration results on screen 
19120803        | #7  Update calibration functions 
19120802        | Added partial support for additional languages
	        | Created function for popover display dialog
	        | Added max-flow error check
19120801        | Added support for MAF lookup tables via external files
	        | Including support for alternate MAF sensors
19120701        | Changed directory structure to resolve dependency issue 
.               | Moved libraries into src subfolder folder
.               | Created this changelog file
19120601        | Fixed compile error (extra curly bracket)


