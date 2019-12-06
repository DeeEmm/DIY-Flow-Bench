/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <inttypes.h>
#include <Arduino.h>
#include "SwitchInput.h"

#define ONE_TURN_OF_ENCODER 32

SwitchInput switches;

void registerInterrupt(uint8_t pin);

KeyboardItem::KeyboardItem() {
	this->repeatInterval = NO_REPEAT;
	this->pin = -1;
	this->notify.callback = NULL;
	this->counter = 0;
	this->stateFlags = NOT_PRESSED;
	this->previousState = NOT_PRESSED;
	this->callbackOnRelease = NULL;
}

void KeyboardItem::initialise(uint8_t pin, KeyCallbackFn callback, uint8_t repeatInterval, bool keyLogicIsInverted) {
	this->notify.callback = callback;
	this->pin = pin;
	this->repeatInterval = repeatInterval;
	previousState = NOT_PRESSED;
	stateFlags = NOT_PRESSED;
	bitWrite(stateFlags, KEY_LISTENER_MODE_BIT, 0);
	bitWrite(stateFlags, KEY_LOGIC_IS_INVERTED, keyLogicIsInverted);
}

void KeyboardItem::initialise(uint8_t pin, SwitchListener* switchListener, uint8_t repeatInterval, bool keyLogicIsInverted) {
	this->notify.listener = switchListener;
	this->pin = pin;
	this->repeatInterval = repeatInterval;
	previousState = NOT_PRESSED;
	stateFlags = NOT_PRESSED;
	bitWrite(stateFlags, KEY_LISTENER_MODE_BIT, 1);
	bitWrite(stateFlags, KEY_LOGIC_IS_INVERTED, keyLogicIsInverted);
}

void KeyboardItem::onRelease(KeyCallbackFn callbackOnRelease) {
	this->callbackOnRelease = callbackOnRelease;
}

void KeyboardItem::trigger(bool held) {
	if (!notify.callback) return;

	if (isUsingListener()) notify.listener->onPressed(pin, held);
	else notify.callback(pin, held);
}

void KeyboardItem::triggerRelease(bool held) {
	if (isUsingListener()) notify.listener->onReleased(pin, held);
	else if(callbackOnRelease) callbackOnRelease(pin, held);
}

void KeyboardItem::checkAndTrigger(uint8_t buttonState){
	if (notify.callback == NULL && callbackOnRelease == NULL) return; 

	if (buttonState == HIGH) {
		if (getState() == NOT_PRESSED) {
			setState(DEBOUNCING1);
		}
		else if (isDebouncing()) {
			setState(PRESSED);
			previousState = PRESSED;
			counter = 0; 
			acceleration = 1;
			trigger(false);
				
		}
		else if (getState() == PRESSED) {
			counter++;
			if (counter > HOLD_THRESHOLD) {
				setState(BUTTON_HELD);
				previousState = BUTTON_HELD;
				trigger(true);
				counter = 0;
				acceleration = 1;
			}
		}
		else if (getState() == BUTTON_HELD && repeatInterval != NO_REPEAT && notify.callback != NULL) {
			counter = counter + (acceleration >> 2) + 1;
			if (counter > repeatInterval) {
				acceleration = min(255, acceleration + 1);
				trigger(true);
				counter = 0;
			}
		}
	}
	else if(getState() == DEBOUNCING1) {
		setState(DEBOUNCING2);
	}
	else {
		setState(NOT_PRESSED);
		if (previousState == PRESSED) {
			previousState = NOT_PRESSED;
			triggerRelease(false);
		} else if (previousState == BUTTON_HELD){
			previousState = NOT_PRESSED;
			triggerRelease(true);
		}
	}
}

SwitchInput::SwitchInput() {
	this->numberOfKeys = 0;	
	this->ioDevice = NULL;
	this->swFlags = 0;
	for (int i = 0; i < MAX_ROTARY_ENCODERS; ++i) {
		encoder[i] = NULL;
	}
}

void SwitchInput::initialiseInterrupt(IoAbstractionRef ioDevice, bool usePullUpSwitching) {
	this->ioDevice = ioDevice;
	this->swFlags = 0;
	this->numberOfKeys = 0;
	bitWrite(swFlags, SW_FLAG_PULLUP_LOGIC, usePullUpSwitching);
	bitSet(swFlags, SW_FLAG_INTERRUPT_DRIVEN);

	// do not start any tasks here, we need to register interrupt on the pins instead.
}

void SwitchInput::initialise(IoAbstractionRef ioDevice, bool usePullUpSwitching) {
	this->ioDevice = ioDevice;
	this->swFlags = 0;
	this->numberOfKeys = 0;
	bitWrite(swFlags, SW_FLAG_PULLUP_LOGIC, usePullUpSwitching);

	taskManager.scheduleFixedRate(20, [] {
		switches.runLoop();
	});

}

bool SwitchInput::addSwitch(uint8_t pin, KeyCallbackFn callback,uint8_t repeat, bool invertLogic) {
	if(internalAddSwitch(pin, invertLogic)) {
    	keys[numberOfKeys++].initialise(pin, callback, repeat, invertLogic);
    	return true;
    }
    
    return false;
}

bool SwitchInput::addSwitchListener(uint8_t pin, SwitchListener* listener, uint8_t repeat, bool invertLogic) {
	if(internalAddSwitch(pin, invertLogic)) {
        keys[numberOfKeys++].initialise(pin, listener, repeat, invertLogic);
    	return true;
    }
    
    return false;
}

bool SwitchInput::internalAddSwitch(uint8_t pin, bool invertLogic) {
	if (numberOfKeys >= MAX_KEYS) return false;
	if (ioDevice == NULL) initialise(ioUsingArduino(), true);

	ioDevicePinMode(ioDevice, pin, isPullupLogic(invertLogic) ? INPUT_PULLUP : INPUT);

    if (isInterruptDriven()) {
		registerInterrupt(pin);
	}

    return true;
}

void SwitchInput::onRelease(uint8_t pin, KeyCallbackFn callbackOnRelease) {
	if (ioDevice == NULL) initialise(ioUsingArduino(), true);

	for(uint8_t i=0; i<numberOfKeys; ++i) {
		if(keys[i].getPin() == pin) {
			keys[i].onRelease(callbackOnRelease);
			return;
		}
	}
}

bool SwitchInput::isSwitchPressed(uint8_t pin) {
	for(uint8_t i=0; i<numberOfKeys; ++i) {
		if(keys[i].getPin() == pin) {
			return keys[i].isPressed();
		}
	}
	return false;
}

void SwitchInput::pushSwitch(uint8_t pin, bool held) {
	for(uint8_t i=0; i<numberOfKeys; ++i) {
		if(keys[i].getPin() == pin) {
			keys[i].trigger(held);
			return;
		}
	}
}

void SwitchInput::changeEncoderPrecision(uint8_t slot, uint16_t precision, uint16_t currentValue) {
	if (slot < MAX_ROTARY_ENCODERS && encoder[slot] != NULL) {
		encoder[slot]->changePrecision(precision, currentValue);
	}
}

void SwitchInput::setEncoder(uint8_t slot, RotaryEncoder* encoder) {
	if (slot < MAX_ROTARY_ENCODERS) {
		this->encoder[slot] = encoder;
	}
}

bool SwitchInput::runLoop() {
	bool needAnotherGo = false;

	ioDeviceSync(ioDevice);

	for (int i = 0; i < numberOfKeys; ++i) {
		// get the pins current state
		uint8_t pinState = ioDeviceDigitalRead(ioDevice, keys[i].getPin());
		if(isPullupLogic(keys[i].isLogicInverted())) {
			pinState = !pinState;
		}
		// and pass to the key handler.
		keys[i].checkAndTrigger(pinState);

		// we need to call into here again if we are debouncing or anything is pressed.
		needAnotherGo |= (keys[i].isDebouncing() || keys[i].isPressed());
	}

	return needAnotherGo;
}


/******ROTARY ENCODERS *****/


RotaryEncoder::RotaryEncoder(EncoderCallbackFn callback) {
	this->callback = callback;
	this->currentReading = 0;
	this->maximumValue = 0;
}

void RotaryEncoder::changePrecision(uint16_t maxValue, int currentValue) {
	this->maximumValue = maxValue;
	this->currentReading = currentValue;
	callback(currentReading);
}

void RotaryEncoder::increment(int8_t incVal) {
	if(incVal >= 0) {
		if(currentReading + incVal <= maximumValue) {
			currentReading = min((uint16_t)(currentReading + incVal), maximumValue);
			callback(currentReading);
		}
	}
	else if(currentReading != 0 && currentReading < abs(incVal)) {
		currentReading = 0;
		callback(currentReading);
	}
	else if(currentReading != 0) {
		currentReading += incVal; 
		callback(currentReading);
	}	
}

HardwareRotaryEncoder::HardwareRotaryEncoder(uint8_t pinA, uint8_t pinB, EncoderCallbackFn callback) : RotaryEncoder(callback) {
	this->pinA = pinA;
	this->pinB = pinB;
	this->lastChange = micros();

	// set the pin directions to input with pull ups enabled
	ioDevicePinMode(switches.getIoAbstraction(), pinA, INPUT_PULLUP);
	ioDevicePinMode(switches.getIoAbstraction(), pinB, INPUT_PULLUP);

	// read back the initial values.
	ioDeviceSync(switches.getIoAbstraction());	
	this->aLast = ioDeviceDigitalRead(switches.getIoAbstraction(), pinA);
	this->cleanFromB = ioDeviceDigitalRead(switches.getIoAbstraction(), pinB);

	registerInterrupt(pinA);
}

void checkRunLoopAndRepeat() {
	// turn off interrupts until deboucing / repeat logic is complete.
	switches.setInterruptDebouncing(true);

	// instead of running constantly, we only run when there's a need to, eg something
	// is still in a debouncing state. Otherwise we wait for an interrupt.
	// switches.runLoop returns true when it needs to run again.
	if(switches.runLoop()) {
		taskManager.scheduleOnce(20, [] { 
			checkRunLoopAndRepeat();
		});
	}
	else {
		// back to normal now - interrupt only
		switches.setInterruptDebouncing(false);
	}
}

void onSwitchesInterrupt(__attribute__((unused)) uint8_t pin) {
	if(switches.isInterruptDriven() && !switches.isInterruptDebouncing()) {
		checkRunLoopAndRepeat();
	}

  for(int i = 0; i < MAX_ROTARY_ENCODERS; ++i) {
		if(switches.encoder[i]) {
			switches.encoder[i]->encoderChanged();
		}
	}
}

int HardwareRotaryEncoder::amountFromChange(unsigned long change) {
	if(change > 250000 || maximumValue < ONE_TURN_OF_ENCODER) return 1;

	if(change > 120000) return 2;
	else if (change > 70000) return 4;
	else if (change > 30000) return 6;
	else return 10;
}

void HardwareRotaryEncoder::encoderChanged() {
	ioDeviceSync(switches.getIoAbstraction());
	uint8_t a = ioDeviceDigitalRead(switches.getIoAbstraction(), pinA);
	uint8_t b = ioDeviceDigitalRead(switches.getIoAbstraction(), pinB);

	if(a != aLast) {
		aLast = a;
		if(b != cleanFromB) {
			cleanFromB = b;
			if(a) {	
				unsigned long timeNow = micros();
				int amt = amountFromChange(timeNow - lastChange);
				increment(a != b ? -amt : amt);
				lastChange = timeNow;
			}
		}
	}
}

/******** UP DOWN BUTTON ENCODER *******/

void switchEncoderUp(__attribute((unused)) uint8_t key, __attribute((unused)) bool heldDown) {
	switches.encoder[0]->increment(1);
}

void switchEncoderDown(__attribute((unused)) uint8_t key, __attribute((unused)) bool heldDown) {
	switches.encoder[0]->increment(-1);
}

EncoderUpDownButtons::EncoderUpDownButtons(uint8_t pinUp, uint8_t pinDown, EncoderCallbackFn callback, uint8_t speed) : RotaryEncoder(callback) {
	switches.addSwitch(pinUp, switchEncoderUp, speed);
	switches.addSwitch(pinDown, switchEncoderDown, speed);
}

/******** ENCODER SETUP METHODS ***********/

void setupUpDownButtonEncoder(uint8_t pinUp, uint8_t pinDown, EncoderCallbackFn callback) {
	if (switches.getIoAbstraction() == NULL) switches.initialise(ioUsingArduino(), true);

	EncoderUpDownButtons* enc = new EncoderUpDownButtons(pinUp, pinDown, callback);
	switches.setEncoder(enc);
}

void registerInterrupt(uint8_t pin) {
	taskManager.setInterruptCallback(onSwitchesInterrupt);
	taskManager.addInterrupt(switches.getIoAbstraction(), pin, CHANGE);
}

void setupRotaryEncoderWithInterrupt(uint8_t pinA, uint8_t pinB, EncoderCallbackFn callback) {
	if (switches.getIoAbstraction() == NULL) switches.initialise(ioUsingArduino(), true);

	switches.setEncoder(new HardwareRotaryEncoder(pinA, pinB, callback));
}
