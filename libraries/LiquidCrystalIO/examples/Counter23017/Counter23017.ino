/*
  LiquidCrystal Library - Hello World for 23017

  This assumes that an LCD display has been attached to the B port of a 23017 IO expander, connected over i2c.
  it will count up from 0 onto the display once per second.
*/

// include the library code:
#include <LiquidCrystalIO.h>
#include <IoAbstractionWire.h>
#include <Wire.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 8, en = 9, d4 = 10, d5 = 11, d6 = 12, d7 = 13;

// if you want to use the optional PWM contrast, you need to set the pin for it
const int pwmContrastPin = 5;

// if you want to reset the 23017 other than hold it to Vcc, include this.
const int resetPin23017 = 30;

// now construct the display using IO from a 23017
LiquidCrystal lcd(rs, en, d4, d5, d6, d7, ioFrom23017(0x20));


void setup() {
  
  // Optional:
  // if you don't want to bother connecting a potentiometer for contrast, instead see my example of
  // creating a PWM contrast circuit for the board, it's very simple see:
  // https://www.thecoderscorner.com/electronics/microcontrollers/driving-displays/90-wiring-a-20x4-character-display-to-an-arduino-board/
  pinMode(pwmContrastPin, OUTPUT);
  analogWrite(pwmContrastPin, 10);
  // End PWM contrast.

  // this is optional, in a real world system you could probably just connect the
  // reset pin of the device to Vcc, but when prototyping you'll want a reset
  // on every restart.
  pinMode(resetPin23017, OUTPUT);
  digitalWrite(resetPin23017, LOW);
  delayMicroseconds(100);
  digitalWrite(resetPin23017, HIGH);
  // End reset optional code.

	Wire.begin();
 
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);
  lcd.print("Counter in seconds");
}


void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);
}


