#ifndef i2c_preset_h
#define i2c_preset_h

#include "i2c.h"
#include "i2c_Sensor.h"


/** ######################################################################

Driver for the preset-Sensor

 CONSUMPTION: standby X µA, measure X µA

 Details:


########################################################################  */

class PRESET : public i2cSensor
{

private:

    /** ######### Register-Map ################################################################# */
static const uint8_t    I2C_ADDRESS 	            =(0x60);


    /** ######### function definition ################################################################# */

public:
    /**< TODO: do i need a constructor? */
    PRESET(void)
    {
        //_address = I2C_ADDRESS;
    };

    /**<  gives values */
    void getMeasurement(uint8_t buffer[])
    {

    };

};

/** ######### Preinstantiate Object ################################################################# */
/** < it's better when this is done by the user */
//PRESET preset = PRESET();

#endif




