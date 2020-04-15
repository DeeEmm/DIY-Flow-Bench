#ifndef WirePlus_h
#define WirePlus_h

#include "Wire.h"
#include <Arduino.h> // for uint8_t data type

/** ######### usefull defines ################################################################# */

#define getmax(a,b) ((a)>(b)?(a):(b)) // TODO: implement as static const
#define BITMASK(a)  (1<<a)
#define BIT(a)      (1<<a)

#ifndef TRUE
#define TRUE        (1==1)
#define FALSE       (1==2)
#endif

#ifndef HIGH
#define LOW         (0)
#define HIGH        (1)
#endif

/// are they really usefull?
#define UBLB(a,b)  ( ( (a) << 8) | (b) )
#define UBLB19(a,b) ( ( (a) << 16 ) | (b) )
#define UBLB32(a,b,c,d)  ((( ((a)<<24) | ((b)<<16) ) | ((c)<<8)) | (d) )

/**< A way to get around Questions with shifting

    uint8_t value = 250;
    Serial.println(value);              // --> 250
    Serial.println(int8_t(value));      // --> -6
    Serial.println(int8_t(value)<<8);   // --> -1536 = -6*256

    Serial.println(uint8_t(value));      // --> 250
    Serial.println(uint8_t(value)<<8);   // --> -1536 = -6*256 !!!!!!!!!!!!!!!!!!!!!!
    Serial.println(uint16_t(value<<8)); // --> 64000

 */


class WirePlus
{

public:

    WirePlus();

    uint8_t probe       (const uint8_t);
    uint8_t probeAddress(const uint8_t);
    void    write       (const uint8_t, const uint8_t, const uint8_t *, const uint8_t);
    void    writeByte   (const uint8_t, const uint8_t, const uint8_t);
    void    writeCMD    (const uint8_t, const uint8_t);
    uint8_t readByte    (const uint8_t, const uint8_t);
    void    read        (const uint8_t, const uint8_t, uint8_t *,     const uint8_t);
    void    setRegister (const uint8_t, const uint8_t, const uint8_t, const uint8_t);
    uint8_t getRegister (const uint8_t, const uint8_t, const uint8_t);

private:

    WirePlus(const WirePlus&);            // declaration only for copy constructor
    WirePlus& operator=(const WirePlus&);  // declaration only for copy assignment --> make it uncopyable
};

/** ######### Implementation ################################################################# */

WirePlus::WirePlus()
{
    Wire.begin(); 		// I2C as Master
    bitSet(PORTC, 4); 	// deactivate internal pull-ups for twi
    bitSet(PORTC, 5); 	// as per note from atmega8 manual pg167
    // switch to 400KHz I2C - eheheh
    TWBR = ((F_CPU / 400000L) - 16) / 2; // see twi_init in Wire/utility/twi.c
};
/** ######### Public Methods ################################################################# */



uint8_t WirePlus::probe(const uint8_t address)
{
    Wire.beginTransmission(address);
    if (Wire.endTransmission(true)==0) return 1; // found something
    else                               return 0; // no response
};

uint8_t WirePlus::probeAddress(const uint8_t address)
{
    return probe(address);
};

void WirePlus::write(const uint8_t address, const uint8_t register_address, const uint8_t write_value[], const uint8_t length=1)
{
    if (!length) return;
    Wire.beginTransmission(address);
    Wire.write(register_address);
    uint8_t counter;
    counter = 0;
    while (counter < length)
    {
        Wire.write(write_value[counter]);
        counter++;
    }
    Wire.endTransmission(true);
};

void WirePlus::writeByte(const uint8_t address, const uint8_t register_address, const uint8_t write_value)
{
    Wire.beginTransmission(address);
    Wire.write(register_address);
    Wire.write(write_value);
    Wire.endTransmission(true);
};

void WirePlus::writeCMD(const uint8_t address, const uint8_t cmd)
{
    Wire.beginTransmission(address);
    Wire.write(cmd);
    Wire.endTransmission();
};

void WirePlus::read(const uint8_t address, const uint8_t registeraddress, uint8_t buff[], const uint8_t length=1)
{
    Wire.beginTransmission(address); 	// Adress + WRITE (0)
    Wire.write(registeraddress);
    Wire.endTransmission(false); 		// No Stop Condition, for repeated Talk

    if (!length) return;
    Wire.requestFrom(address, length); 	// Address + READ (1)
    uint8_t _i;
    _i=0;
    while(Wire.available())
    {
        buff[_i] = Wire.read();
        _i++;
    }

    Wire.endTransmission(true); 		// Stop Condition
};

uint8_t WirePlus::readByte(const uint8_t address, const uint8_t register_address)
{
    uint8_t _readvalue;
    read(address, register_address, &_readvalue, 1);
    return _readvalue;
};

void WirePlus::setRegister(const uint8_t address, const uint8_t registeraddress, const uint8_t mask, const uint8_t writevalue)
{
    uint8_t _setting;
    read(address, registeraddress, &_setting, 1 );
    _setting     &= ~mask;
    _setting     |= (writevalue&mask);
    writeByte(address, registeraddress, _setting);
};

uint8_t WirePlus::getRegister(const uint8_t address, const uint8_t registeraddress, const uint8_t mask)
{
    uint8_t _setting;
    read(address, registeraddress, &_setting, (uint8_t)1 );
    return (_setting & mask);
};




extern WirePlus i2c;

/** ######### Preinstantiate Object ################################################################# */
WirePlus i2c;

//#include "i2c.cpp" // TODO: ugly BUGFIX to get Scripts without i2c down in filesize (i2c.cpp is loaded w/o request)

#endif
