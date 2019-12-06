/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <Arduino.h>
#include "IoAbstraction.h"

#define LATCH_TIME 5

ShiftRegisterIoAbstraction::ShiftRegisterIoAbstraction(uint8_t readClockPin, uint8_t readDataPin, uint8_t readLatchPin, uint8_t writeClockPin, uint8_t writeDataPin, 
                                                       uint8_t writeLatchPin, uint8_t noReadDevices, uint8_t noWriteDevices) {
	needsWrite = true;
	toWrite = 0;

	this->readClockPin = readClockPin;
	this->readDataPin = readDataPin;
	this->readLatchPin = readLatchPin;
	this->writeLatchPin = writeLatchPin;
	this->writeDataPin = writeDataPin;
	this->writeClockPin = writeClockPin;
	this->lastRead = 0;
	this->numOfDevicesRead = noReadDevices;
	this->numOfDevicesWrite = noWriteDevices; 

	if (writeDataPin != 0xff) {
		pinMode(writeLatchPin, OUTPUT);
		pinMode(writeDataPin, OUTPUT);
		pinMode(writeClockPin, OUTPUT);
		digitalWrite(writeLatchPin, LOW);
	}

	if (readLatchPin != 0xff) {
		pinMode(readLatchPin, OUTPUT);
		pinMode(readDataPin, INPUT);
		pinMode(readClockPin, OUTPUT);
		digitalWrite(readLatchPin, HIGH);
	}
}

void ShiftRegisterIoAbstraction::pinDirection(__attribute((unused)) uint8_t pin, __attribute((unused)) uint8_t mode) {
	// ignored, this implementation has hardwired inputs and outputs - inputs are 0-31, outputs are 32 onwards
}

void ShiftRegisterIoAbstraction::writeValue(uint8_t pin, uint8_t value) {
	if (pin < SHIFT_REGISTER_OUTPUT_CUTOVER) return;
	pin = pin - SHIFT_REGISTER_OUTPUT_CUTOVER;

	bitWrite(toWrite, pin, value);
	needsWrite = true;
}

void ShiftRegisterIoAbstraction::writePort(uint8_t pin, uint8_t portV) {
	uint32_t portVal = portV;
	if(pin < SHIFT_REGISTER_OUTPUT_CUTOVER) return;
	pin = pin - SHIFT_REGISTER_OUTPUT_CUTOVER;
	if(pin < 8) {
		toWrite &= 0xffffff00;
		toWrite |= portVal;
	}
	else if(pin < 16) {
		toWrite &= 0xffff00ff;
		toWrite |= (portVal << 8);
	}
	else if(pin < 24) {
		toWrite &= 0xff00ffff;
		toWrite |= (portVal << 16);
	}
	else {
		toWrite &= 0x00ffffff;
		toWrite |= (portVal << 24);
	}
	toWrite = portVal;
	needsWrite = true;
}

uint8_t ShiftRegisterIoAbstraction::readPort(uint8_t pin) {
	if(pin < 8) {
		return lastRead & 0xff;
	}
	else if(pin < 16) {
		return (lastRead >> 8) & 0xff;
	}
	else if(pin < 24) {
		return (lastRead >> 16) & 0xff;
	}
	else {
		return (lastRead >> 24) & 0xff;
	}
}

uint8_t ShiftRegisterIoAbstraction::readValue(uint8_t pin) {
	return ((lastRead & (1 << pin)) != 0) ? HIGH : LOW;
}

bool ShiftRegisterIoAbstraction::runLoop() {
	uint8_t i;
	if (readDataPin != 0xff) {
		digitalWrite(readLatchPin, LOW);
		delayMicroseconds(LATCH_TIME);
		digitalWrite(readLatchPin, HIGH);

		lastRead = 0;
		for(i = 0; i < numOfDevicesRead; ++i) {
			lastRead = lastRead << 8;
			lastRead |= (shiftIn(readDataPin, readClockPin, MSBFIRST) & 0xff);
		}
	}
	
	if (writeDataPin != 0xff && needsWrite) {
		digitalWrite(writeLatchPin, LOW);
		delayMicroseconds(LATCH_TIME);
		
		uint32_t shiftLocal = toWrite;
		for(i = 0; i < numOfDevicesWrite; ++i) {
			uint8_t regVal = (shiftLocal & 0xff);
			shiftOut(writeDataPin, writeClockPin, MSBFIRST, regVal);
			shiftLocal = shiftLocal >> 8;
		}
		needsWrite = false;
		digitalWrite(writeLatchPin, HIGH);
	}
	return true;
}

MultiIoAbstraction::MultiIoAbstraction(uint8_t arduinoPinsNeeded) {
	limits[0] = arduinoPinsNeeded;
	delegates[0] = ioUsingArduino();
	numDelegates = 1;
}

MultiIoAbstraction::~MultiIoAbstraction() {
	// delegates added are our responsibility to clean up
	for(uint8_t i=0; i<numDelegates; ++i) {
		delete delegates[i];
	}
}

void MultiIoAbstraction::addIoExpander(IoAbstractionRef expander, uint8_t numOfPinsNeeded) {
	limits[numDelegates]= limits[numDelegates - 1] + numOfPinsNeeded;
	delegates[numDelegates] = expander;

	numDelegates++;
}

uint8_t MultiIoAbstraction::doExpanderOp(uint8_t pin, uint8_t aVal, ExpanderOpFn fn) {
	uint8_t ret = -1;
	for(uint8_t i=0; i<numDelegates; ++i) {
		// when we are on the first expander, the "previous" last pin is 0.
		uint8_t last = (i==0) ? 0 : limits[i-1];

		// then we find the limit of the expander we are on
		uint8_t currLimit = limits[i];

		// and check if we have a match!
		if(pin >= last && pin < currLimit) {
			ret = fn(delegates[i], pin - last, aVal);
			break;
		}
	}
	return ret;
}

void MultiIoAbstraction::pinDirection(uint8_t pin, uint8_t mode) {
	doExpanderOp(pin, mode, [](IoAbstractionRef a, uint8_t p, uint8_t v) {
		a->pinDirection(p, v);
		return (uint8_t)0;
	});
}

void MultiIoAbstraction::writeValue(uint8_t pin, uint8_t value) {
	doExpanderOp(pin, value, [](IoAbstractionRef a, uint8_t p, uint8_t v) {
		a->writeValue(p, v);
		return (uint8_t)0;
	});
}

uint8_t MultiIoAbstraction::readValue(uint8_t pin) {
	return doExpanderOp(pin, 0, [](IoAbstractionRef a, uint8_t p, uint8_t) {
		uint8_t retn = a->readValue(p);
		return retn;
	});
}

void MultiIoAbstraction::writePort(uint8_t pin, uint8_t val) {
	doExpanderOp(pin, val, [](IoAbstractionRef a, uint8_t p, uint8_t v) {
		a->writePort(p, v);
		return (uint8_t)0;
	});
}

uint8_t MultiIoAbstraction::readPort(uint8_t pin) {
	return doExpanderOp(pin, 0, [](IoAbstractionRef a, uint8_t p, uint8_t) {
		return a->readPort(p);
	});
}

void MultiIoAbstraction::attachInterrupt(uint8_t pin, RawIntHandler intHandler, uint8_t mode) {
	for(uint8_t i=0; i<numDelegates; ++i) {
		// when we are on the first expander, the "previous" last pin is 0.
		uint8_t last = (i==0) ? 0 : limits[i-1];

		// then we find the limit of the expander we are on
		uint8_t currLimit = limits[i];

		// and check if we have a match!
		if(pin >= last && pin < currLimit) {
			delegates[i]->attachInterrupt(pin - last, intHandler, mode);
			break;
		}
	}
}

bool MultiIoAbstraction::runLoop() {
	bool runStatus = true;
	for(uint8_t i=0; i<numDelegates; ++i) {
		runStatus = runStatus && delegates[i]->runLoop();
	}
	return runStatus;
}

// helper functions to create the abstractions. 

IoAbstractionRef outputOnlyFromShiftRegister(uint8_t writeClkPin, uint8_t dataPin, uint8_t latchPin, uint8_t numOfDevices) {
	return new ShiftRegisterIoAbstraction(0xff, 0xff, 0xff, writeClkPin, dataPin, latchPin, 1, numOfDevices);
}

IoAbstractionRef inputOnlyFromShiftRegister(uint8_t readClkPin, uint8_t dataPin, uint8_t latchPin, uint8_t numOfDevices) {
	return new ShiftRegisterIoAbstraction(readClkPin, dataPin, latchPin, 0xff, 0xff, 0xff, numOfDevices, 1);
}

IoAbstractionRef inputOutputFromShiftRegister(uint8_t readClockPin, uint8_t readDataPin, uint8_t readLatchPin, uint8_t numOfReadDevices,
									          uint8_t writeClockPin, uint8_t writeDataPin, uint8_t writeLatchPin, uint8_t numOfWriteDevices) {
	return new ShiftRegisterIoAbstraction(readClockPin, readDataPin, readLatchPin, writeClockPin, writeDataPin, writeLatchPin, numOfReadDevices, numOfWriteDevices);
}

IoAbstractionRef inputOutputFromShiftRegister(uint8_t readClockPin, uint8_t readDataPin, uint8_t readLatchPin,
									          uint8_t writeClockPin, uint8_t writeDataPin, uint8_t writeLatchPin) {
	return new ShiftRegisterIoAbstraction(readClockPin, readDataPin, readLatchPin, writeClockPin, writeDataPin, writeLatchPin, 1, 1);
}

IoAbstractionRef arduinoAbstraction = NULL;
IoAbstractionRef ioUsingArduino() { 
	noInterrupts();
	if (arduinoAbstraction == NULL) {
		arduinoAbstraction = new BasicIoAbstraction();
	}
	interrupts();
	return arduinoAbstraction;
}
