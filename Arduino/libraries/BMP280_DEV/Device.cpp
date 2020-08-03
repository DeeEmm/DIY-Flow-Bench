/*
  Device is an I2C/SPI compatible base class library.
	
	Copyright (C) Martin Lindupp 2019
	
	V1.0.0 -- Initial release
	V1.0.1 -- Added ESP32 HSPI support	
	V1.0.2 -- Modification to allow external creation of HSPI object on ESP32
	
	The MIT License (MIT)
	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:
	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include <Device.h>

////////////////////////////////////////////////////////////////////////////////
// Device Class Constructors
////////////////////////////////////////////////////////////////////////////////

Device::Device() : comms(I2C_COMMS) {}															// Initialise constructor for I2C communications
Device::Device(uint8_t cs) : comms(SPI_COMMS), cs(cs), spiClockSpeed(1000000) {}		// Constructor for SPI communications
#ifdef ARDUINO_ARCH_ESP32																														// Constructor for ESP32 HSPI communications
Device::Device(uint8_t cs, uint8_t spiPort, SPIClass& spiClass) 
	: comms(SPI_COMMS), cs(cs), spiPort(spiPort), spi(&spiClass), spiClockSpeed(1000000) {}
#endif

////////////////////////////////////////////////////////////////////////////////
// Device Public Member Function
////////////////////////////////////////////////////////////////////////////////

void Device::setClock(uint32_t clockSpeed)													// Set the I2C or SPI clock speed
{
	if (comms == I2C_COMMS)
	{
		Wire.setClock(clockSpeed);
	}
	else
	{
		spiClockSpeed = clockSpeed;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Device I2C & SPI Wrapper (Protected) Member Functions
////////////////////////////////////////////////////////////////////////////////

void Device::initialise()																						// Initialise device communications
{
  if (comms == I2C_COMMS)																						// Check with communications bus has been selected I2C or SPI
	{
		Wire.begin();																										// Initialise I2C communication
		Wire.setClock(400000);																					// Set the SCL clock to default of 400kHz
	}
	else
	{
		digitalWrite(cs, HIGH);																					// Pull the chip select (CS) pin high
		pinMode(cs, OUTPUT);																						// Set-up the SPI chip select pin
#ifdef ARDUINO_ARCH_ESP32
		if (spiPort == HSPI)																						// Set-up spi pointer for VSPI or HSPI communications
		{
			spi->begin(14, 27, 13, 2);																		// Start HSPI on SCK 14, MOSI 13, MISO 24, SS CS (GPIO2 acts as dummy pin)
		}
		else
		{
			spi = &SPI;																										// Start VSPI on SCK 5, MOSI 18, MISO 19, SS CS
			spi->begin();
		}														
#else
		spi = &SPI;																											// Set-up spi pointer for SPI communications
		spi->begin();
#endif
	}
}

void Device::setI2CAddress(uint8_t addr)														// Set the Device's I2C address
{	
	address = addr;
}

void Device::writeByte(uint8_t subAddress, uint8_t data)
{
  if (comms == I2C_COMMS)
	{
		Wire.beginTransmission(address);  															// Write a byte to the sub-address using I2C
		Wire.write(subAddress);          
		Wire.write(data);                 
		Wire.endTransmission();          
	}
	else // if (comms == SPI_COMMS)
	{
		spi->beginTransaction(SPISettings(spiClockSpeed, MSBFIRST, SPI_MODE0));	// Write a byte to the sub-address using SPI
		digitalWrite(cs, LOW);
		spi->transfer(subAddress);
		spi->transfer(data);
		digitalWrite(cs, HIGH);
		spi->endTransaction();
	}
}

uint8_t Device::readByte(uint8_t subAddress)												// Read a byte from the sub-address using I2C
{
  uint8_t data;
	if (comms == I2C_COMMS)																		
	{
		Wire.beginTransmission(address);         
		Wire.write(subAddress);                  
		Wire.endTransmission(false);             
		Wire.requestFrom(address, (uint8_t)1);	 
		data = Wire.read();                      
	}
	else // if (comms == SPI_COMMS)
	{
		spi->beginTransaction(SPISettings(spiClockSpeed, MSBFIRST, SPI_MODE0));		// Read a byte from the sub-address using SPI
		digitalWrite(cs, LOW);
		spi->transfer(subAddress);
		data = spi->transfer(data);
		digitalWrite(cs, HIGH);
		spi->endTransaction();	
	}
  return data;                             													// Return data read from sub-address register
}

void Device::readBytes(uint8_t subAddress, uint8_t* dest, uint8_t count)
{  
  if (comms == I2C_COMMS)																						// Read "count" bytes into the "dest" buffer using I2C
	{
		Wire.beginTransmission(address);          
		Wire.write(subAddress);                   
		Wire.endTransmission(false);              
		uint8_t i = 0;
		Wire.requestFrom(address, count);  
		while (Wire.available()) 
		{
			dest[i++] = Wire.read();          
		}
	}
	else // if (comms == SPI_COMMS)		
	{
		spi->beginTransaction(SPISettings(spiClockSpeed, MSBFIRST, SPI_MODE0));	// Read "count" bytes into the "dest" buffer using SPI
		digitalWrite(cs, LOW);
		spi->transfer(subAddress);
		spi->transfer(dest, count);
		digitalWrite(cs, HIGH);
		spi->endTransaction();	
	}
}
