/****************************************
 * The DIY Flow Bench project
 * https://diyflowbench.com
 *
 * boards.h - define boards + pins
 *
 ***/


/****************************************
 * DIYFB Shield
 *
 * DESIGN NOTES: 
 ***/
#ifdef DIYFB_SHIELD 

    // Define Physical Pins

    // VAC CONTROL
    #define VAC_CONTROL_PIN         1       // vac motor(s) on/off
    #define VAC_SPEED_PIN           25      // DAC speed reference for VFD
    #define VAC_BLEED_VALVE_PIN     26      // DAC bleed valve control

    // SENSORS
    #define SPEED_SENSOR_PIN        12      // turbine speed for turbo flow bench
    #define VOLTAGE_PIN             2
    #define MAF_PIN                 4
    #define REF_PRESSURE_PIN        35
    #define REF_VAC_PIN             34
    #define PITOT_PIN               36
    #define TEMPERATURE_PIN         39
    #define REF_BARO_PIN            27
    #define HUMIDITY_PIN            17

    // COMMS
    #define SERIAL0_TX              1       // UART0
    #define SERIAL0_RX              3       // UART0
    #define SCA_PIN                 21 
    #define SCL_PIN                 22 

    //STEPPER MOTOR CONTROLLER
    #define AVO_ENBL                18
    #define AVO_STEP                19
    #define AVO_DIR                 23


    // NVM Addresses (NOTE: 0-99 reserved for menu)
    #define NVM_HIGH_FLOW_CAL_ADDR  100     //8 bytes for float
    #define NVM_LOW_FLOW_CAL_ADDR   107     //8 bytes for float
    #define NVM_LEAK_CAL_ADDR       115     //8 bytes for float
    #define NVM_CD_CAL_OFFSET_ADDR  116     //8 bytes for float
    #define NVM_REF_PRESS_CAL_ADDR  123     //8 bytes for float

#endif



/****************************************
 * ESP32 / ESP32DUINO
 ***/
#ifdef ESP32DUINO 


    // Define Physical Pins

    // VAC CONTROL
    #define VAC_CONTROL_PIN         1       // vac motor(s) on/off
    #define VAC_SPEED_PIN           25      // DAC speed reference for VFD
    #define VAC_BLEED_VALVE_PIN     26      // DAC bleed valve control

    // SENSORS
    #define SPEED_SENSOR_PIN        12      // turbine speed for turbo flow bench
    #define VOLTAGE_PIN             2
    #define MAF_PIN                 4
    #define REF_PRESSURE_PIN        35
    #define REF_VAC_PIN             34
    #define PITOT_PIN               36
    #define TEMPERATURE_PIN         39
    #define REF_BARO_PIN            27
    #define HUMIDITY_PIN            17

    // COMMS
    #define SERIAL0_TX              1       // UART0
    #define SERIAL0_RX              3       // UART0
    #define SCA_PIN                 21 
    #define SCL_PIN                 22 

    //STEPPER MOTOR CONTROLLER
    #define AVO_ENBL                18
    #define AVO_STEP                19
    #define AVO_DIR                 23


    // NVM Addresses (NOTE: 0-99 reserved for menu)
    #define NVM_HIGH_FLOW_CAL_ADDR  100    //8 bytes for float
    #define NVM_LOW_FLOW_CAL_ADDR   107    //8 bytes for float
    #define NVM_LEAK_CAL_ADDR       115    //8 bytes for float
    #define NVM_CD_CAL_OFFSET_ADDR  116    //8 bytes for float
    #define NVM_REF_PRESS_CAL_ADDR  123    //8 bytes for float

#endif


